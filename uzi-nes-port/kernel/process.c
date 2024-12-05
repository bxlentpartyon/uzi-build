#include <unix.h>
#include <extern.h>
#include <devio.h>
#include <devtty.h>
#include <filesys.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <scall.h>
#include <lib/string.h>
#include <kb.h>

extern char ppu_readbuf_dirty;
extern char ppu_readbuf[];

int swapout(void);

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

/* Getproc returns the process table pointer of a runnable process.
It is actually the scheduler.
If there are none, it loops.  This is the only time-wasting loop in the
system. */

ptptr getproc(void)
{
    register status;
    static ptptr pp = ptab;    /* Pointer for round-robin scheduling */

    for (;;)
    {
	if (++pp >= ptab + PTABSIZE)
	    pp = ptab;

	di();
	status = pp->p_status;
	ei();

	if (status == P_RUNNING)
	    panic("getproc: extra running");
	if (status == P_READY)
	    return(pp);
    }
}

void swrite(void)
{
	return;
}

void swapin(ptptr pp)
{
	return;
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
	for(p=ptab;p < ptab+PTABSIZE; ++p) {
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
	kprintf("P %d U %d T %x r %x A %x X %d W %x R %d S %x I %x\n",
		proc->p_pid, proc->p_uid, proc->p_status, proc->p_pptr,
		proc->p_alarm, proc->p_exitval, proc->p_wait, proc->p_priority,
		proc->p_pending, proc->p_ignored);
}

void init2(void)
{
	register char *j;
	static char bootchar;

	bufinit();

	/* Create the context for the first process */
	initproc = ptab_alloc();
	udata.u_ptab = initproc;
	newproc(initproc);
	dump_proc(initproc);
	initproc->p_status = P_RUNNING;

	/* User's file table */
	for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
		*j = -1;

	/* Turn on the clock */
	start_clock();
	ei();

	/* Wait until the clock has interrupted, to set tod */
	while (!tod.t_date);

	/* Open the console tty device */
	tty_init();
	if (d_open(TTYDEV) != 0)
		panic("no tty");

	kprintf("boot:\n");
	udata.u_base = &bootchar;
	udata.u_count = 1;
	cdread(TTYDEV);
	ROOTDEV = bootchar - '0';

	dump_proc(initproc);

	test_ppu_read();
	kprintf("char %c\n", ppu_readbuf[0]);
	while(1)
		ppu_readbuf_dirty = 0;
}

/* psleep() puts a process to sleep on the given event.
If another process is runnable, it swaps out the current one
and starts the new one.
Normally when psleep is called, the interrupts have already been
disabled.   An event of 0 means a pause(), while an event equal
to the process's own ptab address is a wait().   */

void psleep(void *event)
{
    register dummy;  /* Force saving of registers */

    di();
    if (udata.u_ptab->p_status != P_RUNNING)
	panic("psleep: voodoo");
    if (!event)
	udata.u_ptab->p_status = P_PAUSE;
    else if (event == (char *)udata.u_ptab)
	udata.u_ptab->p_status = P_WAIT;
    else
	udata.u_ptab->p_status = P_SLEEP;

    udata.u_ptab->p_wait = event;

    ei();

    swapout();          /* Swap us out, and start another process */

    /* Swapout doesn't return until we have been swapped back in */
}

/* wakeup() looks for any process waiting on the event,
and make them runnable */

void wakeup(char *event)
{
    register ptptr p;

    di();
    for(p=ptab;p < ptab+PTABSIZE; ++p)
    {
	if (p->p_status > P_RUNNING && p->p_wait == event)
	{
	    p->p_status = P_READY;
	    p->p_wait = (char *)NULL;
	}
    }
    ei();
}


/* Temp storage for swapout() */
char *stkptr;

/* Swapout swaps out the current process, finds another that is READY,
possibly the same process, and swaps it in.
When a process is restarted after calling swapout,
it thinks it has just returned from swapout(). */

/* This function can have no arguments or auto variables */
int swapout(void)
{
    static ptptr newp;
    ptptr getproc();


    /* See if any signals are pending */
    chksigs();

    /* Get a new process */
    newp = getproc();

    /* If there is nothing else to run, just return */
    if (newp == udata.u_ptab)
    {
	udata.u_ptab->p_status = P_RUNNING;
	return (runticks = 0);
    }

    ;
    /* Save the stack pointer and critical registers */
/*
#asm
	LD      HL,01   ;this will return 1 if swapped.
	PUSH    HL      ;will be return value
	PUSH    BC
	PUSH    IX
	LD      HL,0
	ADD     HL,SP   ;get sp into hl
	LD      (stkptr?),HL
#endasm
*/
    udata.u_sp = stkptr;

    swrite();
    /* Read the new process in, and return into its context. */
    swapin(newp);

    /* We should never get here. */
    panic("swapin failed");
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

/* This sees if the current process has any signals set, and deals with them */
void chksigs(void)
{
    register j;

    di();
    ifnot (udata.u_ptab->p_pending)
    {
	ei();
	return;
    }

    for (j=1; j < NSIGS; ++j)
    {
	ifnot (sigmask(j) & udata.u_ptab->p_pending)
	    continue;
	if (udata.u_sigvec[j] == SIG_DFL)
	{
	    ei();
	    doexit(0,j);
	}

	if (udata.u_sigvec[j] != SIG_IGN)
	{
	    /* Arrange to call the user routine at return */
	    udata.u_ptab->p_pending &= !sigmask(j);
	    udata.u_cursig = j;
	}
    }
    ei();
}

void sendsig(ptptr proc, int16 sig)
{
    register ptptr p;

    if (proc)
	ssig(proc,sig);
    else
	for (p=ptab; p < ptab+PTABSIZE; ++p)
	    if (p->p_status)
	        ssig(p,sig);
}

void ssig(register ptptr proc, int16 sig)
{
    register stat;

    di();
    ifnot(proc->p_status)
	goto done;              /* Presumably was killed just now */

    if (proc->p_ignored & sigmask(sig))
	goto done;

    stat = proc->p_status;
    if (stat == P_PAUSE || stat == P_WAIT || stat == P_SLEEP)
	proc->p_status = P_READY;

    proc->p_wait = (char *)NULL;
    proc->p_pending |= sigmask(sig);
done:
    ei();
}
