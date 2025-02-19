#ifndef __FS_H__
#define __FS_H__

#include <stddef.h>

#include <unix.h>

#define BLOCKSIZE	512

/*
 * We need our temporary superblock buffers to be slightly larger than an
 * actual block.  This is because the dwrite that writes out the freelist
 * blocks uses &filsys->s_nfree as the beginning of the freelist block, which
 * is 6 bytes into the actual filesys struct.  If we use the normal blocksize
 * for that type of write, we can potentially write 6 bytes of garbage after
 * each freelist block.
 */
#define DUMMY_BLOCKSIZE	BLOCKSIZE + offsetof(filesys, s_nfree)

int yes(void);

#endif /* __FS_H__ */
