/**************************************************
UZI (Unix Z80 Implementation) Kernel:  extern.h
***************************************************/

#ifndef __EXTERN_H__
#define __EXTERN_H__

#include <unix.h>

/* These are the global data structures */

#ifdef MAIN
#define extern  
#endif


extern struct u_data udata; /* MUST BE FIRST */
extern struct p_tab ptab[1];

extern inoptr root;   /* Address of root dir in inode table */
extern int16 ROOTDEV;  /* Device number of root filesystem. */

extern struct cinode i_tab[1];    /* In-core inode table */
extern struct oft of_tab[1]; /* Open File Table */

extern struct filesys fs_tab[1];  /* Table entry for each
                                        device with a filesystem. */
extern struct blkbuf bufpool[1];

extern ptptr initproc; /* The process table address of the first process. */
extern int16 inint;   /* flag is set whenever interrupts are being serviced */

extern int16 sec;   /* Tick counter for counting off one second */
extern int16 runticks;  /* Number of ticks current process has been
                         swapped in */

extern time_t tod;      /* Time of day */
extern time_t ticks;    /* Cumulative tick counter, in minutes and ticks  */

extern char *swapbase;          /* Used by device driver for swapping */
extern unsigned swapcnt;
extern blkno_t swapblk;

extern char vector[3];  /* Place for interrupt vector */

#ifdef MAIN
#undef extern
#endif

#endif /* __EXTERN_H__ */
