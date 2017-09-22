#include <bmk-core/memalloc.h>
#include <bmk-core/types.h>
#include <bmk-core/null.h>
#include <bmk-core/core.h>
#include <bmk-core/sched.h>
#include <bmk-core/queue.h>
#include <execinfo.h>
#include "cosrun.h"

#include <arch/i386/types.h>

#include <rumpcalls.h>

int bmk_spldepth = 1;

typedef long long bmk_time_t;

unsigned long bmk_stackpageorder;
unsigned long bmk_stacksize;

unsigned long bmk_pagesize = 4096;
unsigned long bmk_pageshift = 12;

unsigned long bmk_memsize;

struct cos_rumpcalls crcalls;

void* _GLOBAL_OFFSET_TABLE_ = (void *) 0x1337BEEF;

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))

/* Prototypes */

struct bmk_thread;

struct bmk_tcb;

struct lwp *bmk_curlwp(void);
void  bmk_printf(const char *fmt, ...);
int bmk_snprintf(char *bf, unsigned long size, const char *fmt, ...);
bmk_time_t bmk_platform_clock_monotonic(void);
bmk_time_t bmk_platform_clock_epochoffset(void);
bmk_time_t vm_platform_clock_monotonic(void);
void __attribute__((noreturn)) bmk_platform_halt(const char *panicstring);
int bmk_strcmp(const char *a, const char *b);
void *bmk_memset(void *b, int c, unsigned long n);
void *bmk_memcpy(void *d, const void *src, unsigned long n);
void *bmk_pgalloc(int order);
void  bmk_pgfree(void *pointer, int order);
void  bmk_vprintf(const char *fmt, va_list ap);
char *bmk_strncpy(char *d, const char *s, unsigned long n);
int   rumprun_platform_rumpuser_init(void);
int   bmk_isr_init(int (*func)(void *), void *arg, int intr);

void print_trace(void);

void bmk_platform_cpu_sched_settls(struct bmk_tcb *next);
unsigned long bmk_platform_splhigh(void);
void bmk_platform_block(bmk_time_t until);
void bmk_cpu_sched_create(struct bmk_thread *thread, struct bmk_tcb *tcb, void (*f)(void *), void *arg, void *stack_base, unsigned long stack_size);
char * bmk_strcpy(char *d, const char *s);
void bmk_platform_splx(unsigned long);
void bmk_cpu_sched_switch_viathd(struct bmk_thread *prev, struct bmk_thread *next);

int bmk_cpu_intr_init(int intr);
void bmk_cpu_intr_ack(void);

void bmk_fs_test(void);

void rumpns_bmk_printf(const char *fmt, ...) __attribute__ ((weak, alias ("bmk_printf")));

/* Prototype Definitions */
extern void *bmk_va2pa(void *addr);
extern void *bmk_pa2va(void *addr, unsigned long len);

/* scheduler api */
void bmk_cpu_sched_wakeup(struct bmk_thread *thread);
int  bmk_cpu_sched_block_timeout(struct bmk_thread *curr, unsigned long long timeout);
void bmk_cpu_sched_block(struct bmk_thread *curr);
void bmk_cpu_sched_yield(void);
void bmk_cpu_sched_exit(void);

int
rump_shmem_dequeue_size(unsigned int srcvm, unsigned int dstvm)
{ return crcalls.rump_dequeue_size(srcvm, dstvm); }

int
rump_shmem_write(void *buff, unsigned int size, unsigned int srcvm, unsigned int dstvm)
{ return crcalls.rump_shmem_send(buff, size, srcvm, dstvm); }

void *
rump_shmem_read(void *buff, unsigned int srcvm, unsigned int dstvm)
{

	if(crcalls.rump_shmem_recv(buff, srcvm, dstvm) == -1) return NULL;
	return buff;
}

void
bmk_printf(const char *fmt, ...)
{
	char s[128];
	va_list arg_ptr;
	int ret, len = 128;

	va_start(arg_ptr, fmt);
	ret = crcalls.rump_vsnprintf(s, len, fmt, arg_ptr);
	va_end(arg_ptr);
	crcalls.rump_cos_print(s, ret);

	return;
}

void*
bmk_va2pa(void *addr)
{
	return crcalls.rump_va2pa(addr);
}

void*
bmk_pa2va(void *addr, unsigned long len)
{
	return crcalls.rump_pa2va(addr, len);
}

int
bmk_snprintf(char *bf, unsigned long size, const char *fmt, ...)
{

	int ret;
	va_list arg_ptr;

	va_start(arg_ptr, fmt);
	ret = crcalls.rump_vsnprintf(bf, size, fmt, arg_ptr);
	va_end(arg_ptr);

	return ret;
}

void
bmk_platform_cpu_sched_settls(struct bmk_tcb *next)
{ return; }

