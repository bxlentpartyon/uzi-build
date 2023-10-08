#include <unix.h>
#include <extern.h>
#include <devio.h>
#include <filesys.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <lib/string.h>

/* Newproc fixes up the tables for the child of a fork */
void newproc(ptptr p)
{
    register char *j;

    /* Note that ptab_alloc clears most of the entry */
    di();
    p->p_swap = (p - ptab) * 65  + 1;  /* Allow 65 blocks per process */
    p->p_status = P_RUNNING;

    p->p_pptr = udata.u_ptab;
    p->p_ignored = udata.u_ptab->p_ignored;
    p->p_uid = udata.u_ptab->p_uid;
    udata.u_ptab = p;
    bzero((char *) &udata.u_utime, 4 * sizeof(time_t)); /* Clear tick counters */
    ei();

    rdtime(&udata.u_time);
    i_ref(udata.u_cwd);
    udata.u_cursig = udata.u_error = 0;

    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
	if (*j >= 0)
	   ++of_tab[*j].o_refs;
}

/* This allocates a new process table slot, and fills
in its p_pid field with a unique number.  */
ptptr
ptab_alloc()
{
    register ptptr p;
    register ptptr pp;
    static int nextpid = 0;

    di();
    for(p=ptab;p < ptab+PTABSIZE; ++p)
    {
	if (p->p_status == P_EMPTY)
	    goto found;
    }
    ei();
    return(NULL);

found:

    /* See if next pid number is unique */
nogood:
    if (nextpid++ > 32000)
	nextpid = 1;
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
	if (pp->p_status != P_EMPTY && pp->p_pid == nextpid)
	    goto nogood;
    }

    bzero((char *) p, sizeof(struct p_tab));
    p->p_pid = nextpid;
    p->p_status = P_FORKING;
    ei();
    return (p);
}

void dump_proc(ptptr proc)
{
	kprintf("P %d U %d T %x r %x A %d X %d W %x R %d S %x I %x\n",
		proc->p_pid, proc->p_uid, proc->p_status, proc->p_pptr,
		proc->p_alarm, proc->p_exitval, proc->p_wait, proc->p_priority,
		proc->p_pending, proc->p_ignored);
}

void init2(void)
{
	register char *j;

	bufinit();

	/* Create the context for the first process */
	initproc = ptab_alloc();
	udata.u_ptab = initproc;
	newproc(initproc);
	initproc->p_status = P_RUNNING;

	/* User's file table */
	for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
		*j = -1;

	/* Turn on the clock */
	start_clock();
	ei();

	kprintf("boot:\n");

	dump_proc(initproc);

	while(1);
}

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

int clk_int(void)
{
	static ptptr p;

#define INTS_PER_TICK	6
	if (clk_int_count < INTS_PER_TICK) {
		clk_int_count++;
		return 0;
	} else {
		clk_int_count = 0;
		tick_count++;
	}

	stop_clock();

#if 0
UZI-NES WIP
	/* Increment processes and global tick counters */
	if (udata.u_ptab->p_status == P_RUNNING)
		incrtick(udata.u_insys ? &udata.u_stime : &udata.u_utime);

	incrtick(&ticks);
#endif

	/* Do once-per-second things */

	if (++sec == TICKSPERSEC)
	{
		/* Update global time counters */
		sec = 0;

		rdtod();  /* Update time-of-day */

		kprintf("%d/%d/%d %d:%d:%d\n", (tod.t_date & 0x3e0) >> 5,
						tod.t_date & 0x1f,
					       (tod.t_date & 0xfe00) >> 9,
					       (tod.t_time & 0xf800) >> 11,
					       (tod.t_time & 0x7e0) >> 5,
					       (tod.t_time & 0x1f) << 1);

#if 0
UZI-NES WIP
		/* Update process alarm clocks */
		for (p=ptab; p < ptab+PTABSIZE; ++p)
		{
			if (p->p_alarm)
				ifnot(--p->p_alarm)
					sendsig(p,SIGALRM);
		}
#endif
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

	start_clock();
}
