#ifndef __ASM_H__
#define __ASM_H__

void __fastcall__ ppu_setup(void);
void __fastcall__ ppu_on(void);
void __fastcall__ reset_cursor(void);
void __fastcall__ put_chr(char ch);

#endif /* __ASM_H__ */
