#ifndef __FSUTIL_TIME_H__
#define __FSUTIL_TIME_H__

void fsutil_rdtime(void *tloc);

struct fsutil_time_t {
	unsigned short t_time;
	unsigned short t_date;
};

#endif /* __FSUTIL_TIME_H__ */
