/**************************************************
UZI (Unix Z80 Implementation) Kernel:  devmisc.c
***************************************************/

#include <extern.h>
#include <lib/string.h>
#include <machdep.h>
#include <unix.h>

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
