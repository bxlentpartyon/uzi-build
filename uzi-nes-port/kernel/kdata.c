#include <unix.h>

/* Process/userspace stuff */

struct u_data udata;
ptptr initproc; /* The process table address of the first process. */
struct p_tab ptab[PTABSIZE];

/* Interrupt/timer stuff */

time_t tod;	/* Time of day */
int16 sec;	/* Tick counter for counting off one second */

/* Filesystem stuff */

struct oft of_tab[1];

/* Driver stuff */

struct blkbuf bufpool[NBUFS];
