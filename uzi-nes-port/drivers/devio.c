/**************************************************
UZI (Unix Z80 Implementation) Kernel:  devio.c
***************************************************/

#include <devio.h>
#include <machdep.h>
#include <ppu.h>
#include <unix.h>
#include <extern.h>

void bufinit(void)
{
	register bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		bp->bf_dev = -1;
	}
}

int cdread(int dev)
{
	ifnot (validdev(dev))
		panic("cdread: invalid dev");
	return ((*dev_tab[dev].dev_read)(dev_tab[dev].minor, 1));
}

int d_open(int dev)
{
	ifnot (validdev(dev))
		return(-1);
	return ((*dev_tab[dev].dev_open)(dev_tab[dev].minor));
}

int ok(void)
{
	return(0);
}

int nogood(void)
{
	return(-1);
}

int validdev(int dev)
{
	return(dev >= 0 && dev < (sizeof(dev_tab)/sizeof(struct devsw)));
}

/*************************************************************
Character queue management routines
************************************************************/

/* add something to the tail */
int insq(register struct s_queue *q, char c)
{
    di();
    if (q->q_count == q->q_size)
    {
        ei();
        return(0);
    }
    *(q->q_tail) = c;
    ++q->q_count;
    if (++q->q_tail >= q->q_base + q->q_size)
        q->q_tail = q->q_base;
    ei();
    return(1);
}

/* Remove something from the head. */
int remq(struct s_queue *q, char *cp)
{
    di();
    ifnot (q->q_count)
    {
        ei();
        return(0);
    }
    *cp = *(q->q_head);
    --q->q_count;
    if (++q->q_head >= q->q_base + q->q_size)
        q->q_head = q->q_base;
    ei();
    return(1);
}
