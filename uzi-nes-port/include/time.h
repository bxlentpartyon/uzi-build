#ifndef __TIME_H__
#define __TIME_H__

void rdtime(time_t * tloc);

/* time functions */
void rdtod(void);
void addtick(time_t * t1, time_t * t2);
void incrtick(time_t * t);

#endif				/* __TIME_H__ */
