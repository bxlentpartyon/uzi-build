/**************************************************
UZI (Unix Z80 Implementation) Kernel:  devio.c
***************************************************/

#include <unix.h>
#include <extern.h>

void bufinit(void)
{
	register bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		bp->bf_dev = -1;
	}
}

int d_open(int dev)
{
    ifnot (validdev(dev))
        return(-1);
    return ((*dev_tab[dev].dev_open)(dev_tab[dev].minor));
}

int validdev(int dev)
{
    return(dev >= 0 && dev < (sizeof(dev_tab)/sizeof(struct devsw)));
}
