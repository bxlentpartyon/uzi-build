/**************************************************
UZI (Unix Z80 Implementation) Kernel:  machdep.c
***************************************************/

#include <extern.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <unix.h>
#include <lib/string.h>

#include <stdarg.h>

extern void ei(void);

char printbuf[SCREEN_BUF_SIZE];
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
                        panic("bad char");
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

	/* TODO: size shouldn't be hard-coded here */
	bzero(&printbuf, 896);

	va_start(ap, fmt);
	vsprintf(printbuf, fmt, ap);
	va_end(ap);

	puts(printbuf);
}

int nr_apu_irqs = 0;
extern unsigned char apu_status_byte;
#pragma zpsym ("apu_status_byte");

extern unsigned char kb_rows[9];

void print_kb_bytes(void)
{
	int i;

	kprintf("KB: ");
	for (i = 0; i < 9; i++)
		kprintf("%x ", kb_rows[i]);
	kprintf("\n");

	return;
}

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

/* Port addresses of clock chip registers. */

#define SECS 0xe2
#define MINS 0xe3
#define HRS 0xe4
#define DAY 0xe6
#define MON 0xe7
#define YEAR 86

/* Read BCD clock register, convert to binary. */
uint16 tread(uint16 port)
{
	int n;

#define SECS_PER_MIN	60
#define MINS_PER_HOUR	60
#define HRS_PER_DAY	24
	switch(port) {
		case SECS:
			n = (tick_count / TICKSPERSEC) % SECS_PER_MIN;
			break;
		case MINS:
			n = (tick_count / TICKSPERSEC / SECS_PER_MIN) % MINS_PER_HOUR;
			break;
		case HRS:
			n = (tick_count / TICKSPERSEC / SECS_PER_MIN / MINS_PER_HOUR) % HRS_PER_DAY;
			break;
		case DAY:
			n = 27;
			break;
		case MON:
			n = 9;
			break;
	}

	return n;
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

/* This adds two tick counts together.
The t_time field holds up to one second of ticks,
while the t_date field counts minutes */

void addtick(time_t *t1, time_t *t2)
{

    t1->t_time += t2->t_time;
    t1->t_date += t2->t_date;
    if (t1->t_time >= 60*TICKSPERSEC)
    {
        t1->t_time -= 60*TICKSPERSEC;
        ++t1->t_date;
    }
}

void incrtick(time_t *t)
{
    if (++t->t_time == 60*TICKSPERSEC)
    {
        t->t_time = 0;
        ++t->t_date;
    }
}
