#include <unix.h>

#include "fsutil.h"
#include "fsutil_machdep.h"

void warning(char *msg)
{
	fsutil_printf("WARNING: %s\n", msg);
}

void panic(char *msg)
{
	fsutil_panic(msg);
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

int valadr(char *base, uint16 size)
{
	return 1;
}
