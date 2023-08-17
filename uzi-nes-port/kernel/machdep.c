/**************************************************
UZI (Unix Z80 Implementation) Kernel:  machdep.c
***************************************************/

#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <unix.h>
#include <lib/string.h>

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

#define KPRINTF_BUF_SIZE	20

/* Short version of printf to save space */
/* TODO: fix the vararg handling here */
void kprintf(char *fmt, ...)
        {
	va_list ap;
        register int c, base;
        char s[KPRINTF_BUF_SIZE];

	va_start(ap, fmt);

        while (c = *fmt++) {
                if (c != '%') {
                        kputchar(c);
                        continue;
                        }
                switch (c = *fmt++) {
                case 'c':
                        kputchar(va_arg(ap, char));
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
			puts(s);
                        continue;
                case 's':
                        puts(va_arg(ap, char *));
                        continue;
                default:
                        panic("bad char");
                        continue;
                        }
                }

	va_end(ap);
        }

int nr_apu_irqs = 0;
extern unsigned char apu_status_byte;

void handle_irq(void)
{
	if (apu_status_byte & 0x40) {
		nr_apu_irqs++;
		clk_int();
	} else {
		panic("spurious IRQ");
	}
}

void rdtime(time_t *tloc)
{
    di();
    tloc->t_time = tod.t_time;
    tloc->t_date = tod.t_date;
    ei();
}

/* Update global time of day */
void rdtod(void)
{
    tod.t_time = (tread(SECS)>>1) | (tread(MINS)<<5) | (tread(HRS)<<11);
    tod.t_date = tread(DAY) | (tread(MON)<<5) | (YEAR<<9);
}

void start_kernel(void)
{
	di();

	init_ppu();

	/* Turn off clock */
	stop_clock();

	ei();

	init2();	/* in process.c */
}
