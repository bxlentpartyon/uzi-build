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
