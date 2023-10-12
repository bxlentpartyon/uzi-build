#ifndef __PROCESS_H__
#define __PROCESS_H__

void chksigs(void);
void init2(void);
int clk_int(void);

extern uint16 clk_int_count;
extern uint16 tick_count;

#endif /* __PROCESS_H__ */
