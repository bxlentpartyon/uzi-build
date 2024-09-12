#include <machdep.h>
#include <ppu.h>
#include <lib/string.h>

// FIXME duplicates of stuff in asm/ppu_databuf.S
#define	PPU_MAX_WRITE	64	// Max number of bytes we can write to the PPU in one VBlank
#define PPU_BUF_BYTE	5	// Write buffer entry size for a single byte
#define PPU_BUF_SIZE	PPU_MAX_WRITE * PPU_BUF_BYTE

extern char *ppu_databuf;
extern int databuf_pos;
extern void wait_frame(void);

char *cur_screen_ptr = PPU_FIRST_VIS_ROW;

void ppu_putc(char c)
{
	struct ppu_desc desc;
	desc.size = 1;
	desc.target = cur_screen_ptr;
	desc.flags = 0;
	desc.data = c;

	if (databuf_pos + sizeof(struct ppu_desc) + 1 > PPU_BUF_SIZE)
		wait_frame();

	/*
	 * We need to disable interrupts to avoid having a vblank occur with a
	 * partially-written descriptor in the data buffer
	 */
	di();

	bcopy(&desc, &ppu_databuf + databuf_pos, sizeof(struct ppu_desc));
	databuf_pos += sizeof(struct ppu_desc);
	ppu_databuf[databuf_pos] = 0;

	if (++cur_screen_ptr > PPU_FIRST_VIS_ROW + SCREEN_BUF_SIZE)
		cur_screen_ptr = PPU_FIRST_VIS_ROW;

	ei();
}
