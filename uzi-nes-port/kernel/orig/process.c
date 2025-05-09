
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

