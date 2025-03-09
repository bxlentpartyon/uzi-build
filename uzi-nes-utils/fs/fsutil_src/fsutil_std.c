#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "fsutil.h"

int fsutil_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	printf(format, args);
	va_end(args);
}

void warning(char *msg)
{
	printf("WARNING: %s\n", msg);
}

void panic(char *msg)
{
	printf("%s\n", msg);
	assert(0);
}

void ei(void)
{
	return;
}

void di(void)
{
	return;
}

void wakeup(char *event)
{
	return;
}

void psleep(void *event)
{
	panic("shouldn't be sleeping");
}
