#ifndef __DEVIO_H__
#define __DEVIO_H__

#include "unix.h"

int remq(struct s_queue *q, char *cp);
int insq(register struct s_queue *q, char c);
char *bread(int dev, blkno_t blk, int rewrite);
void brelse(bufptr bp);
int cdread(int dev);
void bufinit(void);
int validdev(int dev);

int ok(void);
int nogood(void);

#endif /* __DEVIO_H__ */
