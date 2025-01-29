#ifndef __DEVPPUWD_H__
#define __DEVPPUWD_H__

int ppuwd_open(int minor);
int ppuwd_read(int minor, int rawflag);

#endif /* __DEVPPUWD_H__ */
