#include <bmk/kernel.h>

#include <rumprun-base/rumprun.h>
#include <rumprun-base/config.h>

/* Located on the composite side */
#include <cos_init.h>

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