unsigned long
bmk_platform_splhigh(void)
{
	crcalls.rump_intr_disable();
	return 0;
}

TAILQ_HEAD(threadqueue, bmk_thread);
bmk_time_t time_blocked = 0;

void
bmk_platform_block(bmk_time_t until)
{
	bmk_time_t now = 0;

	/* Returned if called too late */
	now = bmk_platform_clock_monotonic();
	if(until < now) return;

	/* Enable interupts around yield */
	bmk_platform_splx(0);

	/* if there is nothing in runq, blocking this should not run any BMK thread */
	bmk_cpu_sched_block_timeout(bmk_current, until);

	bmk_platform_splhigh();

	return;
}

void
bmk_platform_splx(unsigned long x)
{ crcalls.rump_intr_enable(); }

void
bmk_cpu_sched_create(struct bmk_thread *thread, struct bmk_tcb *tcb,
        void (*f)(void *), void *arg,
        void *stack_base, unsigned long stack_size)
{

	/*
	 * In the composite related function we want to:
	 * We need to get the thdid so we can set up bmk_current thd
	 * later on *initcurrent*, We want to assign thdcap_t within the
	 * thread that is being passed in.
	 * */
	crcalls.rump_cpu_sched_create(thread, tcb, f, arg, stack_base, stack_size);
}

char *
bmk_strcpy(char *d, const char *s)
{
	char *orig = d;

	while ((*d++ = *s++) != '\0')
		continue;
	return orig;
}

void
bmk_cpu_sched_switch_viathd(struct bmk_thread *prev, struct bmk_thread *next)
{ crcalls.rump_cpu_sched_switch_viathd(prev, next); }

bmk_time_t
vm_platform_clock_monotonic(void)
{
	bmk_time_t cur_time;

	/* bmk_time_t is just a long long */
	cur_time = (bmk_time_t)crcalls.rump_vm_clock_now();

	return cur_time;
}

bmk_time_t
bmk_platform_clock_monotonic(void)
{
	bmk_time_t cur_time;

	/* bmk_time_t is just a long long */
	cur_time = (bmk_time_t)crcalls.rump_cpu_clock_now();

	return cur_time;
}

bmk_time_t
bmk_platform_clock_epochoffset(void)
{
	bmk_printf("bmk_platform_clock_epochoffset is being called.\n");

	return 0;
}

void __attribute__((noreturn))
bmk_platform_halt(const char *panicstring)
{
	bmk_printf("bmk_platform_halt is being called.\n");
	bmk_printf("It's message is: ");
	bmk_printf(panicstring);
	crcalls.rump_platform_exit();

	while (1) ;
}

int
bmk_strcmp(const char *a, const char *b)
{

	int rv;

	rv = crcalls.rump_strcmp(a, b);

	return rv;
}

void *
bmk_memset(void *b, int c, unsigned long n)
{
	unsigned char *v = b;

	while (n--)
		*v++ = (unsigned char)c;

	return b;
}

void *
bmk_memcpy(void *d, const void *src, unsigned long n)
{

	void *ret;

	ret = crcalls.rump_memcpy(d, src, n);

	return ret;
}


void
bmk_vprintf(const char *fmt, va_list ap)
{

	char s[128];
	int ret, len = 128;

	ret = crcalls.rump_vsnprintf(s, len, fmt, ap);
	crcalls.rump_cos_print(s, ret);

	return;
}

char *
bmk_strncpy(char *d, const char *s, unsigned long n)
{

	char *rv;

	rv = crcalls.rump_strncpy(d, s, n);

	return rv;
}

int
bmk_cpu_intr_init(int intr)
{ return 0; }

void
bmk_cpu_intr_ack(void)
{
        /*
         * ACK interrupts on PIC
         */
	crcalls.rump_cpu_intr_ack();
}

int
rumprun_platform_rumpuser_init(void)
{ return 0; }

void
bmk_fs_test(void)
{
	/* Discontinued, now implemented as a syscall into RK */
	//crcalls.rump_fs_test();
}

void
bmk_cpu_sched_wakeup(struct bmk_thread *thread)
{
	crcalls.rump_cpu_sched_wakeup(thread);
}

int
bmk_cpu_sched_block_timeout(struct bmk_thread *curr, unsigned long long timeout)
{
	return crcalls.rump_cpu_sched_block_timeout(curr, timeout);
}

void
bmk_cpu_sched_block(struct bmk_thread *curr)
{
	crcalls.rump_cpu_sched_block(curr);
}

void
bmk_cpu_sched_yield(void)
{
	crcalls.rump_cpu_sched_yield();
}

void
bmk_cpu_sched_exit(void)
{
	crcalls.rump_cpu_sched_exit();
	/* should not return here! */
	bmk_assert(0);
}
