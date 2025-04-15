#include <extern.h>
#include <interrupts.h>
#include <machdep.h>

void rdtime(time_t * tloc)
{
	di();
	tloc->t_time = tod.t_time;
	tloc->t_date = tod.t_date;
	ei();
}

/* Port addresses of clock chip registers. */

#define SECS 0xe2
#define MINS 0xe3
#define HRS 0xe4
#define DAY 0xe6
#define MON 0xe7
#define YEAR 86

/* Read BCD clock register, convert to binary. */
uint16 tread(uint16 port)
{
	int n;

#define SECS_PER_MIN	60
#define MINS_PER_HOUR	60
#define HRS_PER_DAY	24
	switch (port) {
	case SECS:
		n = (tick_count / TICKSPERSEC) % SECS_PER_MIN;
		break;
	case MINS:
		n = (tick_count / TICKSPERSEC / SECS_PER_MIN) % MINS_PER_HOUR;
		break;
	case HRS:
		n = (tick_count / TICKSPERSEC / SECS_PER_MIN / MINS_PER_HOUR) %
		    HRS_PER_DAY;
		break;
	case DAY:
		n = 27;
		break;
	case MON:
		n = 9;
		break;
	}

	return n;
}

/* Update global time of day */
void rdtod(void)
{
	tod.t_time =
	    (tread(SECS) >> 1) | (tread(MINS) << 5) | (tread(HRS) << 11);
	tod.t_date = tread(DAY) | (tread(MON) << 5) | (YEAR << 9);
}

/* This adds two tick counts together.
The t_time field holds up to one second of ticks,
while the t_date field counts minutes */

void addtick(time_t * t1, time_t * t2)
{

	t1->t_time += t2->t_time;
	t1->t_date += t2->t_date;
	if (t1->t_time >= 60 * TICKSPERSEC) {
		t1->t_time -= 60 * TICKSPERSEC;
		++t1->t_date;
	}
}

void incrtick(time_t * t)
{
	if (++t->t_time == 60 * TICKSPERSEC) {
		t->t_time = 0;
		++t->t_date;
	}
}
