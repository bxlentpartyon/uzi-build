#ifndef __PPU_H__
#define __PPU_H__

void init_ppu(void);
void ppu_puts(char *s);
void ppu_putc(char c);
void panic(char *s);

#endif /* __PPU_H__ */
