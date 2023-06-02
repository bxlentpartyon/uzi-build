#ifndef __PROCESS_H__
#define __PROCESS_H__

void psleep(void *event);
void sendsig(ptptr proc, int16 sig);
void wakeup(char *event);
void ssig(register ptptr proc, int16 sig);
void chksigs(void);
void swapin(ptptr pp);
ptptr getproc(void);
int dofork(void);
void init2(void);
int clk_int(void);

/* only referenced inside process.c */
void newproc(ptptr p);
int swapout(void);
void swrite(void);

#endif /* __PROCESS_H__ */
