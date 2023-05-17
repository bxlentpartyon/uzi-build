extern void __dump_screenbuf(void);
extern void ppu_reset(void);
extern void ei(void);

extern char *buffer_dump_pos;
extern char *ppu_dump_pos;
extern char screenbuf[];

#pragma zpsym ("buffer_dump_pos")
#pragma zpsym ("ppu_dump_pos")

#define SCREEN_BUF_SIZE 896
int cursor_pos = 0;

void putc(char c)
{
	char ptable_c = c - 0x20;
	screenbuf[cursor_pos] = ptable_c;
	cursor_pos += 1;

	if (cursor_pos >= SCREEN_BUF_SIZE)
		cursor_pos = 0;

	return;
}

void start_kernel(void)
{
	buffer_dump_pos = (char *) &screenbuf;
	ppu_dump_pos = (char *) 0x2020;

	putc('H');
	putc('E');
	putc('L');
	putc('L');
	putc('O');

	ei();

	while (1);
}

void dump_screenbuf(void)
{
	__dump_screenbuf();

	buffer_dump_pos += 16;
	ppu_dump_pos += 16;

	if (buffer_dump_pos == ((char *) &screenbuf) + SCREEN_BUF_SIZE) {
		buffer_dump_pos = (char *) &screenbuf;
		ppu_dump_pos = (char *) 0x2020;
	}
}

void handle_vblank(void)
{
	dump_screenbuf();
	ppu_reset();
}
