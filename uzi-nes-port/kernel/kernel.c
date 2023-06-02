#include <ppu.h>
#include <process.h>

extern void ei(void);

void start_kernel(void)
{
	init_ppu();

	puts("boot:");

	ei();

	init2();
}
