#include <unix.h>

/* Process/userspace stuff */

struct u_data udata;
ptptr initproc; /* The process table address of the first process. */
struct p_tab ptab[PTABSIZE];

/* Driver stuff */

struct blkbuf bufpool[NBUFS];
