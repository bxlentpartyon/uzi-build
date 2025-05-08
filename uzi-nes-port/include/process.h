#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "trampoline.h"

#pragma wrapped-call (push, trampoline, PROCESS_PAGE)
void psleep(void *event);
void sendsig(ptptr proc, int16 sig);
void wakeup(char *event);
void ssig(register ptptr proc, int16 sig);
void chksigs(void);
void swapin(ptptr pp);
int dofork(void);
void init2(void);
ptptr getproc(void);
#pragma wrapped-call (pop)

#endif				/* __PROCESS_H__ */
