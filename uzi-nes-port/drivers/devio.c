/**************************************************
UZI (Unix Z80 Implementation) Kernel:  devio.c
***************************************************/

#include <unix.h>
#include <extern.h>

void bufinit(void)
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
        bp->bf_dev = -1;
    }
}

