#include <extern.h>

#include <devio.h>

/* Interrupt/timer stuff */

int16 inint;	/* flag is set whenever interrupts are being serviced */

/* Driver stuff */

unsigned swapcnt;
char *swapbase;
blkno_t swapblk;
struct blkbuf bufpool[NBUFS];
