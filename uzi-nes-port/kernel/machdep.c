/**************************************************
UZI (Unix Z80 Implementation) Kernel:  machdep.c
***************************************************/

#include <ppu.h>
#include <process.h>

#include <stdarg.h>

extern void ei(void);

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

/* Short version of printf to save space */
/* TODO: fix the vararg handling here */
void kprintf(char *fmt, ...)
        {
	va_list ap;
        register char **arg;
        register int c, base;
        //char s[7]; //, *itob();

	va_start(ap, fmt);

        while (c = *fmt++) {
                if (c != '%') {
                        kputchar(c);
                        continue;
                        }
                switch (c = *fmt++) {
                case 'c':
                        kputchar(*--arg);
                        continue;
/* this stuff is broken until I have itob()
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
                prt:
                        //puts(itob(*--arg, s, base));
                        continue;
*/
                case 's':
                        puts(va_arg(ap, char *));
                        continue;
                default:
                        kputchar(c);
                        continue;
                        }
                }

	//va_end(ap);
        }

void start_kernel(void)
{
	init_ppu();

	ei();

	init2();
}
