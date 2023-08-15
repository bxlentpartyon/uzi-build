#include <unix.h>
#include <extern.h>
#include <devio.h>
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
	bufinit();

	initproc = ptab_alloc();
	udata.u_ptab = initproc;

	/* Create the context for the first process */
	newproc(initproc);
	initproc->p_status = P_RUNNING;

	/* Turn on the clock */
	start_clock();
	ei();

	kprintf("boot:\n");

	dump_proc(initproc);

	while(1);
}
