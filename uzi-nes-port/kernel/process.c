#include <unix.h>
#include <extern.h>
#include <devio.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <string.h>

extern void ei(void);
extern void di(void);

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

void init2(void)

{
	bufinit();

	initproc = ptab_alloc();
	udata.u_ptab = initproc;

	kprintf("boot: %s", "garbage");

	while(1);
}
