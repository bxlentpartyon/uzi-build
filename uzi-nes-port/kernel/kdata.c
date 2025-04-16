#include <devio.h>
#include <devppuwd.h>
#include <devtty.h>
#include <unix.h>

struct devsw dev_tab[] =	/* The device driver switch table */
{
	{0, ppuwd_open, ok, ppuwd_read, ppuwd_write, ok},	/* root */
	{4096, ppuwd_open, ok, ppuwd_read, ppuwd_write, ok},	/* swap */
	{0, tty_open, tty_close, tty_read, tty_write, ok},	/* tty */
	{0, ok, ok, ok, ok, /* null_write, */ nogood},	/* /dev/null */
	{0, ok, ok, ok, ok, /* mem_read, mem_write, */ nogood}	/* /dev/mem */
};

/* Process/userspace stuff */

ptptr initproc;			/* The process table address of the first process. */
struct p_tab ptab[PTABSIZE];

/* Interrupt/timer stuff */

time_t tod;			/* Time of day */
time_t ticks;			/* Cumulative tick counter, in minutes and ticks  */
int16 sec;			/* Tick counter for counting off one second */
int16 runticks;			/* Number of ticks current process has been
				   swapped in */

/* Filesystem stuff */

inoptr root;			/* Address of root dir in inode table */
int16 ROOTDEV;
struct cinode i_tab[ITABSIZE];	/* In-core inode table */
struct oft of_tab[OFTSIZE];
struct filesys fs_tab[NDEVS];

/* Driver stuff */

unsigned swapcnt;
char *swapbase;
blkno_t swapblk;

/*
 * Put this in its own section so that we can page-align the buffers.
 * This is not strictly necessary, but some instructions perform better
 * if we don't cross a page bounadry.
 */
#pragma data-name (push, "BLKDATA")
struct blkbuf bufpool[NBUFS] = { 0 };

#pragma data-name (pop)
