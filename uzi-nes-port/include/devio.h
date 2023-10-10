#ifndef __DEVIO_H__
#define __DEVIO_H__

#include "unix.h"

void bufinit(void);
int validdev(int dev);

int ok(void);
int nogood(void);

#endif /* __DEVIO_H__ */
