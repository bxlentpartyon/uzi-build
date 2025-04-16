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
