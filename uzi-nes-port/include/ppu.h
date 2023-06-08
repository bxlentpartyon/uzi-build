#ifndef __PPU_H__
#define __PPU_H__

void init_ppu(void);
void ppu_puts(char *s);
void ppu_putc(char c);

/* This doesn't really belong here... */
void panic(char *s);

#define BUG_ON(cond)				\
	do {					\
		if (cond) {			\
			panic(#cond);		\
		}				\
	} while (0)


#endif /* __PPU_H__ */
