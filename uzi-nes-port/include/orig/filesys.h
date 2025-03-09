#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <unix.h>

int fmount(register int dev, register inoptr ino);
blkno_t bmap(inoptr ip, blkno_t bn, int rwflg);

#endif /* __FILESYS_H__ */
