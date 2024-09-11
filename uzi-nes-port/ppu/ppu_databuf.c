#include <ppu.h>
#include <lib/string.h>

// FIXME duplicates of stuff in asm/ppu_databuf.S
#define	PPU_MAX_WRITE	64	; Max number of bytes we can write to the PPU in one VBlank
#define PPU_BUF_BYTE	5	; Write buffer entry size for a single byte
#define PPU_BUF_SIZE	PPU_MAX_WRITE * PPU_BUF_BYTE

extern char *ppu_databuf;
extern int databuf_pos;
extern char *cur_screen_ptr;
extern void wait_frame(void);

int cursor_pos = 0;

void ppu_putc(char c)
{
	struct ppu_desc desc = {
		.size = 1,
		.target = cur_screen_ptr,
		.flags = 0,
		.data = c,
	};

	if (databuf_pos + sizeof(struct ppu_desc) + 1 > PPU_BUF_SIZE)
		wait_frame();

	bcopy(&desc, ppu_databuf + databuf_pos, sizeof(struct ppu_desc));
	databuf_pos += sizeof(struct ppu_desc);
	ppu_databuf[databuf_pos] = 0;
	databuf_pos++;
}
