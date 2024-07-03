#include <ppu.h>

extern char screenbuf[];
extern char screenbuf_advance;

#define SCREEN_VIS_ROWS		28
#define SCREEN_COLS		32
#define SCREEN_BUF_SIZE		SCREEN_VIS_ROWS * SCREEN_COLS

int cursor_pos = 0;
int in_panic = 0;

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
		cursor_pos = 0;
		ppu_puts("PANIC: ");
		ppu_puts(msg);
		while(1);
	}
}
