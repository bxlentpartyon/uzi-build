#include <ppu.h>

extern void ppu_load_font(void);
extern void ppu_init_screenbuf(void);

void init_ppu(void)
{
	ppu_load_font();
	ppu_init_screenbuf();
}
