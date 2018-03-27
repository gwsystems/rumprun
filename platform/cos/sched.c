/*-
 * Copyright (c) 2015 Antti Kantee.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Historically based on the Xen Mini-OS scheduler by Grzegorz Milos,
 * rewritten to deal with multiple infrequently running threads in the
 * current reincarnation.
 */

#include <bmk-core/core.h>
#include <bmk-core/errno.h>
#include <bmk-core/memalloc.h>
#include <bmk-core/platform.h>
#include <bmk-core/pgalloc.h>
#include <bmk-core/printf.h>
#include <bmk-core/queue.h>
#include <bmk-core/string.h>
#include <bmk-core/types.h>

#include <rumpcalls.h>
#include <bmk-core/sched.h>

void *bmk_mainstackbase;
unsigned long bmk_mainstacksize;
/*
 * sleep for how long if there's absolutely nothing to do
 * (default 1s)
 */
#define BLOCKTIME_MAX (1*1000*1000*1000)

extern const char _tdata_start[], _tdata_end[];
extern const char _tbss_start[], _tbss_end[];
#define TDATASIZE (_tdata_end - _tdata_start)
#define TBSSSIZE (_tbss_end - _tbss_start)
#define TCBOFFSET \
    (((TDATASIZE + TBSSSIZE + sizeof(void *)-1)/sizeof(void *))*sizeof(void *))
#define TLSAREASIZE (TCBOFFSET + BMK_TLS_EXTRA)

int tcboffset;
int tdatasize;
int tbsssize;
const char *_tdata_start_cpy;

/* flags and their meanings + invariants */
#define THR_RUNQ	0x0001		/* on runq, can be run		*/
#define THR_TIMEQ	0x0002		/* on timeq, blocked w/ timeout	*/
#define THR_BLOCKQ	0x0004		/* on blockq, indefinite block	*/
#define THR_QMASK	0x0007
#define THR_RUNNING	0x0008		/* no queue, thread == current	*/

#define THR_TIMEDOUT	0x0010
#define THR_MUSTJOIN	0x0020
#define THR_JOINED	0x0040

#define THR_EXTSTACK	0x0100
#define THR_DEAD	0x0200
#define THR_BLOCKPREP	0x0400

/* scheduler api */
extern void bmk_cpu_sched_wakeup(struct bmk_thread *thread);
extern int  bmk_cpu_sched_block_timeout(struct bmk_thread *curr, unsigned long long timeout);
extern void bmk_cpu_sched_block(struct bmk_thread *curr);
extern void bmk_cpu_sched_yield(void);
extern void bmk_cpu_sched_exit(void);
extern void bmk_cpu_sched_set_prio(int prio); /* curr thd */

extern int bmk_strcmp(const char *a, const char *b);

/*
 * RG: The struct definition for bmk_thread was moved
 * to rumpcalls.h on the composite side so the composite system
 * could access bmk_current thread without whinning about imcomplete
 * types.
 */

__thread struct bmk_thread *bmk_current;

/* **** RG For debugging **** */

char *
get_name(struct bmk_thread *thread)
{
	return thread->bt_name;
}

/* ****************** */

void
set_cos_thddata(struct bmk_thread *thread, capid_t thd, thdid_t tid)
{
	thread->cos_thdcap = thd;
	thread->cos_tid = tid;
	thread->firsttime = 1;
}

capid_t
get_cos_thdcap(struct bmk_thread *thread)
{ return thread->cos_thdcap; }

thdid_t
get_cos_thdid(struct bmk_thread *thread)
{ return thread->cos_tid; }


TAILQ_HEAD(threadqueue, bmk_thread);
static struct threadqueue threadq = TAILQ_HEAD_INITIALIZER(threadq);
static void (*scheduler_hook)(void *, void *);

static void
print_threadinfo(struct bmk_thread *thread)
{ bmk_printf("Thread info: %s, %p, %i\n", thread->bt_name, thread, thread->bt_flags); }

