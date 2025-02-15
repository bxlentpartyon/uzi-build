#include <machdep.h>

#define KB_ROW_COUNT	9
#define BYTE_BITS	8
#define KB_OVERALL	KB_ROW_COUNT * BYTE_BITS

extern unsigned char kb_rows[KB_ROW_COUNT];
extern unsigned char kb_map[KB_OVERALL];
extern void ppu_putc(char c);
extern char kb_checked;

unsigned char prev_kb_rows[KB_ROW_COUNT];

void reset_prev_kb_rows(void)
{
	int i;

	for (i = 0; i < KB_ROW_COUNT; i++)
		prev_kb_rows[i] = 0x00;
}

void dump_keyboard(void)
{
	int i, bit;
	int overall = 0;

	if (kb_checked)
		return;

	kb_checked = 1;

	for (i = 0; i < KB_ROW_COUNT; i++) {
		for (bit = 0; bit < BYTE_BITS; bit++) {
			/*
			 * we only print the char if the key was "freshly" pressed
			 * on this frame
			 */
			if ((kb_rows[i] >> bit & 0x1) &&
			    !(prev_kb_rows[i] >> bit & 0x1))
				ppu_putc(kb_map[overall]);
			overall++;
		}

		prev_kb_rows[i] = kb_rows[i];
	}
}

#ifdef KB_DEBUG
void print_kb_bytes(void)
{
	int i;

	kprintf("KB: ");
	for (i = 0; i < 9; i++)
		kprintf("%x ", kb_rows[i]);
	kprintf("\n");

	return;
}
#endif
