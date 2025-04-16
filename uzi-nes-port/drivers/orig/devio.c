int ok(), nogood();
#define DEVIO

#include <string.h>

int swapread(int dev, blkno_t blkno, unsigned nbytes, char *buf)
{
    swapbase = buf;
    swapcnt = nbytes;
    swapblk = blkno;
    return ((*dev_tab[dev].dev_read)(dev_tab[dev].minor, 2));
}


int swapwrite(int dev, blkno_t blkno, unsigned nbytes, char *buf)
{
    swapbase = buf;
    swapcnt = nbytes;
    swapblk = blkno;
    return ((*dev_tab[dev].dev_write)(dev_tab[dev].minor, 2));
}
