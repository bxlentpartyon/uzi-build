#include <ppu.h>

extern char *buffer_dump_pos;
extern char *ppu_dump_pos;
#pragma zpsym ("buffer_dump_pos")
#pragma zpsym ("ppu_dump_pos")

extern char screenbuf[];

#define SCREEN_BUF_START	(char *) &screenbuf

int cursor_pos = 0;

void next_line(void)
{
	int chars_left, i;

	chars_left = SCREEN_COLS - (cursor_pos % SCREEN_COLS);

	for (i = 0; i < chars_left; i++)
		screenbuf[cursor_pos++] = '*';
}

void ppu_putc(char c)
{
	if (c == '\r') {
		next_line();
	} else {
		screenbuf[cursor_pos++] = c;
	}

	BUG_ON(cursor_pos > SCREEN_BUF_SIZE);

	if (cursor_pos == SCREEN_BUF_SIZE)
		cursor_pos = 0;

	return;
}

void ppu_init_screenbuf(void)
{
	buffer_dump_pos = SCREEN_BUF_START;
	ppu_dump_pos = PPU_FIRST_VIS_ROW;
}
