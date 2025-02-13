/**************************************************
UZI (Unix Z80 Implementation) Kernel:  machdep.c
***************************************************/

#include <extern.h>
#include <interrupts.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <unix.h>
#include <lib/string.h>

#include <stdarg.h>

extern void ei(void);

char printbuf[SCREEN_VIS_SIZE];
void kputchar(char c);

void puts(char *s)
{
	while (*s)
		kputchar(*(s++));
}

#define _putc ppu_putc

void kputchar(char c)
{
	if (c == '\n') {
		_putc('\r');
		return;
	} else if (c == '\t') {
		puts("\177\177\177\177\177\177\177\177\177\177");
		return;
	}

	_putc(c);
}

void strcpy_idx(char *dest, char *src, int *idx)
{
	int copy_idx = 0;

	while(src[copy_idx]) {
		dest[copy_idx] = src[copy_idx];
		copy_idx++;
	}

	*idx = *idx + copy_idx;
	dest[copy_idx] = 0;
}

#define KPRINTF_BUF_SIZE	20

/* Short version of printf to save space */
void vsprintf(char *str, char *fmt, va_list ap)
        {
        register int c, base;
	int idx = 0;
        char s[KPRINTF_BUF_SIZE];
	char ctmp[] = { 0, 0 };

        while (c = *fmt++) {
                if (c != '%') {
			ctmp[0] = c;
			strcpy_idx(str + idx, &ctmp, &idx);
                        continue;
                        }
                switch (c = *fmt++) {
                case 'c':
			ctmp[0] = va_arg(ap, char);
			strcpy_idx(str + idx, &ctmp, &idx);
                        continue;
                case 'd':
                        base = -10;
                        goto prt;
                case 'o':
                        base = 8;
                        goto prt;
                case 'u':
                        base = 10;
                        goto prt;
                case 'x':
                        base = 16;
			goto prt;
		case 'b':
			base = 2;
                prt:
			bzero(s, KPRINTF_BUF_SIZE);
                        itob(va_arg(ap, int), s, base);
			strcpy_idx(str + idx, s, &idx);
                        continue;
                case 's':
                        strcpy_idx(str + idx, va_arg(ap, char *), &idx);
                        continue;
                default:
			bzero(s, KPRINTF_BUF_SIZE);
			s[0] = '%';
			s[1] = c;
			strcpy_idx(str + idx, s, &idx);
                        continue;
                        }
                }
        }

void sprintf(char *str, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);
}

void kprintf(char *fmt, ...)
{
	va_list ap;

	bzero(&printbuf, SCREEN_VIS_SIZE);

	va_start(ap, fmt);
	vsprintf(printbuf, fmt, ap);
	va_end(ap);

	puts(printbuf);
}

void start_kernel(void)
{
	di();

	init_ppu();

	ei();

	init2();	/* in process.c */
}
