#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <ppu.h>
#include <unix.h>

#include <lib/string.h>

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
