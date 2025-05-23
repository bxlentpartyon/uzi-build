#include <interrupts.h>
#include <machdep.h>
#include <ppu.h>
#include <lib/string.h>

#define	PPU_MAX_CHAR_WRITE	16	// Max number of bytes we can write to the PPU in one VBlank
#define PPU_BUF_BYTE	sizeof(struct ppu_desc) + 1	// Write buffer entry size for a single byte
#define PPU_BUF_SIZE	(PPU_MAX_CHAR_WRITE * PPU_BUF_BYTE)

#pragma code-name (push, "PPU_CODE")

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

enum ppu_desc_type {
	PPU_VIDEO_DESC,
	PPU_READ_DESC,
	PPU_WRITE_DESC,
	PPU_NULL_DESC,
	PPU_BAD_DESC
};

enum ppu_desc_type get_desc_type(struct ppu_desc *desc)
{
	char low_flag_bits = desc->flags & 0x0f;
	char high_flag_bits = desc->flags & 0xf0;

	if (!desc->flags) {
		return PPU_VIDEO_DESC;
	} else if (low_flag_bits & PPU_DESC_FLAG_READ) {
		return PPU_READ_DESC;
	} else if (low_flag_bits & PPU_DESC_FLAG_WRITE) {
		return PPU_WRITE_DESC;
	} else if (low_flag_bits & PPU_DESC_FLAG_NULL) {
		return PPU_NULL_DESC;
	} else {
		return PPU_BAD_DESC;
	}
}

#define PPU_LOOP_MAX_CYCLES		2200

#define BASE_PPU_LOOP_CYCLES		171
#define VIDEO_DESC_OVERHEAD_CYCLES	74
#define RW_DESC_OVERHEAD_CYCLES		115
#define NULL_DESC_OVERHEAD_CYCLES	78

#define VIDEO_DESC_PERBYTE_CYCLES	20
#define RW_DESC_PERBYTE_CYCLES		20
#define NULL_DESC_PERBYTE_CYCLES	10

unsigned int calc_desc_weight(struct ppu_desc *desc)
{
	unsigned int desc_cycles, perbyte_cycles;

	switch (get_desc_type(desc)) {
	case PPU_VIDEO_DESC:
		desc_cycles = VIDEO_DESC_OVERHEAD_CYCLES;
		perbyte_cycles = VIDEO_DESC_PERBYTE_CYCLES;
		break;
	case PPU_READ_DESC:
		/* fall through */
	case PPU_WRITE_DESC:
		desc_cycles = RW_DESC_OVERHEAD_CYCLES;
		perbyte_cycles = RW_DESC_PERBYTE_CYCLES;
		break;
	case PPU_NULL_DESC:
		desc_cycles = NULL_DESC_OVERHEAD_CYCLES;
		perbyte_cycles = NULL_DESC_OVERHEAD_CYCLES;
		break;
	case PPU_BAD_DESC:
		panic("bad PPU desc");
	}

	return desc_cycles + desc->size * perbyte_cycles;
}

int __queue_descriptor(struct ppu_desc *desc, char *data)
{
	char datalen = 0;

	if (desc->flags & PPU_DESC_FLAG_READ)
		datalen = 1;
	else if (desc->flags & PPU_DESC_FLAG_NULL)
		datalen = 0;
	else if (desc->flags & PPU_DESC_FLAG_WRITE)
		/* Add one for the bank byte here */
		/* FIXME this should use a macro like BANK_BYTE_PAD */
		datalen = desc->size + 1;
	else
		datalen = desc->size;

	ppu_lock();

	if (databuf_pos + sizeof(struct ppu_desc) + datalen + 1 > PPU_BUF_SIZE)
		goto unlock_again;
	if (calc_desc_weight(desc) + BASE_PPU_LOOP_CYCLES > PPU_LOOP_MAX_CYCLES)
		goto unlock_again;

	bcopy((char *)desc, ppu_databuf + databuf_pos, sizeof(struct ppu_desc));
	databuf_pos += sizeof(struct ppu_desc);

	if (datalen) {
		bcopy(data, ppu_databuf + databuf_pos, datalen);
		databuf_pos += datalen;
	}

	ppu_databuf[databuf_pos] = 0;

	ppu_unlock();

	return 0;

 unlock_again:
	ppu_unlock();
	return -EAGAIN;
}

void queue_descriptor(struct ppu_desc *desc, char *data)
{
	int ret = 0;

 queue_again:
	ret = __queue_descriptor(desc, data);

	if (ret == -EAGAIN) {
		wait_frame();
		goto queue_again;
	}
}

void test_ppu_read(void)
{
	struct ppu_desc desc;
	char data = 0x00;

	desc.size = 64;
	desc.target = 0x1200;
	desc.flags = PPU_DESC_FLAG_READ;

	queue_descriptor(&desc, &data);
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
	write_blank_line_desc();

	ppu_lock();

	y_scroll_fine += SCREEN_ROW_PX;
	if (y_scroll_fine >= SCREEN_ROW_PX * SCREEN_ROWS) {
		y_scroll_coarse = (y_scroll_coarse ? 0 : 1);
		y_scroll_fine = 1;
	}

	ppu_unlock();
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
	if (!scroll_started
	    && cur_nametable_pos >= SCREEN_VIS_SIZE + SCREEN_COLS)
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

	chars_left = SCREEN_COLS - ((unsigned int)cur_screen_ptr % SCREEN_COLS);
	update_screen_ptrs(chars_left);
}

void ppu_putc(char c)
{
	struct ppu_desc desc;
	char data;

	if (c == '\r') {
		next_line();
		return;
	}

	desc.size = 1;
	desc.target = cur_screen_ptr;
	desc.flags = PPU_DESC_FLAGS_EMPTY;
	data = c;

	queue_descriptor(&desc, &data);
	update_screen_ptrs(1);
}

#pragma code-name (pop)

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
