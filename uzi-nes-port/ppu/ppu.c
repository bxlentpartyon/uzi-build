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
#define SCREEN_LAST_ROW_IDX	27
#define SCREEN_BUF_SIZE		SCREEN_VIS_ROWS * SCREEN_COLS
#define SCREEN_BUF_START	(char *) &screenbuf

#define PPU_FIRST_VIS_ROW	(char *) 0x2020;

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

	if (cursor_pos >= SCREEN_BUF_SIZE)
		cursor_pos = 0;

	return;
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

void panic(char *s)
{
	cursor_pos = 0;
	ppu_puts("PANIC: ");
	ppu_puts(s);
	while(1);
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
