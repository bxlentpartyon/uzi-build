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
	char data;
};

#define SCREEN_VIS_ROWS		28
#define SCREEN_ROWS		30
#define SCREEN_COLS		32
#define SCREEN_SIZE		(SCREEN_ROWS * SCREEN_COLS)
#define SCREEN_VIS_SIZE		(SCREEN_VIS_ROWS * SCREEN_COLS)

#define SCREEN_NUM_TABLES	2
#define SCREEN_ROW_PX		8
#define SCREEN_BUF_PX		(SCREEN_ROW_PX * SCREEN_ROWS * SCREEN_NUM_TABLES)

#define PPU_FIRST_VIS_ROW	(char *) 0x2020
#define	PPU_TABLE0_ADDR		(char *) 0x2000
#define	PPU_TABLE1_ADDR		(char *) 0x2800

#endif /* __PPU_H__ */
