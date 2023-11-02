#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <unix.h>

int d_open(int dev);
void i_ref(inoptr ino);
void i_deref(inoptr ino);

#endif /* __FILESYS_H__ */
