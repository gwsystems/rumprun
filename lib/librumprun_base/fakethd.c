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
#include <rumprun-base/fakethd.h>
#include "rumprun-private.h"

static void *
fake_thd_fn(void *d)
{
	assert(0);

	return NULL;
}

int
rk_fake_thd_creation(int instance, char *name)
{
	int ret;
	pthread_t t;

	assert(name);
//	printf("Creating a fake!! thread:%d, %s\n", instance, name);
	rumprun_set_meuserthd(name);

	/* this should call down to composite.. so I can now map that to some thread!! */
	/* TODO: instance use?? */
	ret = pthread_create(&t, NULL, fake_thd_fn, NULL);

	return ret;
}

