/**************************************************
UZI (Unix Z80 Implementation) Kernel:  devmisc.c
***************************************************/

#include <unix.h>
#include <extern.h>

#include <devmisc.h>
#include <extras.h>
#include <machdep.h>

unsigned int mem_read(int minor, int rawflag)
{
    bcopy((char *)(512*udata.u_offset.o_blkno+udata.u_offset.o_offset),
                 udata.u_base, udata.u_count);
    return(udata.u_count);
}

unsigned int mem_write(int minor, int rawflag)
{
    bcopy(udata.u_base,
        (char *)(512*udata.u_offset.o_blkno+udata.u_offset.o_offset),
                udata.u_count);
    return(udata.u_count);
}



unsigned int null_write(int minor, int rawflag)
{
    return(udata.u_count);
}



static char lop = 0;

int lpr_open(void)
{
    lop = 1;
    return(0);
}

int lpr_close(void)
{
    if (lop)
    {
        lop  = 0;
        lpout('\f');
        lpout('\f');
    }
    return(0);
}



unsigned int lpr_write(int minor, int rawflag)
{
    unsigned n;

    n = udata.u_count;
    while (n--)
        lpout(*udata.u_base++);
    return(udata.u_count);
}


void lpout(char c)
{
    while(in(0x84)&02)
        ;

    out(c,0x85);
    out(0xfe,0x86);
    out(0xff,0x86);
    out(0xff,0x85);
}


//#include "devmt.c"