static inline void
setflags(struct bmk_thread *thread, int add, int remove)
{
	thread->bt_flags &= ~remove;
	thread->bt_flags |= add;
}

static void
set_runnable(struct bmk_thread *thread)
{ bmk_cpu_sched_wakeup(thread); }

/*
 * Called with interrupts disabled
 */
static int
clear_runnable(void)
{
	int ret = 0;
	struct bmk_thread *thread = bmk_current;

	if (thread->bt_wakeup_time != BMK_SCHED_BLOCK_INFTIME) {
		ret = bmk_cpu_sched_block_timeout(thread, thread->bt_wakeup_time);
	} else {
		bmk_cpu_sched_block(thread);
	}

	return ret;
}

static void
stackalloc(void **stack, unsigned long *ss)
{
	*stack = bmk_pgalloc(bmk_stackpageorder);
	*ss = bmk_stacksize;
}

void
bmk_sched_dumpqueue(void)
{ print_threadinfo(bmk_current); }

extern bmk_time_t time_blocked;

static void
schedule(void)
{ bmk_cpu_sched_yield(); }

/*
 * Allocate tls and initialize it.
 * NOTE: does not initialize tcb, see inittcb().
 */
void *
bmk_sched_tls_alloc(void)
{
	char *tlsmem;

	tlsmem = bmk_memalloc(TLSAREASIZE, 0, BMK_MEMWHO_WIREDBMK);

	bmk_memcpy(tlsmem, _tdata_start, TDATASIZE); //copy from alloc to tlsmem
	bmk_memset(tlsmem + TDATASIZE, 0, TBSSSIZE);

	return tlsmem + TCBOFFSET;
}

/*
 * Free tls
 */
void
bmk_sched_tls_free(void *mem)
{
	/* BMK doesn't control their tls sections anymore */
	mem = (void *)((unsigned long)mem - TCBOFFSET);
	//bmk_memfree(mem, BMK_MEMWHO_WIREDBMK);
}

void *
bmk_sched_gettcb(void)
{ return (void *)bmk_current->bt_tcb.btcb_tp; }

void *
bmk_sched_thd_gettcb(struct bmk_thread *thread)
{ return crcalls.rump_tls_alloc(thread); }

static void
inittcb(struct bmk_tcb *tcb, void *tlsarea, unsigned long tlssize)
{

#if 0
	/* TCB initialization for Variant I */
	/* TODO */
#else
	/* TCB initialization for Variant II */
	*(void **)tlsarea = tlsarea;
	tcb->btcb_tp = (unsigned long)tlsarea;
	tcb->btcb_tpsize = tlssize;
#endif
}

static long bmk_curoff;
static void
initcurrent(void *tcb, struct bmk_thread *value)
{
	struct bmk_thread **dst = (void *)((unsigned long)tcb + bmk_curoff);

	*dst = value;
}


struct bmk_thread *
bmk_sched_create_withtls(const char *name, void *cookie, int joinable,
			 void (*f)(void *), void *data,
			 void *stack_base, unsigned long stack_size, void *tlsarea)
{
	struct bmk_thread *thread;
	void *tlsmgr_area = NULL;
	capid_t cos_thdcap;

	thread = bmk_xmalloc_bmk(sizeof(*thread));

	bmk_memset(thread, 0, sizeof(*thread));
	bmk_strncpy(thread->bt_name, name, sizeof(thread->bt_name)-1);

	if (!stack_base) {
		bmk_assert(stack_size == 0);
		stackalloc(&stack_base, &stack_size);
	} else {
		thread->bt_flags = THR_EXTSTACK;
	}
	thread->bt_stackbase = stack_base;
	if (joinable)
		thread->bt_flags |= THR_MUSTJOIN;

	bmk_cpu_sched_create(thread, &thread->bt_tcb, f, data,
			     stack_base, stack_size);

	/*
	 * TLS area provided here will be ignored and replaced
	 * with our own from TLS Manager component
	 */
	tlsmgr_area = crcalls.rump_tls_alloc(thread);
	bmk_memcpy(tlsmgr_area, tlsarea, TLSAREASIZE);

	thread->bt_cookie = cookie;
	thread->bt_wakeup_time = BMK_SCHED_BLOCK_INFTIME;

	inittcb(&thread->bt_tcb, tlsmgr_area, TCBOFFSET);
	initcurrent(tlsmgr_area, thread);

	/* RG set tls in gs register here */
	cos_thdcap = get_cos_thdcap(thread);
	crcalls.rump_tls_init((unsigned long)tlsmgr_area, cos_thdcap);

	TAILQ_INSERT_TAIL(&threadq, thread, bt_threadq);

	return thread;
}

