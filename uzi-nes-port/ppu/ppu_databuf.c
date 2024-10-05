#include <machdep.h>
#include <ppu.h>
#include <lib/string.h>

#define	PPU_MAX_WRITE	16	// Max number of bytes we can write to the PPU in one VBlank
#define PPU_BUF_BYTE	5	// Write buffer entry size for a single byte
#define PPU_BUF_SIZE	PPU_MAX_WRITE * PPU_BUF_BYTE

extern char ppu_databuf[];
extern int databuf_pos;
extern void wait_frame(void);
extern int y_scroll_pos;

char *cur_screen_ptr = PPU_FIRST_VIS_ROW;
int cur_nametable_pos = SCREEN_COLS;
char scroll_started = 0;
char cur_nametable = 0;

void swap_nametable(void)
{
	cur_nametable_pos = 0;
	if (cur_nametable == 0) {
		cur_nametable = 1;
		cur_screen_ptr = PPU_TABLE1_ADDR;
	} else {
		cur_nametable = 0;
		cur_screen_ptr = PPU_TABLE0_ADDR;
	}
}

void update_screen_ptrs(short dist)
{
	int orig_nametable_pos = cur_nametable_pos;
	cur_screen_ptr += dist;
	cur_nametable_pos += dist;

	if (cur_nametable_pos >= SCREEN_SIZE)
		swap_nametable();

	/*
	 * The math for starting the scroll is a bit cryptic.  We have to add
	 * an extra row here (i.e. SCREEN_COLS) to account for the initially
	 * non-visible first row.
	 */
	if (!scroll_started && cur_nametable_pos >= SCREEN_VIS_SIZE + SCREEN_COLS)
		scroll_started = 1;

	if (scroll_started) {
		if ((cur_nametable_pos % SCREEN_COLS) < (orig_nametable_pos % SCREEN_COLS)) {
			y_scroll_pos = (y_scroll_pos + SCREEN_ROW_PX) % SCREEN_BUF_PX;
		}
	}
}

void next_line(void)
{
	int chars_left;

	chars_left = SCREEN_COLS - ((unsigned int) cur_screen_ptr % SCREEN_COLS);
	update_screen_ptrs(chars_left);
}

void ppu_putc(char c)
{
	struct ppu_desc desc;

	if (c == '\r') {
		di();
		next_line();
		ei();

		return;
	}

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

	bcopy(&desc, ppu_databuf + databuf_pos, sizeof(struct ppu_desc));
	databuf_pos += sizeof(struct ppu_desc);
	ppu_databuf[databuf_pos] = 0;

	update_screen_ptrs(1);

	ei();
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
