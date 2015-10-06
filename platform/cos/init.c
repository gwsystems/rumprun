#include <bmk/kernel.h>

#include <rumprun-base/rumprun.h>
#include <rumprun-base/config.h>

/* Located on the composite side */
#include <cos_init.h>


/* By the time this function is called, the main threadh as already been created
 * and the rumpkernel is running. */

void
bmk_mainthread(void *cmdline)
{
	void *cookie;

	rumprun_boot(cmdline);

	RUNMAINS();

	while ((cookie = rumprun_get_finished()) != NULL)
		rumprun_wait(cookie);

	rumprun_reboot();
}
