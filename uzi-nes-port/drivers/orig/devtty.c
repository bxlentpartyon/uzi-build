#include <unix.h>
#include <machdep.h>
#include <devio.h>
#include <process.h>

/* This tty interrupt routine checks to see if the uart receiver actually
caused the interrupt.  If so it adds the character to the tty input
queue, echoing and processing backspace and carriage return.  If the queue 
contains a full line, it wakes up anything waiting on it.  If it is totally
full, it beeps at the user. */

int tty_int(void)
{
    register char c;
    register found;
    char oc;

    found = 0;

again:
    if( (in(0x72)&0x81) != 0x81 )
       return (found);
    c = in(0x73) & 0x7f;

    if (c==0x1a) /* ^Z */
        idump();        /* For debugging */

    if (c == '\003')  /* ^C */
        sendsig(NULL, SIGINT);
    else if (c == '\017')  /* ^O */
        flshflag = !flshflag;
    else if (c == '\023')   /* ^S */
        stopflag = 1;
    else if (c == '\021')  /* ^Q */
    {
        stopflag = 0;
        wakeup(&stopflag);
    }
    else if (c == '\b')
    {
        if (uninsq(&ttyinq,&oc))
        {
            if (oc == '\n')
                insq(&ttyinq,oc);   /* Don't erase past newline */
            else
            {
                _putc('\b');
                _putc(' ');
                _putc('\b');
            }
        }
    }
    else
    {
        if (c == '\r' || c == '\n')
        {
            c = '\n';
            _putc('\r');
        }

        if (insq(&ttyinq,c))
            _putc(c);
        else
            _putc('\007');              /* Beep if no more room */
    }

    if (c == '\n' || c == '\004')   /* ^D */
        wakeup(&ttyinq);

    found = 1;
    goto again;         /* Loop until the uart has no data ready */
}
