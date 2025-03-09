#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <trampoline.h>
#include <unix.h>

#pragma wrapped-call (push, trampoline, FS_PAGE)
int super(void);

int uf_alloc(void);
int oft_alloc(void);
void oft_deref(register int of);

int getperm(inoptr ino);
int getmode(inoptr ino);
int isdevice(inoptr ino);
inoptr i_open(int dev, unsigned ino);
void i_ref(inoptr ino);
void i_deref(inoptr ino);
void f_trunc(register inoptr ino);
void wr_inode(register inoptr ino);
int ch_link(register inoptr wd, char *oldname, char *newname, inoptr nindex);
int fmount(int dev, inoptr ino);

void setftime(register inoptr ino, register int flag);
#pragma wrapped-call (pop)

#define BLOCK_SIZE	512

#endif /* __FILESYS_H__ */