struct bmk_thread *
bmk_sched_create(const char *name, void *cookie, int joinable,
	void (*f)(void *), void *data,
	void *stack_base, unsigned long stack_size)
{
	struct bmk_thread *ret;
	void *tlsarea;

	tlsarea = bmk_sched_tls_alloc();

	ret = bmk_sched_create_withtls(name, cookie, joinable, f, data,
				stack_base, stack_size, tlsarea);

	return ret;
}

struct join_waiter {
	struct bmk_thread *jw_thread;
	struct bmk_thread *jw_wanted;
	TAILQ_ENTRY(join_waiter) jw_entries;
};
static TAILQ_HEAD(, join_waiter) joinwq = TAILQ_HEAD_INITIALIZER(joinwq);

void
bmk_sched_exit_withtls(void)
{
	struct bmk_thread *thread = bmk_current;
	struct join_waiter *jw_iter;
	unsigned long flags;

	/* if joinable, gate until we are allowed to exit */
	flags = bmk_platform_splhigh();
	while (thread->bt_flags & THR_MUSTJOIN) {
		thread->bt_flags |= THR_JOINED;
		bmk_platform_splx(flags);

		/* see if the joiner is already there */
		TAILQ_FOREACH(jw_iter, &joinwq, jw_entries) {
			if (jw_iter->jw_wanted == thread) {
				bmk_sched_wake(jw_iter->jw_thread);
				break;
			}
		}
		bmk_sched_blockprepare();
		bmk_sched_block();
		flags = bmk_platform_splhigh();
	}

	bmk_cpu_sched_exit();

	/* FIXME: BMK REAPER FUNCTIONALITY LOST!! */
	bmk_platform_halt("schedule() returned for a dead thread!\n");
}

void
bmk_sched_exit(void)
{
	bmk_sched_tls_free((void *)bmk_current->bt_tcb.btcb_tp);
	bmk_sched_exit_withtls();
}

void
bmk_sched_join(struct bmk_thread *joinable)
{
	struct join_waiter jw;
	struct bmk_thread *thread = bmk_current;
	unsigned long flags;

	bmk_assert(joinable->bt_flags & THR_MUSTJOIN);

	flags = bmk_platform_splhigh();
	/* wait for exiting thread to hit thread_exit() */
	while ((joinable->bt_flags & THR_JOINED) == 0) {
		bmk_platform_splx(flags);

		jw.jw_thread = thread;
		jw.jw_wanted = joinable;
		TAILQ_INSERT_TAIL(&joinwq, &jw, jw_entries);
		bmk_sched_blockprepare();
		bmk_sched_block();
		TAILQ_REMOVE(&joinwq, &jw, jw_entries);

		flags = bmk_platform_splhigh();
	}

	/* signal exiting thread that we have seen it and it may now exit */
	bmk_assert(joinable->bt_flags & THR_JOINED);
	joinable->bt_flags &= ~THR_MUSTJOIN;
	bmk_platform_splx(flags);

	bmk_sched_wake(joinable);
}

/*
 * These suspend calls are different from block calls in the that
 * can be used to block other threads.  The only reason we need these
 * was because someone was clever enough to invent _np interfaces for
 * libpthread which allow randomly suspending other threads.
 */
