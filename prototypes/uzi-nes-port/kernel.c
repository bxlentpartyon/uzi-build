extern void dump_screenbuf(void);
extern void ppu_reset(void);

#define SCREEN_BUF_SIZE 10
char screenbuf[10] = { 'f', 'u', 'c', 'k', 0, 0, 0, 0, 0, 0 };
char *screenbuf_chunk1 = screenbuf;
char *screenbuf_chunk2 = screenbuf + 240;
char *screenbuf_chunk3 = screenbuf + 480;
char *screenbuf_chunk4 = screenbuf + 720;
int bufpos = 0;

void putc(char c)
{
	char ptable_c = c - 0x20;
	screenbuf[bufpos] = ptable_c;
	//screenbuf[bufpos] = c;
	bufpos += 1;

	if (bufpos > SCREEN_BUF_SIZE)
		bufpos = 0;

	return;
}

void start_kernel(void)
{
	putc('H');
	putc('E');
	putc('L');
	putc('L');
	putc('O');
	while (1);
}

void handle_vblank(void)
{
	dump_screenbuf();
	ppu_reset();
}
