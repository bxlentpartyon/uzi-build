/**************************************************
UZI (Unix Z80 Implementation) Kernel:  filesys.c
***************************************************/

#include <string.h>

#include <unix.h>
#include <extern.h>

#include <devio.h>
#include <extras.h>
#include <filesys.h>
#include <machdep.h>
#include <process.h>
#include <scall.h>


char *bread();

/* Ch_link modifies or makes a new entry in the directory for the name
and inode pointer given. The directory is searched for oldname.
When found, it is changed to newname, and it inode # is that of
*nindex.  A oldname of "" matches a unused slot, and a nindex
of NULLINODE means an inode # of 0.  A return status of 0 means there
was no space left in the filesystem, or a non-empty oldname was not found,
or the user did not have write permission. */

int ch_link(register inoptr wd, char *oldname, char *newname, inoptr nindex)
{
    struct direct curentry;

    ifnot (getperm(wd) & OTH_WR)
    {
	udata.u_error = EPERM;
	return (0);
    }

    /* Search the directory for the desired slot. */

    udata.u_offset.o_blkno = 0;
    udata.u_offset.o_offset = 0;

    for (;;)
    {
	udata.u_count = 16;
	udata.u_base = (char *)&curentry;
	readi(wd);

	/* Read until EOF or name is found */
	/* readi() advances udata.u_offset */
	if (udata.u_count == 0 || namecomp(oldname, curentry.d_name))
	    break;
    }

    if (udata.u_count == 0 && *oldname)
	return (0);   /* Entry not found */

    bcopy(newname, curentry.d_name, 14);
    if (nindex)
	curentry.d_ino = nindex->c_num;
    else
	curentry.d_ino = 0;

    /* If an existing slot is being used, we must back up the file offset */
    if (udata.u_count)
    {
	ifnot (udata.u_offset.o_offset)
	{
	    --udata.u_offset.o_blkno;
	    udata.u_offset.o_offset = 512;
	}
	udata.u_offset.o_offset -= 16;
    }

    udata.u_count = 16;
    udata.u_base = (char *)&curentry;
    writei(wd);

    if (udata.u_error)
	return (0);

    setftime(wd, A_TIME|M_TIME|C_TIME);  /* Sets c_dirty */

    /* Update file length to next block */
    if (wd->c_node.i_size.o_offset)
    {
	wd->c_node.i_size.o_offset = 0;
	++wd->c_node.i_size.o_blkno;
    }

    return (1);
}



/* Filename is given a path name, and returns a pointer
to the final component of it. */

char *filename(char *path)
{
    register char *ptr;

    ptr = path;
    while (*ptr)
	++ptr;
    while (*ptr != '/' && ptr-- > path)
	;
    return (ptr+1);
}

/* Newfile is given a pointer to a directory and a name, and
   creates an entry in the directory for the name, dereferences
   the parent, and returns a pointer to the new inode.
   It allocates an inode number,
   and creates a new entry in the inode table for the new file,
   and initializes the inode table entry for the new file.  The new file
   will have one reference, and 0 links to it.
   Better make sure there isn't already an entry with the same name. */

inoptr newfile(inoptr pino, char *name)
{

    register inoptr nindex;
    register int j;
    inoptr i_open();

    ifnot (nindex = i_open(pino->c_dev, 0))
	goto nogood;

    nindex->c_node.i_mode_lo = 0;   /* For the time being */
    nindex->c_node.i_mode_hi = F_REG;   /* For the time being */
    nindex->c_node.i_nlink = 1;
    nindex->c_node.i_size.o_offset = 0;
    nindex->c_node.i_size.o_blkno = 0;
    for (j=0; j <20; j++)
	nindex->c_node.i_addr[j] = 0;
    wr_inode(nindex);

    ifnot (ch_link(pino,"",filename(name),nindex))
    {
	i_deref(nindex);
	goto nogood;
    }

    i_deref (pino);
    return(nindex);

nogood:
    i_deref (pino);
    return (NULLINODE);
}

/* I_free is given a device and inode number,
and frees the inode.  It is assumed that there
are no references to the inode in the inode table or
in the filesystem. */

i_free(int devno, unsigned ino)
{
    register fsptr dev;

    if (baddev(dev = getdev(devno)))
	return;

    if (ino < 2 || ino >= (dev->s_isize-2)*8)
	panic("i_free: bad ino");

    ++dev->s_tinode;
    if (dev->s_ninode < 50)
	dev->s_inode[dev->s_ninode++] = ino;
}


/* Blk_free is given a device number and a block number,
and frees the block. */

blk_free(int devno, blkno_t blk)
{
    register fsptr dev;
    register char *buf;

    ifnot (blk)
	return;

    if (baddev(dev = getdev(devno)))
	return;

    validblk(devno, blk);

    if (dev->s_nfree == 50)
    {
	buf = bread(devno, blk, 1);
	bcopy((char *)&(dev->s_nfree), buf, 512);
	bawrite(buf);
	dev->s_nfree = 0;
    }

    ++dev->s_tfree;
    dev->s_free[(dev->s_nfree)++] = blk;

}




/* Oft_alloc and oft_deref allocate and dereference (and possibly free)
entries in the open file table. */

