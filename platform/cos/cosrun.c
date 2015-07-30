#include <rumpcalls.h>

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
	while(1){}
}

void *
bmk_memrealloc_user(void *cp, unsigned long nbytes)
{
	while(1){}
	return NULL;
}

int *
bmk_sched_geterrno(void)
{
	while(1){}
        return NULL;
}

void
bmk_sched_yield(void)
{
	while(1){}
        return;
}

void
bmk_sched_suspend(struct bmk_thread *thread)
{
	while(1){}
	return;
}

void
bmk_sched_unsuspend(struct bmk_thread *thread)
{
	while(1){}
}

void
bmk_sched_blockprepare_timeout(bmk_time_t deadline)
{
	while(1){}
}

int
bmk_sched_block(void)
{
	while(1){}
}

void
bmk_sched_wake(struct bmk_thread *thread)
{
	while(1){}
}

void
bmk_sched_init(void)
{
	while(1){}
}

struct bmk_thread *
bmk_sched_create_withtls(const char *name, void *cookie, int joinable,
        void (*f)(void *), void *data,
        void *stack_base, unsigned long stack_size, void *tlsarea)
{
	while(0){}
	return NULL;
}

void
bmk_sched_exit_withtls(void)
{
	while(1){}
}

void
bmk_sched_blockprepare(void)
{
	while(0){}
}

void *
bmk_sched_tls_alloc(void)
{
	while(1){}
	return NULL;
}

void
bmk_sched_tls_free(void *mem)
{
	while(1){}
}

void *
bmk_sched_gettcb(void)
{
	while(1){}
	return NULL;
}

void
bmk_sched_set_hook(void (*f)(void *, void *))
{
	while(1){}
}

struct bmk_thread *
bmk_sched_init_mainlwp(void *cookie)
{
	while(1){}
	return NULL;
}

struct bmk_thread *
bmk_sched_create(const char *name, void *cookie, int joinable,
        void (*f)(void *), void *data,
        void *stack_base, unsigned long stack_size)
{
	while(1){}
	return NULL;
}

void
bmk_sched_exit(void)
{
	while(1){}
}

void
bmk_sched_join(struct bmk_thread *joinable)
{
	while(1){}
}

bmk_time_t
bmk_platform_clock_monotonic(void)
{
	while(1){}
	return 0;
}

bmk_time_t
bmk_platform_clock_epochoffset(void)
{
	while(1){}
	return 0;
}

void __attribute__((noreturn))
bmk_platform_halt(const char *panicstring)
{
	while(1){}
}

int
bmk_strcmp(const char *a, const char *b)
{
	while(1){}
	return 0;
}

void *
bmk_memset(void *b, int c, unsigned long n)
{
	while(1){}
	return NULL;
}

void *
bmk_memcpy(void *d, const void *src, unsigned long n)
{
	while(1){}
	return NULL;
}


void *
bmk_memalloc(unsigned long nbytes, unsigned long align, enum bmk_memwho who)
{
	while(1);
	return NULL;
}

void *
bmk_pgalloc(int order)
{
	while(1);
	return NULL;
}

void
bmk_pgfree(void *pointer, int order)
{
	while(1);
	return;
}

void
bmk_vprintf(const char *fmt, va_list ap)
{
	while(1);
}

char *
bmk_strncpy(char *d, const char *s, unsigned long n)
{
	while(1);
	return NULL;
}

int
rumprun_platform_rumpuser_init(void)
{
	while(1);
	return 0;
}

int
bmk_isr_init(int (*func)(void *), void *arg, int intr)
{
	while(1);
	return 0;
}

void *
bmk_memcalloc(unsigned long n, unsigned long size, enum bmk_memwho who)
{
	while(1);
	return NULL;
}
