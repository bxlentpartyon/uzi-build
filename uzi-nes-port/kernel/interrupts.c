#include <extern.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <time.h>
#include <unix.h>

/* For now, this is the heart of how we tell time.  It only allows
 * us to count up to about 2 hours, but that's good enough to get
 * things moving. */
uint16 clk_int_count;
uint16 tick_count;

/* This is the clock interrupt routine.   Its job is to
increment the clock counters, increment the tick count of the
running process, and either swap it out if it has been in long enough
and is in user space or mark it to be swapped out if in system space.
Also it decrements the alarm clock of processes.
This must have no automatic or register variables */

extern void print_kb_bytes(void);

int clk_int(void)
{
	static ptptr p;
	int cur_hours, cur_mins, cur_secs;
	char tmp_hours[3], tmp_mins[3], tmp_secs[3];

#define INTS_PER_TICK	6
	if (clk_int_count < INTS_PER_TICK) {
		clk_int_count++;

/*
 * Disable the keyboard for now.  Echoing pressed keys to the screen in
 * interrupt context is problematic, since ppu_putc needs to take the ppu_lock.
 * I have a number of potential solutions for this, but there are other, more
 * important things to work on, before perfecting keyboard input.
 */
#if 0
		if (read_keyboard())
			dump_keyboard();
		else
			reset_prev_kb_rows();
#endif

		return 0;
	} else {
		clk_int_count = 0;
		tick_count++;
	}

	/* Increment processes and global tick counters */
	if (udata.u_ptab->p_status == P_RUNNING)
		incrtick(udata.u_insys ? &udata.u_stime : &udata.u_utime);

	incrtick(&ticks);

	/* Do once-per-second things */

	if (++sec == TICKSPERSEC)
	{
		/* Update global time counters */
		sec = 0;

		rdtod();  /* Update time-of-day */

#ifdef DEBUG_TIME
		cur_hours = (tod.t_time & 0xf800) >> 11;
		if (cur_hours < 10)
			sprintf(tmp_hours, "0%d", cur_hours);
		else
			sprintf(tmp_hours, "%d", cur_hours);

		cur_mins = (tod.t_time & 0x7e0) >> 5;
		if (cur_mins < 10)
			sprintf(tmp_mins, "0%d", cur_mins);
		else
			sprintf(tmp_mins, "%d", cur_mins);

		cur_secs =  (tod.t_time & 0x1f) << 1;
		if (cur_secs < 10)
			sprintf(tmp_secs, "0%d", cur_secs);
		else
			sprintf(tmp_secs, "%d", cur_secs);

		kprintf("%d/%d/%d %s:%s:%s\n", (tod.t_date & 0x3e0) >> 5,
						tod.t_date & 0x1f,
					       (tod.t_date & 0xfe00) >> 9,
						tmp_hours, tmp_mins, tmp_secs);
#endif

		/* Update process alarm clocks */
		for (p=ptab; p < ptab+PTABSIZE; ++p)
		{
			if (p->p_alarm)
				ifnot(--p->p_alarm)
					sendsig(p,SIGALRM);
		}
	}


#if 0
UZI-NES WIP
	/* Check run time of current process */
	if (++runticks >= MAXTICKS && !udata.u_insys)    /* Time to swap out */
	{
		udata.u_insys = 1;
		inint = 0;
		udata.u_ptab->p_status = P_READY;
		swapout();
		di();
		udata.u_insys = 0;      /* We have swapped back in */
	}
#endif
}

int nr_apu_irqs = 0;
extern unsigned char apu_status_byte;
#pragma zpsym ("apu_status_byte");

void handle_irq(void)
{
	if (apu_status_byte & 0x40) {
		nr_apu_irqs++;
		clk_int();
	} else {
		panic("spurious IRQ");
	}
}
