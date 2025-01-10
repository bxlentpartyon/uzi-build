#include <extern.h>

#include <devio.h>

/* Interrupt/timer stuff */

int16 inint;	/* flag is set whenever interrupts are being serviced */

/* Filesystem stuff */

inoptr root;   /* Address of root dir in inode table */
struct cinode i_tab[1];    /* In-core inode table */

/* Driver stuff */

unsigned swapcnt;
char *swapbase;
blkno_t swapblk;
struct blkbuf bufpool[NBUFS];
