#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <trampoline.h>
#include <unix.h>

#pragma wrapped-call (push, trampoline, FS_PAGE)
int getperm(inoptr ino);
int getmode(inoptr ino);
inoptr i_open(int dev, unsigned ino);
void i_ref(inoptr ino);
void i_deref(inoptr ino);
void f_trunc(register inoptr ino);
void wr_inode(register inoptr ino);
int fmount(int dev, inoptr ino);

void setftime(register inoptr ino, register int flag);
#pragma wrapped-call (pop)

#define BLOCK_SIZE	512

#endif /* __FILESYS_H__ */
