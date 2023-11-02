#include <filesys.h>
#include <ppu.h>
#include <unix.h>

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
