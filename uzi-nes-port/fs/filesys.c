#include <filesys.h>
#include <ppu.h>
#include <unix.h>

/* I_ref increases the reference count of the given inode table entry. */

void i_ref(inoptr ino)
{
    if (++(ino->c_refs) == 2*ITABSIZE)  /* Arbitrary limit. */
	panic("too many i-refs");
}
