#include <unix.h>
#include <extern.h>
#include <devio.h>
#include <devtty.h>
#include <filesys.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <scall.h>
#include <time.h>
#include <lib/string.h>
#include <kb.h>

#pragma code-name (push, "PROCESS_CODE")

int swapout(void);

/* Newproc fixes up the tables for the child of a fork */
void newproc(ptptr p)
{
	register char *j;

	/* Note that ptab_alloc clears most of the entry */
	di();
	p->p_swap = (p - ptab) * 65 + 1;	/* Allow 65 blocks per process */
	p->p_status = P_RUNNING;

	p->p_pptr = udata.u_ptab;
	p->p_ignored = udata.u_ptab->p_ignored;
	p->p_uid = udata.u_ptab->p_uid;
	udata.u_ptab = p;
	bzero((char *)&udata.u_utime, 4 * sizeof(time_t));	/* Clear tick counters */
	ei();

	rdtime(&udata.u_time);
	i_ref(udata.u_cwd);
	udata.u_cursig = udata.u_error = 0;

	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j)
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
	static ptptr pp = ptab;	/* Pointer for round-robin scheduling */

	for (;;) {
		if (++pp >= ptab + PTABSIZE)
			pp = ptab;

		di();
		status = pp->p_status;
		ei();

		if (status == P_RUNNING)
			panic("getproc: extra running");
		if (status == P_READY)
			return (pp);
	}
}

void swrite(void);

/* Temp storage for dofork */
int16 newid;

/* dofork implements forking.  */
/* This function can have no arguments or auto variables */
int dofork(void)
{
    static ptptr p;
    ptptr ptab_alloc();

    ifnot (p = ptab_alloc())
    {
	udata.u_error = EAGAIN;
	return(-1);
    }
    di();
    udata.u_ptab->p_status = P_READY; /* Parent is READY */
    newid = p->p_pid;
    ei();

    /* Save the stack pointer and critical registers */
    /* When the process is swapped back in, it will be as if
    it returns with the value of the childs pid. */
    udata.u_sp = fork_prep_begin(newid);
    swrite();
    fork_prep_end();

    /* Make a new process table entry, etc. */
    newproc(p);

    di();
    runticks = 0;
    p->p_status = P_RUNNING;
    ei();
    return (0);  /* Return to child */
}

/* This allocates a new process table slot, and fills
in its p_pid field with a unique number.  */
ptptr ptab_alloc()
{
	register ptptr p;
	register ptptr pp;
	static int nextpid = 0;

	di();
	for (p = ptab; p < ptab + PTABSIZE; ++p) {
		if (p->p_status == P_EMPTY)
			goto found;
	}
	ei();
	return (NULL);

 found:

	/* See if next pid number is unique */
 nogood:
	if (nextpid++ > 32000)
		nextpid = 1;
	for (pp = ptab; pp < ptab + PTABSIZE; ++pp) {
		if (pp->p_status != P_EMPTY && pp->p_pid == nextpid)
			goto nogood;
	}

	bzero((char *)p, sizeof(struct p_tab));
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
	static char *arg[2] = { "init", NULL };

	bufinit();

	/* Create the context for the first process */
	initproc = ptab_alloc();
	udata.u_ptab = initproc;
	newproc(initproc);
	dump_proc(initproc);
	initproc->p_status = P_RUNNING;

	/* User's file table */
	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j)
		*j = -1;

	/* Turn on the clock */
	start_clock();
	ei();

	/* Wait until the clock has interrupted, to set tod */
	while (!tod.t_date) ;

	/* Open the console tty device */
	tty_init();
	if (d_open(TTYDEV) != 0)
		panic("no tty");

	kprintf("boot:\n");
	udata.u_base = &bootchar;
	udata.u_count = 1;
	cdread(TTYDEV);
	ROOTDEV = bootchar - '0';

	/* Mount the root device */
	if (fmount(ROOTDEV, NULLINODE)) {
		panic("no filesys");
	} else {
		kprintf("root fs mounted!\n");
	}

	ifnot(root = i_open(ROOTDEV, ROOTINODE))
	    panic("no root");

	i_ref(udata.u_cwd = root);
	rdtime(&udata.u_time);

	kprintf("root inode %x\n", root);

	dump_proc(initproc);

	_execve("/init", &arg[0], &arg[1]);

	panic("no /init %d", udata.u_error);
}

/* psleep() puts a process to sleep on the given event.
If another process is runnable, it swaps out the current one
and starts the new one.
Normally when psleep is called, the interrupts have already been
disabled.   An event of 0 means a pause(), while an event equal
to the process's own ptab address is a wait().   */

void psleep(void *event)
{
	register dummy;		/* Force saving of registers */

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

	swapout();		/* Swap us out, and start another process */

	/* Swapout doesn't return until we have been swapped back in */
}

