#include <extern.h>

#include <devio.h>

/* Interrupt/timer stuff */

time_t ticks;	/* Cumulative tick counter, in minutes and ticks  */
int16 inint;	/* flag is set whenever interrupts are being serviced */
int16 runticks;	/* Number of ticks current process has been
		   swapped in */

/* Filesystem stuff */

inoptr root;   /* Address of root dir in inode table */
struct cinode i_tab[1];    /* In-core inode table */
struct filesys fs_tab[1];

/* Driver stuff */

unsigned swapcnt;
char *swapbase;
blkno_t swapblk;
struct blkbuf bufpool[NBUFS];
