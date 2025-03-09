#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <unix.h>

int super(void);

int uf_alloc(void);
int oft_alloc(void);
void oft_deref(register int of);

int isdevice(inoptr ino);
int ch_link(register inoptr wd, char *oldname, char *newname, inoptr nindex);
int fmount(register int dev, register inoptr ino);
blkno_t bmap(inoptr ip, blkno_t bn, int rwflg);

#endif /* __FILESYS_H__ */
