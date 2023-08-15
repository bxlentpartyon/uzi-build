#ifndef __MACHDEP_H__
#define __MACHDEP_H__

#include <unix.h>

void kprintf(char *fmt, ...);
void ei(void);
void di(void);
void rdtime(time_t *tloc);
void start_clock(void);
void stop_clock(void);

#endif /* __MACHDEP_H__ */
