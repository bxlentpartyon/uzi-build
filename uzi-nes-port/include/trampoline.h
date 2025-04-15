#ifndef __TRAMPOLINE_H__
#define __TRAMPOLINE_H__

extern void trampoline(void);

#define INITDATA_PAGE	0
#define FONTDATA_PAGE	1
#define DEVIO_PAGE	2
#define DEVTTY_PAGE	3
#define SCALL1_PAGE	4
#define SCALL2_PAGE	5
#define PROCESS_PAGE	6
#define FS_PAGE		7
#define PPU_PAGE	8
#define SCALL1B_PAGE	9
#define CORE_PAGE	15
#define NUM_CODE_PAGES	16

#endif				/* __TRAMPOLINE_H__ */
