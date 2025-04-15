/**************************************************
UZI (Unix Z80 Implementation) Kernel:  devtty.c
***************************************************/

#include <devio.h>
#include <extern.h>
#include <machdep.h>
#include <process.h>
#include <ppu.h>
#include <unix.h>

#pragma code-name (push, "DEVTTY_CODE")

#define TTYSIZ 132

char ttyinbuf[TTYSIZ];
void _putc(char c);

struct s_queue ttyinq = {
	ttyinbuf,
	ttyinbuf,
	ttyinbuf,
	TTYSIZ,
	0,
	TTYSIZ / 2
};

int stopflag;			/* Flag for ^S/^Q */
int flshflag;			/* Flag for ^O */

void tty_init(void)
{
	insq(&ttyinq, '0');
}

#pragma code-name (pop)

/*
 * This code can be called indirectly, so it needs to go in the regular CODE
 * section
 */

int tty_read(int16 minor, int16 rawflag)
{
	int nread;

	nread = 0;
	while (nread < udata.u_count) {
		for (;;) {
			di();
			if (remq(&ttyinq, udata.u_base))
				break;
			psleep((void *)&ttyinq);
			if (udata.u_cursig || udata.u_ptab->p_pending) {	/* messy */
				udata.u_error = EINTR;
				return (-1);
			}
		}
		ei();

		if (nread++ == 0 && *udata.u_base == '\004')	/* ^D */
			return (0);

		if (*udata.u_base == '\n')
			break;
		++udata.u_base;
	}
	return (nread);
}

int tty_write(int16 minor, int16 rawflag)
{
	int towrite;

	towrite = udata.u_count;

	while (udata.u_count-- != 0) {
		for (;;) {	/* Wait on the ^S/^Q flag */
			di();
			ifnot(stopflag)
			    break;
			psleep(&stopflag);
			if (udata.u_cursig || udata.u_ptab->p_pending) {	/* messy */
				udata.u_error = EINTR;
				return (-1);
			}
		}
		ei();

		ifnot(flshflag) {
			if (*udata.u_base == '\n')
				_putc('\r');
			_putc(*udata.u_base);
		}
		++udata.u_base;
	}
	return (towrite);
}

int tty_open(int minor)
{
	if (minor == 0)
		return minor;
	return (0);
}

int tty_close(int minor)
{
	return (0);
}

int tty_ioctl(int minor)
{
	return (-1);
}
