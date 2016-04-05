#include <bmk/kernel.h>

#include <rumprun-base/rumprun.h>
#include <rumprun-base/config.h>
#include <bmk-core/printf.h>

/* Located on the composite side */
#include <cos_init.h>


/* By the time this function is called, the main threadh as already been created
 * and the rumpkernel is running.
 * We are in rumpkernel context.
 */

void
bmk_mainthread(void *cmdline)
{
	void *cookie;

	bmk_printf("cmdline: \n%s", (char *)cmdline);

	rumprun_boot(cmdline);

	RUNMAINS();

	while ((cookie = rumprun_get_finished()) != NULL)
		rumprun_wait(cookie);

	rumprun_reboot();
}
