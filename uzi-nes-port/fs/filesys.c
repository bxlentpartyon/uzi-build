#include <filesys.h>
#include <lib/string.h>
#include <ppu.h>
#include <process.h>
#include <unix.h>

void magic(inoptr ino);

/* I_ref increases the reference count of the given inode table entry. */

void i_ref(inoptr ino)
{
    if (++(ino->c_refs) == 2*ITABSIZE)  /* Arbitrary limit. */
	panic("too many i-refs");
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

void magic(inoptr ino)
{
    if (ino->c_magic != CMAGIC)
	panic("Corrupt inode");
}
