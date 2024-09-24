#ifndef __PPU_H__
#define __PPU_H__

void init_ppu(void);
void ppu_puts(char *s);
void ppu_putc(char c);
void ppu_spray(void);

/* This doesn't really belong here... */
void panic(char *s);

#define BUG_ON(cond)				\
	do {					\
		if (cond) {			\
			panic(#cond);		\
		}				\
	} while (0)

struct ppu_desc {
	char size;
	char *target;
	char flags;
};

#define SCREEN_VIS_ROWS		28
#define SCREEN_COLS		32
#define SCREEN_BUF_SIZE		SCREEN_VIS_ROWS * SCREEN_COLS

#define PPU_FIRST_VIS_ROW	(char *) 0x2020
#define PPU_VIS_END		PPU_FIRST_VIS_ROW + SCREEN_BUF_SIZE

#endif /* __PPU_H__ */
