#include <bmk-core/memalloc.h>
#include <bmk-core/types.h>
#include <bmk-core/null.h>
#include <bmk-core/core.h>
#include <bmk-core/sched.h>
#include <execinfo.h>
//#include <bmk/kernel.h>

#include <arch/i386/types.h>

#include <rumpcalls.h>


// THESE MIGHT NEED TO BE DEFINED FIXMEE

typedef long long bmk_time_t;

unsigned long bmk_stackpageorder;
unsigned long bmk_stacksize;

unsigned long bmk_pagesize = 4096;
unsigned long bmk_pageshift = 12;

unsigned long bmk_memsize;

struct cos_rumpcalls crcalls;

void* _GLOBAL_OFFSET_TABLE_ = (void *) 0x1337BEEF;

int bmk_spldepth = 1;

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))

/* Prototypes */

struct bmk_thread;

struct bmk_tcb;

struct lwp *bmk_curlwp(void);
void  bmk_printf(const char *fmt, ...);
int bmk_snprintf(char *bf, unsigned long size, const char *fmt, ...);
bmk_time_t bmk_platform_clock_monotonic(void);
bmk_time_t bmk_platform_clock_epochoffset(void);
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
/* Prototype Definitions */

extern void *bmk_va2pa(void *addr);
extern void *bmk_pa2va(void *addr, unsigned long len);
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
{
	return;
}

unsigned long
bmk_platform_splhigh(void)
{
//	while(1);
	return 0;
}

void
bmk_platform_block(bmk_time_t until)
{
	bmk_time_t now = 0;

	/* Returned if called too late */
	now = bmk_platform_clock_monotonic();
	if(until < now) return;

	while(bmk_platform_clock_monotonic() < until);

	return;
}

void
bmk_platform_splx(unsigned long x)
{
	//while(1);
}

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
{
	crcalls.rump_cpu_sched_switch_viathd(prev, next);
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
	while(1);
}

int
bmk_strcmp(const char *a, const char *b)
{

	int rv;

	rv = crcalls.rump_strcmp(a, b);

	return rv;
}

/*void 
print_trace (void)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	for(i=0; i < size; i++)
		bmk_printf("%s\n", strings[i]);

}*/

void *
bmk_memset(void *b, int c, unsigned long n)
{
	unsigned char *v = b;

	while (n--)
		*v++ = (unsigned char)c;

	return b;

	// Changed in response to malloc bug. See notes
	//crcalls.rump_memset(b, c, n);
	//return b;
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

	//va_start(ap, fmt);
	ret = crcalls.rump_vsnprintf(s, len, fmt, ap);
	//va_end(ap);
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
//int pic2mask = 0xff;
//#define PIC2_DATA	0xa1

int
bmk_cpu_intr_init(int intr)
{
	bmk_printf("\nbmk_cpu_intr_init is being called: %d\n\n", intr);
//	while(1);

//	pic2mask &= ~(1<<(intr-8));
//	outb(PIC2_DATA, pic2mask);
	return 0;
}

void
bmk_cpu_intr_ack(void)
{
	//bmk_printf("BMK_CPU_INTR_ACK\n");
        /*
         * ACK interrupts on PIC
         */
        __asm__ __volatile(
            "movb $0x20, %%al\n"
            "outb %%al, $0xa0\n"
            "outb %%al, $0x20\n"
            ::: "al");

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
