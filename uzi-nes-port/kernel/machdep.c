/**************************************************
UZI (Unix Z80 Implementation) Kernel:  machdep.c
***************************************************/

#include <ppu.h>
#include <process.h>

extern void ei(void);

void start_kernel(void)
{
	init_ppu();

	ppu_puts("boot:");

	ei();

	init2();
}
