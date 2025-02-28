#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <unix.h>

int super(void);

int uf_alloc(void);
int oft_alloc(void);
void oft_deref(register int of);

int isdevice(inoptr ino);
void d_close(int dev);
inoptr i_open(int dev, unsigned ino);
oid i_ref(inoptr ino);
void f_trunc(register inoptr ino);
void wr_inode(register inoptr ino);
int ch_link(register inoptr wd, char *oldname, char *newname, inoptr nindex);
int fmount(register int dev, register inoptr ino);
blkno_t bmap(inoptr ip, blkno_t bn, int rwflg);

void setftime(register inoptr ino, register int flag);

/* static to filesys.c */
void validblk(int dev, blkno_t num);
void freeblk(int dev, blkno_t blk, int level);

#endif /* __FILESYS_H__ */
