#include <rumpcalls.h>
#include <stdio.h>
#include <bmk-core/memalloc.h>
#include <bmk-core/types.h>

typedef long long bmk_time_t;

unsigned long bmk_stackpageorder;
unsigned long bmk_stacksize;

unsigned long bmk_pagesize;
unsigned long bmk_pageshift;

unsigned long bmk_memsize;

__thread struct bmk_thread *bmk_current;

struct cos_rumpcalls crcalls;

void* _GLOBAL_OFFSET_TABLE_ = (void *) 0x1337BEEF;


/* Prototypes */

struct bmk_thread;

void  bmk_printf(const char *fmt, ...);
void  bmk_sched_yield(void);
int  *bmk_sched_geterrno(void);
void  bmk_sched_yield(void);
void  bmk_sched_suspend(struct bmk_thread *thread);
void  bmk_sched_unsuspend(struct bmk_thread *thread);
void  bmk_sched_blockprepare_timeout(bmk_time_t deadline);
int   bmk_sched_block(void);
void  bmk_sched_wake(struct bmk_thread *thread);
void  bmk_sched_init(void);
void  bmk_sched_exit_withtls(void);
void  bmk_sched_blockprepare(void);
void *bmk_sched_tls_alloc(void);
void  bmk_sched_tls_free(void *mem);
void *bmk_sched_gettcb(void);
void  bmk_sched_set_hook(void (*f)(void *, void *));
void  bmk_sched_exit(void);
void  bmk_sched_join(struct bmk_thread *joinable);
struct bmk_thread *bmk_sched_create_withtls(const char *name, void *cookie, int joinable,
        void (*f)(void *), void *data,
        void *stack_base, unsigned long stack_size, void *tlsarea);
struct bmk_thread *bmk_sched_init_mainlwp(void *cookie);
struct bmk_thread *bmk_sched_create(const char *name, void *cookie, int joinable,
        void (*f)(void *), void *data,
        void *stack_base, unsigned long stack_size);
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

int *
bmk_sched_geterrno(void)
{
	bmk_printf("bmk_sched_geterrno is being called.\n");
	while(1){}
        return NULL;
}

void
bmk_sched_yield(void)
{
	bmk_printf("bmk_sched_yield is being called.\n");
	while(1){}
        return;
}

void
bmk_sched_suspend(struct bmk_thread *thread)
{
	bmk_printf("bmk_sched_suspend is being called.\n");
	while(1){}
	return;
}

void
bmk_sched_unsuspend(struct bmk_thread *thread)
{
	bmk_printf("bmk_sched_unsuspend is being called.\n");
	while(1){}
}

void
bmk_sched_blockprepare_timeout(bmk_time_t deadline)
{
	bmk_printf("bmk_sched_blockprepare is being called.\n");
	while(1){}
}

int
bmk_sched_block(void)
{
	bmk_printf("bmk_sched_block is being called.\n");
	while(1){}
}

void
bmk_sched_wake(struct bmk_thread *thread)
{
	bmk_printf("bmk_sched_wake is being called.\n");
	while(1){}
}

void
bmk_sched_init(void)
{
	bmk_printf("bmk_sched_init is being called.\n");
	while(1){}
}

struct bmk_thread *
bmk_sched_create_withtls(const char *name, void *cookie, int joinable,
        void (*f)(void *), void *data,
        void *stack_base, unsigned long stack_size, void *tlsarea)
{
	bmk_printf("bmk_sched_create_withtls is being called.\n");
	while(0){}
	return NULL;
}

void
bmk_sched_exit_withtls(void)
{
	bmk_printf("bmk_sched_exit_withtls is being called.\n");
	while(1){}
}

void
bmk_sched_blockprepare(void)
{
	bmk_printf("bmk_sched_blockprepare is being called.\n");
	while(0){}
}

void *
bmk_sched_tls_alloc(void)
{
	bmk_printf("bmk_sched_tls_alloc is being called.\n");
	while(1){}
	return NULL;
}

void
bmk_sched_tls_free(void *mem)
{
	bmk_printf("bmk_sched_tls_free is being called.\n");
	while(1){}
}

void *
bmk_sched_gettcb(void)
{
	bmk_printf("bmk_sched_gettcb is being called.\n");
	while(1){}
	return NULL;
}

void
bmk_sched_set_hook(void (*f)(void *, void *))
{
	bmk_printf("bmk_sched_set_hook is being called.\n");
	while(1){}
}

struct bmk_thread *
bmk_sched_init_mainlwp(void *cookie)
{
	bmk_printf("bmk_sched_init_mainlwp is being called.\n");
	while(1){}
	return NULL;
}

struct bmk_thread *
bmk_sched_create(const char *name, void *cookie, int joinable,
        void (*f)(void *), void *data,
        void *stack_base, unsigned long stack_size)
{
	bmk_printf("bmk_sched_create is being called.\n");
	while(1){}
	return NULL;
}

void
bmk_sched_exit(void)
{
	bmk_printf("bmk_sched_exit is being called.\n");
	while(1){}
}

void
bmk_sched_join(struct bmk_thread *joinable)
{
	bmk_printf("bmk_sched_join is being called.\n");
	while(1){}
}

bmk_time_t
bmk_platform_clock_monotonic(void)
{
	bmk_printf("bmk_platform_clock_monotonic is being called.\n");
	while(1){}
	return 0;
}

bmk_time_t
bmk_platform_clock_epochoffset(void)
{
	bmk_printf("bmk_platform_clock_epochoffset is being called.\n");
	while(1){}
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
	while(1){}
	return 0;
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
	while(1);
	return NULL;
}

void *
bmk_pgalloc(int order)
{
	bmk_printf("bmk_pgalloc is being called.\n");
	while(1);
	return NULL;
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
	bmk_printf("bmk_vprintf is being called.\n");
	while(1);
}

char *
bmk_strncpy(char *d, const char *s, unsigned long n)
{
	bmk_printf("bmk_strncpy is being called.\n");
	while(1);
	return NULL;
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
	bmk_printf("bmk_memalloc is being called.\n");
	while(1);
	return NULL;
}
