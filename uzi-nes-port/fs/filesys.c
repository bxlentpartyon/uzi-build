#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <machdep.h>
#include <ppu.h>
#include <time.h>
#include <unix.h>

#include <lib/string.h>

#pragma code-name (push, "FS_CODE")

/* N_open is given a string containing a path name,
  and returns a inode table pointer.  If it returns NULL,
  the file did not exist.  If the parent existed,
  and parent is not null, parent will be filled in with
  the parents inoptr. Otherwise, parent will be set to NULL. */
inoptr n_open(char *name, inoptr *parent)
{

    register inoptr wd;  /* the directory we are currently searching. */
    register inoptr ninode;
    register inoptr temp;
    inoptr srch_dir();
    inoptr srch_mt();

    if (*name == '/')
	wd = root;
    else
	wd = udata.u_cwd;

    i_ref(ninode = wd);
    i_ref(ninode);

    for(;;)
    {
	if (ninode)
	    magic(ninode);

	/* See if we are at a mount point */
	if (ninode)
	    ninode = srch_mt(ninode);

	while (*name == '/')    /* Skip (possibly repeated) slashes */
	    ++name;
	ifnot (*name)           /* No more components of path? */
	    break;
	ifnot (ninode)
	{
	    udata.u_error = ENOENT;
	    goto nodir;
	}
	i_deref(wd);
	wd = ninode;
	if (getmode(wd) != F_DIR)
	{
	    udata.u_error = ENOTDIR;
	    goto nodir;
	}
	ifnot (getperm(wd) & OTH_EX)
	{
	    udata.u_error = EPERM;
	    goto nodir;
	}

	/* See if we are going up through a mount point */
	if ( wd->c_num == ROOTINODE && wd->c_dev != ROOTDEV && name[1] == '.')
	{
	   temp = fs_tab[wd->c_dev].s_mntpt;
	   ++temp->c_refs;
	   i_deref(wd);
	   wd = temp;
	}

	ninode = srch_dir(wd,name);

	while (*name != '/' && *name )
	    ++name;
    }

    if (parent)
	*parent = wd;
    else
	i_deref(wd);
    ifnot (parent || ninode)
	udata.u_error = ENOENT;
    return (ninode);

nodir:
    if (parent)
	*parent = NULLINODE;
    i_deref(wd);
    return(NULLINODE);

}

/* Srch_dir is given a inode pointer of an open directory
and a string containing a filename, and searches the directory
for the file.  If it exists, it opens it and returns the inode pointer,
otherwise NULL. This depends on the fact that ba_read will return unallocated
blocks as zero-filled, and a partially allocated block will be padded with
zeroes.  */
inoptr srch_dir(inoptr wd, char *compname)
{
    register int curentry;
    register blkno_t curblock;
    register struct direct *buf;
    register int nblocks;
    unsigned inum;
    inoptr i_open();
    blkno_t bmap();

    nblocks = wd->c_node.i_size.o_blkno;
    if (wd->c_node.i_size.o_offset)
	++nblocks;

    for (curblock=0; curblock < nblocks; ++curblock)
    {
	buf = (struct direct *)bread( wd->c_dev, bmap(wd, curblock, 1), 0);
	for (curentry = 0; curentry < 32; ++curentry)
	{
	    if (namecomp(compname,buf[curentry].d_name))
	    {
	        inum = buf[curentry&0x1f].d_ino;
	        brelse(buf);
	        return(i_open(wd->c_dev, inum));
	    }
	}
	brelse(buf);
    }
    return(NULLINODE);
}

/* Srch_mt sees if the given inode is a mount point. If
so it dereferences it, and references and returns a pointer
to the root of the mounted filesystem. */
inoptr srch_mt(inoptr ino)
{
    register int j;
    inoptr i_open();

    for (j=0; j < NDEVS; ++j)
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_mntpt == ino)
	{
	    i_deref(ino);
	    return(i_open(j,ROOTINODE));
	}

    return(ino);
}

/* Namecomp compares two strings to see if they are the same file name.
It stops at 14 chars or a null or a slash. It returns 0 for difference. */
int namecomp(register char *n1, register char *n2)
{
    register int n;

    n = 14;
    while (*n1 && *n1 != '/')
    {
	if (*n1++ != *n2++)
	    return(0);
	ifnot (--n)
	    return(-1);
    }
    return(*n2 == '\0' || *n2 == '/');
}

/* Check the given device number, and return its address in the mount table.
Also time-stamp the superblock of dev, and mark it modified.
Used when freeing and allocating blocks and inodes. */
fsptr getdev(int devno)
{
    register fsptr dev;

    dev = fs_tab + devno;
    if (devno < 0 || devno >= NDEVS || !dev->s_mounted)
	panic("getdev: bad dev");
    rdtime(&(dev->s_time));
    dev->s_fmod = 1;
    return (dev);
}

/* Returns true if the magic number of a superblock is corrupt */
baddev(fsptr dev)
{
    return (dev->s_mounted != SMOUNTED);
}

