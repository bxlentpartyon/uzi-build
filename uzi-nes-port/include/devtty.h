#ifndef __DEVTTY_H__
#define __DEVTTY_H__

int tty_open(int minor);
int tty_close(int minor);
void tty_init(void);

#endif /* __DEVTTY_H__ */
