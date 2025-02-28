#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <trampoline.h>
#include <unix.h>

#pragma wrapped-call (push, trampoline, FS_PAGE)
int getperm(inoptr ino);
int getmode(inoptr ino);
void i_ref(inoptr ino);
void i_deref(inoptr ino);
int fmount(int dev, inoptr ino);
#pragma wrapped-call (pop)

/* static to filesys.c */
void magic(inoptr ino);

#define BLOCK_SIZE	512

#endif /* __FILESYS_H__ */
