#include <ppu.h>

extern void ei(void);

void start_kernel(void)
{
	init_ppu();

	puts("boot:");

	ei();

	while (1);
}
