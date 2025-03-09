#include <devio.h>
#include <unix.h>

struct devsw dev_tab[] = /* The device driver switch table */
{
	{ 0, nogood, nogood, nogood, nogood, nogood },
};

/* Process/userspace stuff */

struct u_data udata;

/* Filesystem stuff */

inoptr root;   /* Address of root dir in inode table */
int16 ROOTDEV;

/* Driver stuff */

struct blkbuf bufpool[NBUFS] = { 0 };
