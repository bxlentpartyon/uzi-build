#include <devio.h>
#include <unix.h>

#include "fsutil_wd.h"

struct devsw dev_tab[] =	/* The device driver switch table */
{
	{0, fsutil_wd_open, nogood, fsutil_wd_read, fsutil_wd_write, nogood},
};

/* Process/userspace stuff */

struct u_data udata = { 0 };

/* Filesystem stuff */

#define FSUTIL_NDEVS 1

inoptr root;			/* Address of root dir in inode table */
int16 ROOTDEV;
struct cinode i_tab[ITABSIZE] = { 0 };	/* In-core inode table */
struct oft of_tab[OFTSIZE] = { 0 };
struct filesys fs_tab[FSUTIL_NDEVS] = { 0 };

/* Driver stuff */

struct blkbuf bufpool[NBUFS] = { 0 };
