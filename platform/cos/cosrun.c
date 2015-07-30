#include <rumpcalls.h>

struct cos_rumpcalls crcalls;

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

int
test_function(void)
{
	return 0;
}
