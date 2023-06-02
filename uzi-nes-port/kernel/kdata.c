#if 0
#include <extern.h>
#endif
#include <unix.h>
#if 0

#include <devio.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
    { 0, tty_open, tty_close, tty_read, tty_write, ok },      /* tty */
    { 0, ok, ok, ok, null_write, nogood },                      /* /dev/null */
    { 0, ok, ok, mem_read, mem_write, nogood }              /* /dev/mem */
};

/* Process/userspace stuff */

struct u_data udata;
ptptr initproc; /* The process table address of the first process. */
struct p_tab ptab[1];

/* Interrupt/timer stuff */

time_t tod;	/* Time of day */
time_t ticks;	/* Cumulative tick counter, in minutes and ticks  */
int16 inint;	/* flag is set whenever interrupts are being serviced */
int16 runticks;	/* Number of ticks current process has been
		   swapped in */
int16 sec;	/* Tick counter for counting off one second */

/* Filesystem stuff */

inoptr root;   /* Address of root dir in inode table */
int16 ROOTDEV;
struct cinode i_tab[1];    /* In-core inode table */
struct oft of_tab[1];
struct filesys fs_tab[1];

/* Driver stuff */

unsigned swapcnt;
char *swapbase;
blkno_t swapblk;
#endif
struct blkbuf bufpool[NBUFS];
