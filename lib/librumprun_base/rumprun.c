/*- * Copyright (c) 2015 Antti Kantee.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
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

#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/mount.h>
#include <sys/queue.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#include <fs/tmpfs/tmpfs_args.h>

#include <bmk-core/platform.h>
#include <bmk-core/printf.h>
#include <bmk-core/memalloc.h>

#include <bmk-core/sched.h> // FIXME, FOR DEBUGING

#include <rumprun-base/rumprun.h>
#include <rumprun-base/config.h>

#include "rumprun-private.h"
#include "cnic_init.h"

static pthread_mutex_t w_mtx;
static pthread_cond_t w_cv;

void bmk_fs_test(void);

int rumprun_enosys(void);
int
rumprun_enosys(void)
{
	return ENOSYS;
}

__strong_alias(rumprun_notmain,rumprun_enosys);
__weak_alias(rumpbake_main1,rumprun_notmain);
__weak_alias(rumpbake_main2,rumprun_notmain);
__weak_alias(rumpbake_main3,rumprun_notmain);
__weak_alias(rumpbake_main4,rumprun_notmain);
__weak_alias(rumpbake_main5,rumprun_notmain);
__weak_alias(rumpbake_main6,rumprun_notmain);
__weak_alias(rumpbake_main7,rumprun_notmain);
__weak_alias(rumpbake_main8,rumprun_notmain);

__weak_alias(rump_init_server,rumprun_enosys);

extern int rump_vmid;

void
rumprun_boot(char *cmdline)
{
	bmk_printf("rumprun_boot\n");

	/* Testing to see if gs was set correctly  */
	bmk_printf("\n\nbmk_current: %p\n", bmk_current);
	bmk_printf("&bmk_current: %p\n", &bmk_current);
	/* End of testing */

	struct tmpfs_args ta = {
		.ta_version = TMPFS_ARGS_VERSION,
		.ta_size_max = 1*1024*1024,
		.ta_root_mode = 01777,
	};
	int tmpfserrno;
	int puberrno;

	rump_boot_setsigmodel(RUMP_SIGMODEL_IGNORE);
	bmk_printf("Entering rump_init\n");
	rump_init();
	bmk_printf("Exit rump_init\n");

	/* mount /tmp before we let any userspace bits run */
	bmk_printf("Mounting temporary filesystem\n");
	rump_sys_mount(MOUNT_TMPFS, "/tmp", 0, &ta, sizeof(ta));
	tmpfserrno = errno;
	bmk_printf("Done Mounting temporary filesystem\n");

	/*
	 * XXX: _netbsd_userlevel_init() should technically be called
	 * in mainbouncer() per process.  However, there's currently no way
	 * to run it per process, and besides we need a fully functional
	 * libc to run sysproxy and rumprun_config(), so we just call it
	 * here for the time being.
	 *
	 * Eventually, we of course want bootstrap process which is
	 * rumprun() internally.
	 */
	bmk_printf("Rumprun_lwp_init\n");
	rumprun_lwp_init();
	bmk_printf("Exiting rumprun_lwp_init\n");
	_netbsd_userlevel_init();
	bmk_printf("Exiting _netbsd_userlevel_init\n");

	/* print tmpfs result only after we bootstrapped userspace */
	if (tmpfserrno == 0) {
		fprintf(stderr, "mounted tmpfs on /tmp\n");
	} else {
		warnx("FAILED: mount tmpfs on /tmp: %s\n", strerror(tmpfserrno));
	}

	bmk_printf("Setting up rump server\n");
	rump_init_server("tcp://0:12345");
	bmk_printf("Done setting up rump server\n");

	bmk_printf("Make /dev/paws device\n");
	puberrno = rump_pub_etfs_register("/dev/paws", "paws", RUMP_ETFS_BLK);
	if(puberrno)
		bmk_printf("rump_pub_etfs_register failed: %d\n", puberrno);

	if(cmdline){
		bmk_printf("Parsing cmdline for RK%d: %s\n", rump_vmid, cmdline);
		_rumprun_config(cmdline);
		bmk_printf("Done parsing cmdline\n");
	}


	/*
	 * give all threads a chance to run, and ensure that the main
	 * thread has gone through a context switch
	 */
	sched_yield();

	pthread_mutex_init(&w_mtx, NULL);
	pthread_cond_init(&w_cv, NULL);
}

/*
 * XXX: we have to use pthreads as the main threads for rumprunners
 * because otherwise libpthread goes haywire because it doesn't understand
 * the concept of multiple main threads (which is sort of understandable ...)
 */
#define RUMPRUNNER_DONE		0x01
#define RUMPRUNNER_DAEMON	0x02
struct rumprunner {
	int (*rr_mainfun)(int, char *[]);
	int rr_argc;
	char **rr_argv;

	pthread_t rr_mainthread;

	int rr_flags;

	LIST_ENTRY(rumprunner) rr_entries;
};
static LIST_HEAD(,rumprunner) rumprunners = LIST_HEAD_INITIALIZER(&rumprunners);
static int rumprun_done;

/* XXX: does not yet nuke any pthread that mainfun creates */
static void
releaseme(void *arg)
{
	struct rumprunner *rr = arg;

	pthread_mutex_lock(&w_mtx);
	rumprun_done++;
	rr->rr_flags |= RUMPRUNNER_DONE;
	pthread_cond_broadcast(&w_cv);
	pthread_mutex_unlock(&w_mtx);
}

