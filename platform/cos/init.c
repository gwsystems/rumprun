#include <bmk/kernel.h>

#include <rumprun-base/rumprun.h>
#include <rumprun-base/config.h>
#include <bmk-core/printf.h>

/* By the time this function is called, the main threadh as already been created
 * and the rumpkernel is running.
 * We are in rumpkernel context.
 */
int rump_vmid;
extern unsigned int rump_dom0_rcv;

void
bmk_mainthread(void *cmdline)
{
	void *cookie;
	extern int spdid;
	/*
	 * FIXME vmid has been discontinued, add rumpcall down to composite to fetch new spdid
	 * Use the preexisting sinv, just add another option in addition to the existing yield
	 * and cap_frontier call
	 */
	rump_vmid = spdid;
	bmk_printf("Starting bmk_mainthread\n");
	bmk_printf("cmdline: \n%s", (char *)cmdline);

	rumprun_boot(cmdline);
	bmk_printf("Starting applications.\n");
	RUNMAINS();
	bmk_printf("Done creating applications\n");

	while ((cookie = rumprun_get_finished()) != NULL)
		rumprun_wait(cookie);

	rumprun_reboot();
}
