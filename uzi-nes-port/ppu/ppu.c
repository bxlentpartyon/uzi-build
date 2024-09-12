#include <ppu.h>

extern void ppu_load_font(void);
extern void ppu_init_databuf(void);
#ifdef PPU_SCREENBUF
extern void ppu_init_screenbuf(void);
#endif

int in_panic = 0;

void init_ppu(void)
{
	ppu_load_font();
	ppu_init_databuf();
#ifdef PPU_SCREENBUF
	ppu_init_screenbuf();
#endif
}

void ppu_puts(char *s)
{
	int pos = 0;
	char cur_char = *s;

	while (cur_char != 0x00) {
		ppu_putc(cur_char);
		cur_char = s[++pos];
	}
}

void panic(char *msg)
{
	/* prevent recursion from ppu_put* functions */
	if (!in_panic) {
		in_panic = 1;
#ifdef PPU_SCREENBUF
		cursor_pos = 0;
#endif
		ppu_puts("PANIC: ");
		ppu_puts(msg);
		while(1);
	}
}