void
bmk_sched_suspend(struct bmk_thread *thread)
{
	bmk_platform_halt("sched_suspend unimplemented");
}

void
bmk_sched_unsuspend(struct bmk_thread *thread)
{
	bmk_platform_halt("sched_unsuspend unimplemented");
}

void
bmk_sched_blockprepare_timeout(bmk_time_t deadline)
{
	struct bmk_thread *thread = bmk_current;

	thread->bt_wakeup_time = deadline;
}

void
bmk_sched_blockprepare(void)
{
	bmk_sched_blockprepare_timeout(BMK_SCHED_BLOCK_INFTIME);
}

int
bmk_sched_block(void)
{
	int ret = clear_runnable();

	return ret ? BMK_ETIMEDOUT : 0;
}

void
bmk_sched_wake(struct bmk_thread *thread)
{
	bmk_assert(thread != NULL);
	thread->bt_wakeup_time = BMK_SCHED_BLOCK_INFTIME;
	set_runnable(thread);
}

/*
 * Calculate offset of bmk_current early, so that we can use it
 * in thread creation.  Attempt to not depend on allocating the
 * TLS area so that we don't have to have malloc initialized.
 * We will properly initialize TLS for the main thread later
 * when we start the main thread (which is not necessarily the
 * first thread that we create).
 */
void
bmk_sched_init(void)
{
	unsigned long tlsinit;
	struct bmk_tcb tcbinit;

	tcboffset = TCBOFFSET;
	tdatasize = TDATASIZE;
	tbsssize  = TBSSSIZE;
	_tdata_start_cpy = _tdata_start;

	inittcb(&tcbinit, &tlsinit, 0);
	crcalls.rump_tls_init((&tcbinit)->btcb_tp, boot_thd);

	/*
	 * Not sure if the membars are necessary, but better to be
	 * Marvin the Paranoid Paradroid than get eaten by 999
	 */
	__asm__ __volatile__("" ::: "memory");
	bmk_curoff = (unsigned long)&bmk_current - (unsigned long)&tlsinit;
	__asm__ __volatile__("" ::: "memory");

	/*
	 * Set TLS back to 0 so that it's easier to catch someone trying
	 * to use it until we get TLS really initialized.
	 */
	tcbinit.btcb_tp = 0;
	crcalls.rump_tls_init((&tcbinit)->btcb_tp, boot_thd);
}

void __attribute__((noreturn))
bmk_sched_startmain(void (*mainfun)(void *), void *arg)
{
	struct bmk_thread *mainthread;

	mainthread = bmk_sched_create("main", NULL, 0,
	    mainfun, arg, bmk_mainstackbase, bmk_mainstacksize);
	if (mainthread == NULL)
		bmk_platform_halt("failed to create main thread");

	/* Make the RK intterupt thread */
	bmk_intr_init();

	/*
	 * Composite side scheduler!
	 * Fixed-Priority scheduler (non-preemptive at each PRIORITY)
	 * (Interrupts can preempt BMK thread execution but BMK thread execution will
	 * resume at the last preemption point before any other BMK thread is run).
	 * BMK threads explicitly yield to other BMK threads.
	 */
	crcalls.rump_resume();

	bmk_platform_halt("bmk_sched_init unreachable");
}

void
bmk_sched_set_hook(void (*f)(void *, void *))
{
	scheduler_hook = f;
}

struct bmk_thread *
bmk_sched_init_mainlwp(void *cookie)
{
	bmk_current->bt_cookie = cookie;
	return bmk_current;
}

const char *
bmk_sched_threadname(struct bmk_thread *thread)
{
	return thread->bt_name;
}

/*
 * XXX: this does not really belong here, but libbmk_rumpuser needs
 * to be able to set an errno, so we can't push it into libc without
 * violating abstraction layers.
 */
int *
bmk_sched_geterrno(void)
{
	return &bmk_current->bt_errno;
}

void
bmk_sched_yield(void)
{
	schedule();
}
