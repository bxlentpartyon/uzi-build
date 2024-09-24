#include <machdep.h>
#include <ppu.h>
#include <lib/string.h>

#define	PPU_MAX_WRITE		16				// Max number of bytes we can write to the PPU in one VBlank
#define PPU_BUF_BYTE		sizeof(struct ppu_desc) + 1	// Write buffer entry size for a single byte
#define PPU_BUF_SIZE		PPU_MAX_WRITE * PPU_BUF_BYTE
#define PPU_MAX_DESC_BYTES	16
#define NULL_BYTE_SIZE		1

extern char ppu_databuf[];
extern int databuf_pos;
extern void wait_frame(void);

char ppu_descbuf[PPU_MAX_DESC_BYTES];
char *cur_screen_ptr = PPU_FIRST_VIS_ROW;
char ppu_descbuf_pos = 0;

#define CUR_DESC_BYTES	ppu_descbuf_pos + 1
#define CUR_DESC_SIZE	sizeof(struct ppu_desc) + CUR_DESC_BYTES 

void dump_current_descriptor(void)
{
	struct ppu_desc desc;

	if (databuf_pos + CUR_DESC_SIZE + NULL_BYTE > PPU_BUF_SIZE)
		wait_frame();

	desc.size = 1;
	desc.target = cur_screen_ptr;
	desc.flags = 0;

	/*
	 * We need to disable interrupts to avoid having a vblank occur with a
	 * partially-written descriptor in the data buffer
	 */
	di();

	bcopy(&desc, ppu_databuf + databuf_pos, sizeof(struct ppu_desc));
	databuf_pos += sizeof(struct ppu_desc);
	bcopy(ppu_descbuf, ppu_databuf + databuf_pos, CUR_DESC_BYTES);
	databuf_pos += CUR_DESC_BYTES;
	ppu_databuf[databuf_pos] = 0;
	
	if (cur_screen_ptr + CUR_DESC_BYTES > PPU_VIS_END)
		cur_screen_ptr = PPU_FIRST_VIS_ROW;

	ei();
}

void next_line(void)
{
	int chars_left = SCREEN_COLS - ((unsigned int) cur_screen_ptr % SCREEN_COLS);

	/* If we're on the last visible row, wrap around */
	if (cur_screen_ptr + chars_left < PPU_VIS_END)
		cur_screen_ptr += chars_left;
	else
		cur_screen_ptr = PPU_FIRST_VIS_ROW;
}

void ppu_putc(char c)
{
	if (c == '\r') {
		next_line();
		return;
	}

	if (CUR_DESC_BYTES + 1 > PPU_MAX_DESC_BYTES
}

/*
 * 1698 CPU cycles for fully loaded buffer NMI
 * 1019 CPU cycles for fully loaded buffer write_ppubuf (15 bytes)
 * 788 CPU cycles for single byte write NMI
 * 109 CPU cycles for single byte write_ppubuf
 *
 * NMI overhead is 679 CPU cycles
 *
 * 1019 cycles - 109 cycles = 15 byte write - 1 byte write = 910 cycles for 14 byte write => 65 cycles/descriptor min
 */
