#include <ppu.h>

extern void __dump_screenbuf(void);
extern void ppu_reset(void);

extern char *buffer_dump_pos;
extern char *ppu_dump_pos;
#pragma zpsym ("buffer_dump_pos")
#pragma zpsym ("ppu_dump_pos")

extern char screenbuf[];

#define SCREEN_VIS_ROWS		28
#define SCREEN_COLS		32
#define SCREEN_BUF_SIZE		SCREEN_VIS_ROWS * SCREEN_COLS
#define SCREEN_BUF_START	(char *) &screenbuf

#define PPU_FIRST_VIS_ROW	(char *) 0x2020;

int cursor_pos = 0;

void putc(char c)
{
	screenbuf[cursor_pos++] = c;

	if (cursor_pos >= SCREEN_BUF_SIZE)
		cursor_pos = 0;

	return;
}

void puts(char *s)
{
	int pos = 0;
	char cur_char = *s;

	while (cur_char != 0x00) {
		putc(cur_char);
		cur_char = s[++pos];
	}
}

void dump_screenbuf(void)
{
	__dump_screenbuf();

	buffer_dump_pos += 128;
	ppu_dump_pos += 128;

	if (buffer_dump_pos == SCREEN_BUF_START + SCREEN_BUF_SIZE) {
		buffer_dump_pos = SCREEN_BUF_START;
		ppu_dump_pos = PPU_FIRST_VIS_ROW;
	}
}

void handle_vblank(void)
{
	dump_screenbuf();
	ppu_reset();
}

void init_ppu(void)
{
	buffer_dump_pos = SCREEN_BUF_START;
	ppu_dump_pos = PPU_FIRST_VIS_ROW;
}
