int ok(), nogood();
#define DEVIO

#include <string.h>

void bufdump(void)
{
    register bufptr j;

    kprintf("\ndev\tblock\tdirty\tbusy\ttime clock %d\n", bufclock);
    for (j=bufpool; j < bufpool+NBUFS; ++j)
        kprintf("%d\t%u\t%d\t%d\t%u\n",
            j->bf_dev,j->bf_blk,j->bf_dirty,j->bf_busy,j->bf_time);
}

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

int d_ioctl(int dev, int request, char *data)
{
    ifnot (validdev(dev))
    {
        udata.u_error = ENXIO;
        return(-1);
    }
    if((*dev_tab[dev].dev_ioctl)(dev_tab[dev].minor,request,data))
    {
        udata.u_error = EINVAL;
        return(-1);
    }
        return(0);
}

/*************************************************************
Character queue management routines
************************************************************/

/* Remove something from the tail; the most recently added char. */
uninsq(struct s_queue *q, char *cp)
{
    di();
    ifnot (q->q_count)
    {
        ei();
        return(0);
    }
    --q->q_count;
    if (--q->q_tail <= q->q_base)
        q->q_tail = q->q_base + q->q_size - 1;
    *cp = *(q->q_tail);
    ei();
    return(1);
}


/* Returns true if the queue has more characters than its wakeup number */
fullq(struct s_queue *q)
{
    di();
    if (q->q_count > q->q_wakeup)
    {
        ei();
        return (1);
    }
    ei();
    return (0);
}
