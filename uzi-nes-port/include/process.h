#ifndef __PROCESS_H__
#define __PROCESS_H__

void psleep(void *event);
void sendsig(ptptr proc, int16 sig);
void wakeup(char *event);
void ssig(register ptptr proc, int16 sig);
void chksigs(void);
void swapin(ptptr pp);
void init2(void);
ptptr getproc(void);

#endif /* __PROCESS_H__ */
