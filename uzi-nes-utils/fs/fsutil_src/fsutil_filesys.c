#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <unix.h>

#include "fsutil.h"
#include "fsutil_machdep.h"

void fs_init(int bootdev)
{
	register char *j;

	udata.u_euid = 0;
	udata.u_insys = 1;

	bufinit();

	/* User's file table */
	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j)
		*j = -1;

	ROOTDEV = bootdev;

	/* Mount the root device */
	if (fmount(ROOTDEV, NULLINODE))
		panic("no filesys");

	ifnot(root = i_open(ROOTDEV, ROOTINODE))
	    panic("no root");
	i_ref(udata.u_cwd = root);
	rdtime(&udata.u_time);
}

void fs_exit(void)
{
	register int16 j;

	for (j=0; j < UFTSIZE; ++j) {
		ifnot (udata.u_files[j] & 0x80)  /* Portable equivalent of == -1 */
			doclose(j);
	}

	_sync();  /* Not necessary, but a good idea. */
}
