
/**************************************************
UZI (Unix Z80 Implementation) Kernel:  process.c
***************************************************/

#include <string.h>

#include <unix.h>
#include <extern.h>

#include <extras.h>
#include <filesys.h>
#include <machdep.h>
#include <scall.h>

void init2(void)
{
    static char *arg[2] = { "init", NULL };

    kprintf("boot: ");

    ifnot (root = i_open(ROOTDEV,ROOTINODE))
	panic("no root");

    i_ref(udata.u_cwd = root);
    rdtime(&udata.u_time);

    udata.u_argn2 = (int16)("/init");
    udata.u_argn1 = (int16)(&arg[0]);
    udata.u_argn = (int16)(&arg[1]);
    _execve();
/*
    _execve("/init",&arg[0],&arg[1]);
*/
    panic("no /init");

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

    stkptr = udata.u_sp;
/*
#asm
	LD      HL,(stkptr?)
	LD      SP,HL
	POP     IX
	POP     BC
	POP     HL
	LD      A,H
	OR      L
	RET             ;return into the context of the swapped-in process
#endasm
*/

}


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

/*
#asm
	LD      HL,(newid?)
	PUSH    HL
	PUSH    BC
	PUSH    IX
	LD      HL,0
	ADD     HL,SP   ;get sp into hl
	LD      (stkptr?),HL
#endasm
*/

    udata.u_sp = stkptr;
    swrite();

/*
#asm
	POP     HL              ;repair stack pointer
	POP     HL
	POP     HL
#endasm
*/

    /* Make a new process table entry, etc. */
    newproc(p);

    di();
    runticks = 0;
    p->p_status = P_RUNNING;
    ei();
    return (0);  /* Return to child */
}


extern int (*disp_tab[])();

static int j;

/* No auto vars here, so carry flag will be preserved */
unix(argn3, argn2, argn1, argn, uret, callno)
int16 argn3, argn2, argn1, argn;
char *uret;
int callno;
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

    udata.u_retval = (*disp_tab[udata.u_callno])();

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

