//#include <ppu.h>

#define KB_ROW_COUNT	9
#define BYTE_BITS	8
#define KB_OVERALL	KB_ROW_COUNT * BYTE_BITS

extern unsigned char kb_rows[KB_ROW_COUNT];
extern unsigned char kb_map[KB_OVERALL];
extern void ppu_putc(char c);
extern char kb_checked;

void dump_keyboard(void)
{
	int i, bit;
	int overall = 0;

	if (kb_checked)
		return;

	kb_checked = 1;

	for (i = 0; i < KB_ROW_COUNT; i++) {
		for (bit = 0; bit < BYTE_BITS; bit++) {
			if (!(kb_rows[i] >> bit & 0x1))
				// FIXME this should not have the +2 here - that's stupid
				ppu_putc(kb_map[overall]);
			overall++;
		}
	}
}
