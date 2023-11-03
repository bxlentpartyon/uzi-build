#ifndef __MACHDEP_H__
#define __MACHDEP_H__

#include <unix.h>

void kprintf(char *fmt, ...);
void sprintf(char *str, char *fmt, ...);
void ei(void);
void di(void);
void rdtime(time_t *tloc);
void start_clock(void);
void stop_clock(void);

/* time functions */
void rdtod(void);
void addtick(time_t *t1, time_t *t2);
void incrtick(time_t *t);

#endif /* __MACHDEP_H__ */