static void *
mainbouncer(void *arg)
{
	struct rumprunner *rr = arg;
	/*
	 * RG: Removing program name for now...WE JUST WANT TO GET TO MAIN.. *CRAZY LAUGH*
	 * Please remember to put it back in..
	 */
	//const char *progname = rr->rr_argv[0];
	fprintf(stderr, "FIX PROGNAME\n");
	const char *progname = "slack_bot";
	int rv;

	rump_pub_lwproc_rfork(RUMP_RFFDG);

	pthread_cleanup_push(releaseme, rr);

	fprintf(stderr,"\n=== calling \"%s\" main() ===\n\n", progname);

	/* run main application */
	rv = rr->rr_mainfun(rr->rr_argc, rr->rr_argv);

	fflush(stdout);
	fprintf(stderr,"\n=== main() of \"%s\" returned %d ===\n",
	    progname, rv);

	pthread_cleanup_pop(1);

	/*
	 * XXX: missing _netbsd_userlevel_fini().  See comment in
	 * rumprun_boot()
	 */

	/* exit() calls rumprun_pub_lwproc_releaselwp() (via pthread_exit()) */
	exit(rv);
}

int shm_allocate(unsigned int spdid, unsigned int num_pages);
unsigned long shm_get_vaddr(unsigned int spdid, unsigned int id);

void thread_test(void);
static void *test_func(void *);


static void *
test_func(void *unused)
{
	printf("IN TEST FUNC! PID: %d\n", rump_sys_getpid());
	printf("Calling pthread exit: %d\n", rump_sys_getpid());
	pthread_exit((void *)"GOOD\0");
	return NULL;
}

void
thread_test(void)
{
	pthread_t thd;
	int ret;
	void *thd_ret;
	struct lwp *old;

	/* Create a lwp context for our thread */
	assert(!rump_pub_lwproc_newlwp(rump_sys_getpid()));
	old = rump_pub_lwproc_curlwp();
	assert(old);

	printf("Current pid: %d\n", rump_sys_getpid());
	printf("Calling pthread_create for pid: %d\n", rump_sys_getpid());
	ret = pthread_create(&thd, NULL, test_func, NULL);
	printf("Ret from pthread_create: %d\n", ret);

	printf("Calling pthread_join\n");
	ret = pthread_join(thd, &thd_ret);
	printf("ret from pthread_join: %s\n", (char *)thd_ret);
	assert(!ret);

	printf("testing rfork...\n");
	rump_pub_lwproc_rfork(RUMP_RFFDG);
	printf("done testing rfork\n");
	printf("Current pid: %d\n", rump_sys_getpid());

	printf("Calling pthread_create for pid: %d\n", rump_sys_getpid());
	ret = pthread_create(&thd, NULL, test_func, NULL);
	printf("Ret from pthread_create: %d\n", ret);

	/* Switch back to first process */
	rump_pub_lwproc_switch(old);
}

void *
rumprun(int (*mainfun)(int, char *[]), int argc, char *argv[])
{
	printf("\n__________________rumprun_________________\n");
	/* Test threading and process apis */
	//thread_test();

	struct rumprunner *rr;

	rr = malloc(sizeof(*rr));

	/* XXX: should we deep copy argc? */
	rr->rr_mainfun = mainfun;
	rr->rr_argc = argc;
	rr->rr_argv = argv;
	rr->rr_flags = 0;

	rumprun_set_meuserthd();
	printf("pthread_create\n");
	if (pthread_create(&rr->rr_mainthread, NULL, mainbouncer, rr) != 0) {
		fprintf(stderr, "rumprun: running %s failed\n", argv[0]);
		free(rr);
		return NULL;
	}
	printf("pthread_create done\n");
	LIST_INSERT_HEAD(&rumprunners, rr, rr_entries);

	pthread_mutex_lock(&w_mtx);
	while ((rr->rr_flags & (RUMPRUNNER_DONE|RUMPRUNNER_DAEMON)) == 0) {
		pthread_cond_wait(&w_cv, &w_mtx);
	}
	pthread_mutex_unlock(&w_mtx);

	if (rr->rr_flags & RUMPRUNNER_DONE) {
		rumprun_wait(rr);
		rr = NULL;
	}

	return rr;
}

int
rumprun_wait(void *cookie)
{
	struct rumprunner *rr = cookie;
	void *retval;

	pthread_join(rr->rr_mainthread, &retval);
	LIST_REMOVE(rr, rr_entries);
	free(rr);

	assert(rumprun_done > 0);
	rumprun_done--;

	return (int)(intptr_t)retval;
}

void *
rumprun_get_finished(void)
{
	struct rumprunner *rr;

	if (LIST_EMPTY(&rumprunners))
		return NULL;

	pthread_mutex_lock(&w_mtx);
	while (rumprun_done == 0) {
		pthread_cond_wait(&w_cv, &w_mtx);
	}
	LIST_FOREACH(rr, &rumprunners, rr_entries) {
		if (rr->rr_flags & RUMPRUNNER_DONE) {
			break;
		}
	}
	pthread_mutex_unlock(&w_mtx);
	assert(rr);

	return rr;
}

/*
 * Detaches current program.  Must always be called from
 * the main thread of an application.
 *
 * XXX: there's no public prototype for this for now.
 */
void
rumprun_daemon(void)
{
	struct rumprunner *rr;

	LIST_FOREACH(rr, &rumprunners, rr_entries) {
		if (rr->rr_mainthread == pthread_self())
			break;
	}
	assert(rr);

	pthread_mutex_lock(&w_mtx);
	rr->rr_flags |= RUMPRUNNER_DAEMON;
	pthread_cond_broadcast(&w_cv);
	pthread_mutex_unlock(&w_mtx);
}

void __dead
rumprun_reboot(void)
{

	_rumprun_deconfig();
	_netbsd_userlevel_fini();
	rump_sys_reboot(0, 0);

	bmk_platform_halt("reboot returned");
}
