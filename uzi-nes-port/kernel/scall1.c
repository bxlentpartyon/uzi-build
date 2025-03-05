#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <process.h>
#include <scall.h>
#include <unix.h>

#include <lib/string.h>

#pragma code-name (push, "SCALL1_CODE")

int doclose(int16 uindex)
{
	return 0;
}

void readi(register inoptr ino)
{
    register uint16 amount;
    register uint16 toread;
    register blkno_t pblk;
    register char *bp;
    int dev;
    int ispipe;
    char *bread();
    char *zerobuf();
    blkno_t bmap();

    dev = ino->c_dev;
    ispipe = 0;
    switch (getmode(ino))
    {

    case F_DIR:
    case F_REG:

	/* See of end of file will limit read */
	toread = udata.u_count =
	    ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno >= 64 ?
	        udata.u_count :
	        min(udata.u_count,
	         512*(ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno) +
	         (ino->c_node.i_size.o_offset-udata.u_offset.o_offset));
	goto loop;

    case F_PIPE:
	ispipe = 1;
	while (psize(ino) == 0)
	{
	    if (ino->c_refs == 1) /* No writers */
	        break;
	    /* Sleep if empty pipe */
	    psleep(ino);
	}
	toread = udata.u_count = min(udata.u_count, psize(ino));
	goto loop;

    case F_BDEV:
	toread = udata.u_count;
	dev = *(ino->c_node.i_addr);

    loop:
	while (toread)
	{
	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 1)) != NULLBLK)
	        bp = bread(dev, pblk, 0);
	    else
	        bp = zerobuf();

	    bcopy(bp+udata.u_offset.o_offset, udata.u_base,
	            (amount = min(toread, 512 - udata.u_offset.o_offset)));
	    brelse(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if (ispipe && udata.u_offset.o_blkno >= 18)
	        udata.u_offset.o_blkno = 0;
	    toread -= amount;
	    if (ispipe)
	    {
	        addoff(&(ino->c_node.i_size), -amount);
	        wakeup(ino);
	    }
	}

	break;

    case F_CDEV:
	udata.u_count = cdread(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
	break;

    default:
	udata.u_error = ENODEV;
    }
}

/* Writei (and readi) need more i/o error handling */
void writei(register inoptr ino)
{
    register uint16 amount;
    register uint16 towrite;
    register char *bp;
    int ispipe;
    blkno_t pblk;
    int created;        /* Set by bmap if newly allocated block used */
    int dev;
    char *zerobuf();
    char *bread();
    blkno_t bmap();

    dev = ino->c_dev;

    switch (getmode(ino))
    {

    case F_BDEV:
	dev = *(ino->c_node.i_addr);
    case F_DIR:
    case F_REG:
	ispipe = 0;
	towrite = udata.u_count;
	goto loop;

    case F_PIPE:
	ispipe = 1;
	while ((towrite = udata.u_count) > (16*512) - psize(ino))
	{
	    if (ino->c_refs == 1) /* No readers */
	    {
	        udata.u_count = -1;
	        udata.u_error = EPIPE;
	        ssig(udata.u_ptab, SIGPIPE);
	        return;
	    }
	    /* Sleep if empty pipe */
	    psleep(ino);
	}

	/* Sleep if empty pipe */
	goto loop;

    loop:

	while (towrite)
	{
	    amount = min(towrite, 512 - udata.u_offset.o_offset);


	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 0)) == NULLBLK)
	        break;    /* No space to make more blocks */

	    /* If we are writing an entire block, we don't care
	    about its previous contents */
	    bp = bread(dev, pblk, (amount == 512));

	    bcopy(udata.u_base, bp+udata.u_offset.o_offset, amount);
	    bawrite(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if(ispipe)
	    {
	        if (udata.u_offset.o_blkno >= 18)
	            udata.u_offset.o_blkno = 0;
	        addoff(&(ino->c_node.i_size), amount);
	        /* Wake up any readers */
	        wakeup(ino);
	    }
	    towrite -= amount;
	}

	/* Update size if file grew */
	ifnot (ispipe)
	{
	    if ( udata.u_offset.o_blkno > ino->c_node.i_size.o_blkno ||
	        (udata.u_offset.o_blkno == ino->c_node.i_size.o_blkno &&
	            udata.u_offset.o_offset > ino->c_node.i_size.o_offset))
	    {
	        ino->c_node.i_size.o_blkno = udata.u_offset.o_blkno;
	        ino->c_node.i_size.o_offset = udata.u_offset.o_offset;
	        ino->c_dirty = 1;
	    }
	}

	break;

    case F_CDEV:
	udata.u_count = cdwrite(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
	break;

    default:
	udata.u_error = ENODEV;
    }

}

int min(int a, int b)
{
    return ( a < b ? a : b);
}

int psize(inoptr ino)
{
    return (512*ino->c_node.i_size.o_blkno+ino->c_node.i_size.o_offset);
}

void addoff(off_t *ofptr, int amount)
{
    if (amount >= 0)
    {
    ofptr->o_offset += amount % 512;
    if (ofptr->o_offset >= 512)
    {
	ofptr->o_offset -= 512;
	++ofptr->o_blkno;
    }
    ofptr->o_blkno += amount/512;
    }
    else
    {
	ofptr->o_offset -= (-amount) % 512;
	if (ofptr->o_offset < 0)
	{
	    ofptr->o_offset += 512;
	    --ofptr->o_blkno;
	}
	ofptr->o_blkno -= (-amount)/512;
    }
}

/****************************************
sync()
***************************************/
void _sync(void)
{
    register j;
    register inoptr ino;
    register char *buf;
    char *bread();

    /* Write out modified inodes */

    for (ino=i_tab; ino < i_tab+ITABSIZE; ++ino)
	if ((ino->c_refs) > 0 && ino->c_dirty != 0)
	{
	    wr_inode(ino);
	    ino->c_dirty = 0;
	}

    /* Write out modified super blocks */
    /* This fills the rest of the super block with garbage. */

    for (j=0; j < NDEVS; ++j)
    {
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_fmod)
	{
	    fs_tab[j].s_fmod = 0;
	    buf = bread(j, 1, 1);
	    bcopy((char *)&fs_tab[j], buf, 512);
	    bfree(buf, 2);
	}
    }

    bufsync();   /* Clear buffer pool */
}

#pragma code-name (pop)
