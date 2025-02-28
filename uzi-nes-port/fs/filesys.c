#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <ppu.h>
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