/* Blk_alloc is given a device number, and allocates an unused block
from it. A returned block number of zero means no more blocks. */
blkno_t blk_alloc(int devno)
{

    register fsptr dev;
    register blkno_t newno;
    blkno_t *buf;
    register int j;

    if (baddev(dev = getdev(devno)))
	goto corrupt2;

    if (dev->s_nfree <= 0 || dev->s_nfree > 50)
	goto corrupt;

    newno = dev->s_free[--dev->s_nfree];
    ifnot (newno)
    {
	if (dev->s_tfree != 0)
	    goto corrupt;
	udata.u_error = ENOSPC;
	++dev->s_nfree;
	return(0);
    }

    /* See if we must refill the s_free array */

    ifnot (dev->s_nfree)
    {
	buf = (blkno_t *)bread(devno,newno, 0);
	dev->s_nfree = buf[0];
	for (j=0; j < 50; j++)
	{
	    dev->s_free[j] = buf[j+1];
	}
	brelse((char *)buf);
    }

    validblk(devno, newno);

    ifnot (dev->s_tfree)
	goto corrupt;
    --dev->s_tfree;

    /* Zero out the new block */
    buf = bread(devno, newno, 2);
    bzero(buf, 512);
    bawrite(buf);
    return(newno);

corrupt:
    warning("blk_alloc: corrupt");
    dev->s_mounted = 1;
corrupt2:
    udata.u_error = ENOSPC;
    return(0);
}

/* I_ref increases the reference count of the given inode table entry. */
void i_ref(inoptr ino)
{
    if (++(ino->c_refs) == 2*ITABSIZE)  /* Arbitrary limit. */
	panic("too many i-refs");
}

void i_deref(register inoptr ino)
{
	return;
}

/* Changes: blk_alloc zeroes block it allocates */

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * The block is zeroed if created.
 */
blkno_t bmap(inoptr ip, blkno_t bn, int rwflg)
{
	register int i;
	register bufptr bp;
	register int j;
	register blkno_t nb;
	int sh;
	int dev;

	blkno_t blk_alloc();

	if (getmode(ip) == F_BDEV)
	    return (bn);

	dev = ip->c_dev;

	/*
	 * blocks 0..17 are direct blocks
	 */
	if(bn < 18) {
	        nb = ip->c_node.i_addr[bn];
	        if(nb == 0) {
	                if(rwflg || (nb = blk_alloc(dev))==0)
	                        return(NULLBLK);
	                ip->c_node.i_addr[bn] = nb;
	                ip->c_dirty = 1;
	        }
	        return(nb);
	}

	/*
	 * addresses 18 and 19
	 * have single and double indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	bn -= 18;
	sh = 0;
	j = 2;
	if (bn & 0xff00)   /* bn > 255  so double indirect */
	{
	    sh = 8;
	    bn -= 256;
	    j = 1;
	}

	/*
	 * fetch the address from the inode
	 * Create the first indirect block if needed.
	 */
	ifnot (nb = ip->c_node.i_addr[20-j])
	{
	        if(rwflg || !(nb = blk_alloc(dev)))
	                return(NULLBLK);
	        ip->c_node.i_addr[20-j] = nb;
	        ip->c_dirty = 1;
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=2; j++) {
	        bp = (bufptr)bread(dev, nb, 0);
	        /******
	        if(bp->bf_error) {
	                brelse(bp);
	                return((blkno_t)0);
	        }
	        ******/
	        i = (bn>>sh) & 0xff;
	        if (nb = ((blkno_t *)bp)[i])
	            brelse(bp);
	        else
	        {
	                if(rwflg || !(nb = blk_alloc(dev))) {
	                        brelse(bp);
	                        return(NULLBLK);
	                }
	                ((blkno_t *)bp)[i] = nb;
	                bawrite(bp);
	        }
	        sh -= 8;
	}

	return(nb);
}

/* Validblk panics if the given block number is not a valid data block
for the given device. */
void validblk(int dev, blkno_t num)
{
    register fsptr devptr;

    devptr = fs_tab + dev;

    if (devptr->s_mounted == 0)
	panic("validblk: not mounted");

    if (num < devptr->s_isize || num >= devptr->s_fsize)
	panic("validblk: invalid blk");
}

/* Super returns true if we are the superuser */
int super(void)
{
    return(udata.u_euid == 0);
}

/* Getperm looks at the given inode and the effective user/group ids, and
returns the effective permissions in the low-order 3 bits. */
int getperm(inoptr ino)
{
    int mode;

    if (super())
	return(07);

    mode = ino->c_node.i_mode;
    if (ino->c_node.i_uid == udata.u_euid)
	mode >>= 6;
    else if (ino->c_node.i_gid == udata.u_egid)
	mode >>= 3;

    return(mode & 07);
}

int getmode(inoptr ino)
{
    return ino->c_node.i_mode & F_MASK;
}

/* Fmount places the given device in the mount table with
mount point ino */
int fmount(register int dev, register inoptr ino)
{
	char *buf;
	register struct filesys *fp;

	if (d_open(dev) != 0)
		panic("fmount: Cant open filesystem");
	/* Dev 0 blk 1 */
	fp = fs_tab + dev;
	buf = bread(dev, 1, 0);
	bcopy(buf, (char *)fp, sizeof(struct filesys));
	brelse(buf);

	/* See if there really is a filesystem on the device */
	if (fp->s_mounted != SMOUNTED ||
	    fp->s_isize >= fp->s_fsize)
		return (-1);

	fp->s_mntpt = ino;
	if (ino)
		++ino->c_refs;

	return (0);
}

void magic(inoptr ino)
{
    if (ino->c_magic != CMAGIC)
	panic("Corrupt inode");
}

#pragma code-name (pop)
