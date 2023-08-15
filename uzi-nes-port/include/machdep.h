#ifndef __MACHDEP_H__
#define __MACHDEP_H__

void kprintf(char *fmt, ...);
void ei(void);
void di(void);
void start_clock(void);
void stop_clock(void);

#endif /* __MACHDEP_H__ */
