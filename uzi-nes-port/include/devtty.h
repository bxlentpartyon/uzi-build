#ifndef __DEVTTY_H__
#define __DEVTTY_H__

#include <trampoline.h>

#pragma wrapped-call(push, trampoline, DEVTTY_PAGE)
int tty_open(int minor);
int tty_close(int minor);
void tty_init(void);
#pragma wrapped-call(pop)

#endif /* __DEVTTY_H__ */
