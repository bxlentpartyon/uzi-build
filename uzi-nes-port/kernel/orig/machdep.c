#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <unix.h>
#include <extern.h>

#include <devio.h>
#include <devtty.h>
#include <machdep.h>
#include <process.h>

/* This is called at the very beginning to initialize everything. */
/* It is the equivalent of main() */

void main()
{
    di();
    inint = 0;
    udata.u_insys = 1;
    ei();

    init2();   /* in process.c */
}


extern int unix();


static int cursig;
static int (*curvec)();

/* This interrupt device routine calls the service routine of each device
that could have interrupted. */

service()
{
/*
#asm 8080
        PUSH    PSW
        PUSH    B
        PUSH    D
        PUSH    H
.Z80
        PUSH    IX
        PUSH    IY
.8080
#endasm
*/

    inint = 1;

    if (tty_int())
        goto found;
    if (clk_int())
        goto found;
/*  if (  ) ...   */

    warning("Spurious interrupt");

found:
    inint = 0;

    /* Deal with a pending caught signal, if any */
    if (!udata.u_insys)
        calltrap();
    ;

/*
#asm 8080
.Z80
        POP     IY
        POP     IX
.8080
        POP     H
        POP     D
        POP     B
        POP     PSW
        EI
        RET
#endasm
*/
}



void calltrap(void)
{
    /* Deal with a pending caught signal, if any. */
        /* udata.u_insys should be false, and interrupts enabled.
        remember, the user may never return from the trap routine */

    if (udata.u_cursig)
    {
        cursig = udata.u_cursig;
        curvec = udata.u_sigvec[cursig];
        udata.u_cursig = 0;
        udata.u_sigvec[cursig] = SIG_DFL;   /* Reset to default */
        ei();
        (*curvec)(cursig);
        di();
    } 
}

sttime()
{
    panic("Calling sttime");
}


void idump(void)
{
    inoptr ip;
    ptptr pp;
    extern struct cinode i_tab[];

    kprintf(
        "\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY err %d root %d\n",
            udata.u_error, root - i_tab);

    for (ip=i_tab; ip < i_tab+ITABSIZE; ++ip)
    {
        kprintf("%d\t%d\t%d\t%u\t0%o\t0%o\t%d\t%d\t%d\t%d\n",
               ip-i_tab, ip->c_magic, ip->c_dev, ip->c_num,
               ip->c_node.i_mode_lo, ip->c_node.i_mode_hi,
	       ip->c_node.i_nlink, ip->c_node.i_addr[0],
               ip->c_refs,ip->c_dirty);
/*****
        ifnot (ip->c_magic)     
            break;
******/
    }

    kprintf("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\n");
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
        kprintf("%d\t%d\t0x%x\t%d\t%d\t%d\t0x%x\t0x%x\n",
               pp-ptab, pp->p_status, pp->p_wait,  pp->p_pid,
               pp->p_pptr-ptab, pp->p_alarm, pp->p_pending,
                pp->p_ignored);
        ifnot(pp->p_pptr)
            break;
    }   
    
    bufdump();

    kprintf("\ninsys %d ptab %d call %d cwd %d sp 0x%x\n",
        udata.u_insys,udata.u_ptab-ptab, udata.u_callno, udata.u_cwd-i_tab,
       udata.u_sp);
}
