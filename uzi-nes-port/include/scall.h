#ifndef __SCALL_H__
#define __SCALL_H__

#include <unix.h>
#include <trampoline.h>

#pragma wrapped-call(push, trampoline, SCALL1_PAGE)
int doclose(int16 uindex);
void _sync(void);
#pragma wrapped-call(pop)

#pragma wrapped-call(push, trampoline, SCALL2_PAGE)
void doexit(int16 val, int16 val2);
#pragma wrapped-call(pop)

#endif /* __SCALL_H__ */
