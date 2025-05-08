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

sttime()
{
    panic("Calling sttime");
}
