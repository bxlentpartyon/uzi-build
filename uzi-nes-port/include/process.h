#ifndef __PROCESS_H__
#define __PROCESS_H__

void psleep(void *event);
void wakeup(char *event);
void chksigs(void);
void swapin(ptptr pp);
void init2(void);
ptptr getproc(void);
int clk_int(void);

extern uint16 clk_int_count;
extern uint16 tick_count;

#endif /* __PROCESS_H__ */
