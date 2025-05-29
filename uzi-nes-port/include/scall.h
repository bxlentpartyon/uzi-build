#ifndef __SCALL_H__
#define __SCALL_H__

#include <unix.h>
#include <trampoline.h>

#pragma wrapped-call(push, trampoline, SCALL1_PAGE)
void readi(register inoptr ino);
void writei(register inoptr ino);
int doclose(int16 uindex);
int _sync(void);
#pragma wrapped-call(pop)

#pragma wrapped-call(push, trampoline, SCALL2_PAGE)
int _execve(char *name, char *argv[], char *envp[]);
void doexit(int16 val, int16 val2);
#pragma wrapped-call(pop)

/* stuff below is static to scall1.c and should be pulled out of the header */
int min(int a, int b);
int psize(inoptr ino);
void addoff(off_t * ofptr, int amount);

#endif				/* __SCALL_H__ */