/* wakeup() looks for any process waiting on the event,
and make them runnable */

void wakeup(char *event)
{
	register ptptr p;

	di();
	for (p = ptab; p < ptab + PTABSIZE; ++p) {
		if (p->p_status > P_RUNNING && p->p_wait == event) {
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
	if (newp == udata.u_ptab) {
		udata.u_ptab->p_status = P_RUNNING;
		return (runticks = 0);
	}

	/* Save the stack pointer and critical registers */
	udata.u_sp = swapout_prep();

	swrite();
	/* Read the new process in, and return into its context. */
	swapin(newp);

	/* We should never get here. */
	panic("swapin failed");
}

/* This actually writes out the image */
void swrite(void)
{
    blkno_t blk;
    blk = udata.u_ptab->p_swap;

    /* Start by writing out the user data. */

    /* The user data is written so that it is packed to the top of one block */
    swapwrite(SWAPDEV, blk, 512, ((char *)(&udata+1))-512 );

    /* The user address space is written in two i/o operations,
       one from 0x100 to the break, and then from the stack up. */
    /* Notice that this might also include part or all of the user data,
       but never anything above it. */

    swapwrite(SWAPDEV,
	        blk+1,
	        (((char *)(&udata+1))-PROGBASE) & ~511,
	        PROGBASE);

}

/* No automatics can be used past tempstack(); */
void swapin(ptptr pp)
{
    static blkno_t blk;
    static ptptr newp;

    di();
    newp = pp;
    blk = newp->p_swap;
    ei();

    tempstack();

    swapread(SWAPDEV, blk, 512, ((char *)(&udata+1))-512 );

    /* The user address space is read in two i/o operations,
       one from 0x100 to the break, and then from the stack up. */
    /* Notice that this might also include part or all of the user data,
       but never anything above it. */

    swapread(SWAPDEV,
	        blk+1,
	        (((char *)(&udata+1))-PROGBASE) & ~511,
	        PROGBASE);

    if (newp != udata.u_ptab)
	panic("mangled swapin");
    di();
    newp->p_status = P_RUNNING;
    runticks = 0;
    ei();

    /* Restore the registers */
    swapin_finish(udata.u_sp);
}

/* This sees if the current process has any signals set, and deals with them */
void chksigs(void)
{
	register j;

	di();
	ifnot(udata.u_ptab->p_pending) {
		ei();
		return;
	}

	for (j = 1; j < NSIGS; ++j) {
		ifnot(sigmask(j) & udata.u_ptab->p_pending)
		    continue;
		if (udata.u_sigvec[j] == SIG_DFL) {
			ei();
			doexit(0, j);
		}

		if (udata.u_sigvec[j] != SIG_IGN) {
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
		ssig(proc, sig);
	else
		for (p = ptab; p < ptab + PTABSIZE; ++p)
			if (p->p_status)
				ssig(p, sig);
}

void ssig(register ptptr proc, int16 sig)
{
	register stat;

	di();
	ifnot(proc->p_status)
	    goto done;		/* Presumably was killed just now */

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

#pragma code-name (pop)

/*
 * unix() needs to be outside of PROCESS_CODE, in regular CODE
 * No auto vars here, so carry flag will be preserved
 */
int unix(int16 argn3, int16 argn2, int16 argn1, int16 argn,
	 char* uret, int callno)
{
    udata.u_argn3 = argn3;
    udata.u_argn2 = argn2;
    udata.u_argn1 = argn1;
    udata.u_argn = argn;
    udata.u_retloc = uret;
    udata.u_callno = callno;

    udata.u_insys = 1;
    udata.u_error = 0;
    ei();

#ifdef DEBUG
    kprintf ("\t\t\t\t\tcall %d (%x, %x, %x)\n",callno,argn2,argn1,argn);
#endif

    /* Branch to correct routine */
    udata.u_retval = dispatch_call(udata.u_ptab->zpsave);

#ifdef DEBUG
    kprintf("\t\t\t\t\t\tcall %d ret %x err %d\n",
	udata.u_callno,udata.u_retval, udata.u_error);
#endif


    chksigs();
    di();
    if (runticks >= MAXTICKS)
    {
	udata.u_ptab->p_status = P_READY;
	swapout();
    }
    ei();

    udata.u_insys = 0;
    calltrap();         /* Call trap routine if necessary */

    /* If an error, return errno with carry set */

    if (udata.u_error)
    {
	;

/*
#asm
	LD      HL, (udata? + ?OERR)
	POP     BC              ;restore frame pointer
	PUSH    BC
	POP     IX
	SCF
	RET
#endasm
*/
	;
    }

    return(udata.u_retval);
}
