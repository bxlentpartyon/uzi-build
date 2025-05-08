#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <unix.h>

int clk_int(void);

extern uint16 clk_int_count;
extern uint16 tick_count;

#endif				/* __INTERRUPTS_H__ */
