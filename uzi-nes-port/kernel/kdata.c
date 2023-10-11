#include <devio.h>
#include <devtty.h>
#include <unix.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
	{ 0, tty_open, ok, ok, ok, ok /* tty_close, tty_read, tty_write, ok */ },      /* tty */
	{ 0, ok, ok, ok, ok, /* null_write, */ nogood },                      /* /dev/null */
	{ 0, ok, ok, ok, ok, /* mem_read, mem_write, */ nogood }              /* /dev/mem */
};

/* Process/userspace stuff */

struct u_data udata;
ptptr initproc; /* The process table address of the first process. */
struct p_tab ptab[PTABSIZE];

/* Interrupt/timer stuff */

time_t tod;	/* Time of day */
int16 sec;	/* Tick counter for counting off one second */

/* Filesystem stuff */

int16 ROOTDEV;
struct oft of_tab[1];

/* Driver stuff */

struct blkbuf bufpool[NBUFS];
