#ifndef __TRAMPOLINE_H__
#define __TRAMPOLINE_H__

extern void trampoline(void);

#define INITDATA_PAGE	0
#define FONTDATA_PAGE	1
#define CORE_PAGE	2
#define DEVIO_PAGE	3
#define NUM_CODE_PAGES	4

#endif /* __TRAMPOLINE_H__ */
