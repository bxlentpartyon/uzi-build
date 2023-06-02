/**************************************************
UZI (Unix Z80 Implementation) Kernel:  machdep.c
***************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <unix.h>
#include <extern.h>

#include <devio.h>
#include <devtty.h>
#include <machdep.h>
#include <process.h>

/* This is called at the very beginning to initialize everything. */
/* It is the equivalent of main() */

void main()
{
    di();
    stkreset();
    /* Initialize the interrupt vector */
    initvec();
    inint = 0;
    udata.u_insys = 1;
    /* Turn off clock */
    out(0,0xf1);
    ei();

    init2();   /* in process.c */
}


/* This checks to see if a user-suppled address is legitimate */
int valadr(char *base, uint16 size)
{
    if (base < PROGBASE || base+size >= (char *)&udata)
    {
        udata.u_error = EFAULT;
        return(0);
    }
    return(1);
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


void stkreset(void)
{
/*
#asm 8080
        POP     H
        LXI     SP,udata?-2
        PCHL
#endasm
*/
}


void tempstack(void)
{
/*
#asm 8080
        POP     H
        LXI     SP,100H
        PCHL
#endasm
*/
}



void initvec(void)
{
/*
#asm 8080
        LXI     H,vector?
        INX     H
        MOV     A,L
        ANI     0FEH
        MOV     L,A     ;set hl to first even address in vector[].
        MOV     A,H
.Z80
        LD      I,A     ;SET INTERRUPT REGISTER TO UPPER 8 BITS
.8080
        MOV     A,L
        OUT     076H    ;set external vector register with low order byte
        LXI     D,service?
        MOV     M,E
        INX     H
        MOV     M,D     ;STORE ADDRESS OF SERVICE ROUTINE IN vector[].
        RET
#endasm
*/
}

extern int unix();


void doexec(void *new_stack)
{
/*
#asm 8080
        POP     H
        POP     H       ;get argument
        SPHL            ;set stack pointer to it
        MVI     A,0C3H  ;jump inst
        STA     0030H   ;dest of RST6 instruction.
        LXI     H,unix? ;entry address
        SHLD    0031H
        XRA     A
        STA     udata? + ?OSYS
        JMP     0100H
#endasm
*/
}


static int cursig;
static int (*curvec)();

/* This interrupt device routine calls the service routine of each device
that could have interrupted. */

service()
{
/*
#asm 8080
        PUSH    PSW
        PUSH    B
        PUSH    D
        PUSH    H
.Z80
        PUSH    IX
        PUSH    IY
.8080
#endasm
*/

    inint = 1;

    if (tty_int())
        goto found;
    if (clk_int())
        goto found;
/*  if (  ) ...   */

    warning("Spurious interrupt");

found:
    inint = 0;

    /* Deal with a pending caught signal, if any */
    if (!udata.u_insys)
        calltrap();
    ;

/*
#asm 8080
.Z80
        POP     IY
        POP     IX
.8080
        POP     H
        POP     D
        POP     B
        POP     PSW
        EI
        RET
#endasm
*/
}



void calltrap(void)
{
    /* Deal with a pending caught signal, if any. */
        /* udata.u_insys should be false, and interrupts enabled.
        remember, the user may never return from the trap routine */

    if (udata.u_cursig)
    {
        cursig = udata.u_cursig;
        curvec = udata.u_sigvec[cursig];
        udata.u_cursig = 0;
        udata.u_sigvec[cursig] = SIG_DFL;   /* Reset to default */
        ei();
        (*curvec)(cursig);
        di();
    } 
}



/* Port addresses of clock chip registers. */

#define SECS 0xe2
#define MINS 0xe3
#define HRS 0xe4
#define DAY 0xe6
#define MON 0xe7
#define YEAR 86

sttime()
{
    panic("Calling sttime");
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


/* Read BCD clock register, convert to binary. */
uint16 tread(uint16 port)
{
    int n;

    n = in(port);
    return ( 10*((n>>4)&0x0f) + (n&0x0f) );
}


/* Disable interrupts */
void di(void)
{
/*
#asm 8080
        DI      ;disable interrupts
#endasm
*/
}

/* Enable interrupts if we are not in service routine */
void ei(void)
{
    if (inint)
        return;
    ;   /* Empty statement necessary to fool compiler */

/*
#asm 8080
        EI      ;disable interrupts
#endasm
*/
}



/* This shifts an unsigned int right 8 places. */

shift8()
{
/*
#asm 8080
        POP     D       ;ret addr
        POP     H
        MOV     L,H
        MVI     H,0
        MOV     A,L
        ANA     A       ;set Z flag on result
        PUSH    H
        PUSH    D       ;restore stack
#endasm
*/
}


/* This prints an error message and dies. */

void panic(char *s)
{
    di();
    inint = 1;
    kprintf("PANIC: %s\n",s);
    idump();
    abort();
}


void warning(char *s)
{
    kprintf("WARNING: %s\n",s);
}


puts(s)
char *s;
{
    while (*s)
        kputchar(*(s++));
}

void kputchar(int c)
{
    if (c == '\n')
        _putc('\r');
    _putc(c);
    if (c == '\t')
        puts("\177\177\177\177\177\177\177\177\177\177");
}



void idump(void)
{
    inoptr ip;
    ptptr pp;
    extern struct cinode i_tab[];

    kprintf(
        "\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY err %d root %d\n",
            udata.u_error, root - i_tab);

    for (ip=i_tab; ip < i_tab+ITABSIZE; ++ip)
    {
        kprintf("%d\t%d\t%d\t%u\t0%o\t0%o\t%d\t%d\t%d\t%d\n",
               ip-i_tab, ip->c_magic, ip->c_dev, ip->c_num,
               ip->c_node.i_mode_lo, ip->c_node.i_mode_hi,
	       ip->c_node.i_nlink, ip->c_node.i_addr[0],
               ip->c_refs,ip->c_dirty);
/*****
        ifnot (ip->c_magic)     
            break;
******/
    }

    kprintf("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\n");
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
        kprintf("%d\t%d\t0x%x\t%d\t%d\t%d\t0x%x\t0x%x\n",
               pp-ptab, pp->p_status, pp->p_wait,  pp->p_pid,
               pp->p_pptr-ptab, pp->p_alarm, pp->p_pending,
                pp->p_ignored);
        ifnot(pp->p_pptr)
            break;
    }   
    
    bufdump();

    kprintf("\ninsys %d ptab %d call %d cwd %d sp 0x%x\n",
        udata.u_insys,udata.u_ptab-ptab, udata.u_callno, udata.u_cwd-i_tab,
       udata.u_sp);
}



/* Short version of printf to save space */
/* TODO: fix the vararg handling here */
void kprintf(char *fmt, ...)
        {
	va_list ap;
        register char **arg;
        register c, base;
        char s[7]; //, *itob();


	va_start(ap, fmt);

	/* wrong - just comment it out for now.
        arg = (char **)&nargs + nargs;
        fmt = *arg;
	*/
        while (c = *fmt++) {
                if (c != '%') {
                        kputchar(c);
                        continue;
                        }
                switch (c = *fmt++) {
                case 'c':
                        kputchar(*--arg);
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
                prt:
                        //puts(itob(*--arg, s, base));
                        continue;
                case 's':
                        puts(*--arg);
                        continue;
                default:
                        kputchar(c);
                        continue;
                        }
                }

	va_end(ap);
        }


void out(char c, unsigned int *addr)
{
	return;
}

unsigned int in(unsigned int *addr)
{
	return 0;
}
