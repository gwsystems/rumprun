#include <bmk-core/memalloc.h>
#include <bmk-core/types.h>
#include <bmk-core/null.h>
#include <bmk-core/core.h>

//#include <bmk/kernel.h>

#include <arch/i386/types.h>

#include <rumpcalls.h>

typedef long long bmk_time_t;

unsigned long bmk_stackpageorder;
unsigned long bmk_stacksize;

unsigned long bmk_pagesize = 4096;
unsigned long bmk_pageshift;

unsigned long bmk_memsize;

struct cos_rumpcalls crcalls;

void* _GLOBAL_OFFSET_TABLE_ = (void *) 0x1337BEEF;


/* Prototypes */

struct bmk_thread;

struct lwp *bmk_curlwp(void);
void  bmk_printf(const char *fmt, ...);
bmk_time_t bmk_platform_clock_monotonic(void);
bmk_time_t bmk_platform_clock_epochoffset(void);
void __attribute__((noreturn)) bmk_platform_halt(const char *panicstring);
int bmk_strcmp(const char *a, const char *b);
void  bmk_memfree(void *cp, enum bmk_memwho who);
void *bmk_memrealloc_user(void *cp, unsigned long nbytes);
void *bmk_memset(void *b, int c, unsigned long n);
void *bmk_memcpy(void *d, const void *src, unsigned long n);
void *bmk_memalloc(unsigned long nbytes, unsigned long align, enum bmk_memwho who);
void *bmk_memcalloc(unsigned long, unsigned long, enum bmk_memwho);
void *bmk_pgalloc(int order);
void  bmk_pgfree(void *pointer, int order);
void  bmk_vprintf(const char *fmt, va_list ap);
char *bmk_strncpy(char *d, const char *s, unsigned long n);
int   rumprun_platform_rumpuser_init(void);
int   bmk_isr_init(int (*func)(void *), void *arg, int intr);

/* Prototype Definitions */

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

void
bmk_memfree(void *cp, enum bmk_memwho who)
{
	bmk_printf("bmk_memfree is being called.\n");
	while(1){}
}

void *
bmk_memrealloc_user(void *cp, unsigned long nbytes)
{
	bmk_printf("bmk_memreallac_user is being called.\n");
	while(1){}
	return NULL;
}

bmk_time_t
bmk_platform_clock_monotonic(void)
{
	bmk_printf("bmk_platform_clock_monotonic is being called.\n");

	return 0;
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
	while(1){}
}

int
bmk_strcmp(const char *a, const char *b)
{
	bmk_printf("bmk_strcmp is being called.\n");

	int rv;

	rv = crcalls.rump_strcmp(a, b);

	return rv;
}

void *
bmk_memset(void *b, int c, unsigned long n)
{
	bmk_printf("bmk_memset is being called.\n");
	while(1){}
	return NULL;
}

void *
bmk_memcpy(void *d, const void *src, unsigned long n)
{
	bmk_printf("bmk_memcpy is being called.\n");
	while(1){}
	return NULL;
}


void *
bmk_memalloc(unsigned long nbytes, unsigned long align, enum bmk_memwho who)
{
	bmk_printf("bmk_memalloc is being called.\n");
	void *rv;

	rv = crcalls.rump_memalloc(nbytes, align);
	return rv;
}

void *
bmk_pgalloc(int order)
{
	bmk_printf("bmk_pgalloc is being called.\n");
	bmk_printf("bmk_pgalloc order: %d\n", order);
	return crcalls.rump_pgalloc();
}

void
bmk_pgfree(void *pointer, int order)
{
	bmk_printf("bmk_pgfree is being called.\n");
	while(1);
	return;
}

void
bmk_vprintf(const char *fmt, va_list ap)
{

	char s[128];
	int ret, len = 128;

	//va_start(ap, fmt);
	ret = crcalls.rump_vsnprintf(s, len, fmt, ap);
	//va_end(ap);
	crcalls.rump_cos_print(s, ret);

	return;

}

char *
bmk_strncpy(char *d, const char *s, unsigned long n)
{
	bmk_printf("bmk_strncpy is being called.\n");

	char *rv;

	rv = crcalls.rump_strncpy(d, s, n);

	return rv;
}

/* RG:
 * This simply returns 0 within the hw implementation.
 * The xen implementation uses this to begin bio
 */

int
rumprun_platform_rumpuser_init(void)
{
	bmk_printf("rumprun_platform_rumpuser_init is being called.\n");
	return 0;
}

int
bmk_isr_init(int (*func)(void *), void *arg, int intr)
{
	bmk_printf("bmk_isr_init is being called.\n");
	while(1);
	return 0;
}

void *
bmk_memcalloc(unsigned long n, unsigned long size, enum bmk_memwho who)
{
	bmk_printf("bmk_memcalloc is being called.\n");

	void *rv;

	rv = crcalls.rump_memcalloc(n, size);
	return rv;
}

void
bmk_platform_cpu_sched_settls(struct bmk_tcb *next)
{
	bmk_printf("bmk_platform_cpu_sched_settls is being called.\n");
	while(1);
}

unsigned long
bmk_platform_splhigh(void)
{
	bmk_printf("bmk_platform_splhigh is being called.\n");
	return 0;
}

void *
bmk_xmalloc_bmk(unsigned long howmuch)
{
	bmk_printf("bmk_xmalloc_bmk is being called.\n");
	return NULL;
}