int oft_alloc(void)
{
    register int j;

    for (j=0; j < OFTSIZE ; ++j)
    {
	ifnot (of_tab[j].o_refs)
	{
	    of_tab[j].o_refs = 1;
	    of_tab[j].o_inode = NULLINODE;
	    return (j);
	}
    }
    udata.u_error = ENFILE;
    return(-1);
}

void oft_deref(register int of)
{
    register struct oft *ofptr;

    ofptr = of_tab + of;

    if (!(--ofptr->o_refs) && ofptr->o_inode)
    {
	i_deref(ofptr->o_inode);
	ofptr->o_inode = NULLINODE;
    }
}



/* Uf_alloc finds an unused slot in the user file table. */

int uf_alloc(void)
{
    register int j;

    for (j=0; j < UFTSIZE ; ++j)
    {
	if (udata.u_files[j] & 0x80)  /* Portable, unlike  == -1 */
	{
	    return (j);
	}
    }
    udata.u_error = ENFILE;
    return(-1);
}

/* I_deref decreases the reference count of an inode, and frees it
from the table if there are no more references to it.  If it also
has no links, the inode itself and its blocks (if not a device) is freed. */

void i_deref(register inoptr ino)
{
    magic(ino);

    ifnot (ino->c_refs)
	panic("inode freed.");

    if ((ino->c_node.i_mode_hi & F_MASK) == F_PIPE)
	wakeup((char *)ino);

    /* If the inode has no links and no refs, it must have
    its blocks freed. */

    ifnot (--ino->c_refs || ino->c_node.i_nlink)
	    f_trunc(ino);

    /* If the inode was modified, we must write it to disk. */
    if (!(ino->c_refs) && ino->c_dirty)
    {
	ifnot (ino->c_node.i_nlink)
	{
	    ino->c_node.i_mode_lo = 0;
	    ino->c_node.i_mode_hi = 0;
	    i_free(ino->c_dev, ino->c_num);
	}
	wr_inode(ino);
    }
}


/* Wr_inode writes out the given inode in the inode table out to disk,
and resets its dirty bit. */

void wr_inode(register inoptr ino)
{
    struct dinode *buf;
    register blkno_t blkno;

    magic(ino);

    blkno = (ino->c_num >> 3) + 2;
    buf = (struct dinode *)bread(ino->c_dev, blkno,0);
    bcopy((char *)(&ino->c_node),
	(char *)((char **)&buf[ino->c_num & 0x07]), 64);
    bfree(buf, 2);
    ino->c_dirty = 0;
}


/* isdevice(ino) returns true if ino points to a device */
int isdevice(inoptr ino)
{
    return (ino->c_node.i_mode_hi & F_CDEV);
}


/* This returns the device number of an inode representing a device */
devnum(inoptr ino)
{
    return (*(ino->c_node.i_addr));
}


/* F_trunc frees all the blocks associated with the file,
if it is a disk file. */

void f_trunc(register inoptr ino)
{
    int dev;
    int j;

    dev = ino->c_dev;

    /* First deallocate the double indirect blocks */
    freeblk(dev, ino->c_node.i_addr[19], 2);

    /* Also deallocate the indirect blocks */
    freeblk(dev, ino->c_node.i_addr[18], 1);

    /* Finally, free the direct blocks */
    for (j=17; j >= 0; --j)
	freeblk(dev, ino->c_node.i_addr[j], 0);

    bzero((char *)ino->c_node.i_addr, sizeof(ino->c_node.i_addr));

    ino->c_dirty = 1;
    ino->c_node.i_size.o_blkno = 0;
    ino->c_node.i_size.o_offset = 0;
}

/* Companion function to f_trunc(). */
void freeblk(int dev, blkno_t blk, int level)
{
    blkno_t *buf;
    int j;

    ifnot (blk)
	return;

    if (level)
    {
	buf = (blkno_t *)bread(dev, blk, 0);
	for (j=255; j >= 0; --j)
	    freeblk(dev, buf[j], level-1);
	brelse((char *)buf);
    }

    blk_free(dev,blk);
}

/* This returns the inode pointer associated with a user's
file descriptor, checking for valid data structures */

inoptr getinode(int uindex)
{
    register int oftindex;
    register inoptr inoindex;

    if (uindex < 0 || uindex >= UFTSIZE || udata.u_files[uindex] & 0x80 )
    {
	udata.u_error = EBADF;
	return (NULLINODE);
    }

    if ((oftindex = udata.u_files[uindex]) < 0 || oftindex >= OFTSIZE)
	panic("Getinode: bad desc table");

    if ((inoindex = of_tab[oftindex].o_inode) < i_tab ||
	        inoindex >= i_tab+ITABSIZE)
	panic("Getinode: bad OFT");

    magic(inoindex);

    return(inoindex);
}

/* This sets the times of the given inode, according to the flags */

void setftime(register inoptr ino, register int flag)
{
    ino->c_dirty = 1;

    if (flag & A_TIME)
	rdtime(&(ino->c_node.i_atime));
    if (flag & M_TIME)
	rdtime(&(ino->c_node.i_mtime));
    if (flag & C_TIME)
	rdtime(&(ino->c_node.i_ctime));
}
