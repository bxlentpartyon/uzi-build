#ifndef __MACHDEP_H__
#define __MACHDEP_H__

#include <unix.h>
#include <nes/mmc5.h>

void kprintf(char *fmt, ...);
void panic(char *fmt, ...);
void sprintf(char *str, char *fmt, ...);
void ei(void);
void di(void);
int valadr(char *base, uint16 size);
void user_bank_setup(void);
void tempstack(void);
void warning(char *s);
void start_clock(void);
void stop_clock(void);
void __fastcall__ debug_break(int state);
char * __fastcall__ fork_prep_begin(int pid);
void __fastcall__ fork_prep_end(void);
char * __fastcall__ swapout_prep(void);
void calltrap(void);

#define MAX_PROGRAM_SIZE	MMC5_PRG_MODE3_BANK_SIZE

#endif				/* __MACHDEP_H__ */
