#ifndef __DEVIO_H__
#define __DEVIO_H__

#include "unix.h"
#include "trampoline.h"

#pragma wrapped-call (push, trampoline, DEVIO_PAGE)
int remq(struct s_queue *q, char *cp);
int insq(register struct s_queue *q, char c);
char *bread(int dev, blkno_t blk, int rewrite);
void brelse(bufptr bp);
int cdread(int dev);
void bawrite(bufptr bp);
int cdwrite(int dev);
int bfree(register bufptr bp, int dirty);
void bufsync(void);
int d_ioctl(int dev, int request, char *data);
int validdev(int dev);
void bufinit(void);

int d_open(int dev);
void d_close(int dev);
#pragma wrapped-call(pop)

int ok(void);
int nogood(void);

#endif /* __DEVIO_H__ */
