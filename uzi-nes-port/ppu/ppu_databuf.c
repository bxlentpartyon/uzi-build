#include <machdep.h>
#include <ppu.h>
#include <lib/string.h>

#define	PPU_MAX_WRITE	16				// Max number of bytes we can write to the PPU in one VBlank
#define PPU_BUF_BYTE	sizeof(struct ppu_desc) + 1	// Write buffer entry size for a single byte
#define PPU_BUF_SIZE	(PPU_MAX_WRITE * PPU_BUF_BYTE)

extern char ppu_databuf[];
extern int databuf_pos;
extern void wait_frame(void);
extern char y_scroll_coarse;
extern int y_scroll_fine;

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

void queue_descriptor(struct ppu_desc *desc, char *data)
{
	char datalen = 0;

	if (desc->flags & PPU_DESC_FLAG_READ)
		datalen = 1;
	else if (desc->flags & PPU_DESC_FLAG_NULL)
		datalen = 0;
	else
		datalen = desc->size;
		
	if (databuf_pos + sizeof(struct ppu_desc) + datalen + 1 > PPU_BUF_SIZE)
		wait_frame();

	bcopy((char *) desc, ppu_databuf + databuf_pos, sizeof(struct ppu_desc));
	databuf_pos += sizeof(struct ppu_desc);

	if (datalen) {
		bcopy(data, ppu_databuf + databuf_pos, datalen);
		databuf_pos += datalen;
	}

	ppu_databuf[databuf_pos] = 0;
}

void test_ppu_read(void)
{
	struct ppu_desc desc;
	char data = 0x12;

	desc.size = 64;
	desc.target = 0x1400;
	desc.flags = PPU_DESC_FLAG_READ;

	wait_frame();
	di();
	lock_databuf();
	queue_descriptor(&desc, &data);
	wait_frame();
	unlock_databuf();
	ei();
}

void write_blank_line_desc(void)
{
	struct ppu_desc desc;

	desc.size = SCREEN_COLS;
	desc.target = cur_screen_ptr;
	desc.flags = PPU_DESC_FLAG_NULL;

	queue_descriptor(&desc, NULL);
}

void scroll_one_row(void)
{
	/* Blank out the next line before scrolling */
	wait_frame();
	write_blank_line_desc();
	wait_frame();

	y_scroll_fine += SCREEN_ROW_PX;
	if (y_scroll_fine >= SCREEN_ROW_PX * SCREEN_ROWS) {
		y_scroll_coarse = (y_scroll_coarse ? 0 : 1);
		y_scroll_fine = 1;
	}
}

void update_screen_ptrs(short dist)
{
	int orig_nametable_pos = cur_nametable_pos;
	char cur_x, orig_x;

	if (!dist)
		return;

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
		cur_x = cur_nametable_pos % SCREEN_COLS;
		orig_x = orig_nametable_pos % SCREEN_COLS;
		/* Check if we wrapped off the end of the line */
		if (cur_x < orig_x)
			scroll_one_row();
		/*
		 * Detect the case where we got a newline on a currently empty
		 * line.  Note that we don't need to check if dist > 0 because
		 * we can't reach this point with a 0 dist.
		 */
		else if (cur_x == orig_x && cur_x == 0)
			scroll_one_row();
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
	char data;

	if (c == '\r') {
		di();
		lock_databuf();
		next_line();
		unlock_databuf();
		ei();

		return;
	}

	desc.size = 1;
	desc.target = cur_screen_ptr;
	desc.flags = PPU_DESC_FLAGS_EMPTY;
	data = c;

	/*
	 * We need to disable interrupts to avoid having a vblank occur with a
	 * partially-written descriptor in the data buffer
	 */
	di();
	lock_databuf();
	queue_descriptor(&desc, &data);
	update_screen_ptrs(1);
	unlock_databuf();
	ei();
}

/*
 * 1698 CPU cycles for fully loaded buffer NMI (i.e. fully loaded with 1-byte descriptors)
 * 1019 CPU cycles for fully loaded buffer write_ppubuf (15 bytes)
 *
 * 1100 CPU cycles for a 32-byte NULL descriptor NMI
 * 754 CPU cycles for a 32-byte NULL descriptor handle_vblank
 * 1388 CPU cycles for a 64-byte NULL descriptor NMI
 * 1964 CPU cycles for a 128-byte NULL descriptor NMI
 *
 * 788 CPU cycles for single byte write NMI
 * 109 CPU cycles for single byte write_ppubuf
 *
 * NMI overhead is 679 CPU cycles
 *
 * 1019 cycles - 109 cycles = 15 byte write - 1 byte write = 910 cycles for 14 byte write => 65 cycles/descriptor min
 *
 * 2000 CPU cycles for a 64-byte read descriptor
 */
