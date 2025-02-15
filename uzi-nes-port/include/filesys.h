#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <unix.h>

void i_ref(inoptr ino);
void i_deref(inoptr ino);
int fmount(int dev, inoptr ino);

#define BLOCK_SIZE	512

#endif /* __FILESYS_H__ */
