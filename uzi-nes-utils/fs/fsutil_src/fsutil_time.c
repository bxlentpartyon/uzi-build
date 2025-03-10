#include <unix.h>

#include "fsutil_time.h"

void rdtime(time_t *tloc)
{
	fsutil_rdtime((void *)tloc);
}
