#ifndef __MACHDEP_H__
#define __MACHDEP_H__

#include <unix.h>

void kprintf(char *fmt, ...);
void sprintf(char *str, char *fmt, ...);
void ei(void);
void di(void);
int valadr(char *base, uint16 size);
void tempstack(void);
void warning(char *s);
void start_clock(void);
void stop_clock(void);

#endif /* __MACHDEP_H__ */
