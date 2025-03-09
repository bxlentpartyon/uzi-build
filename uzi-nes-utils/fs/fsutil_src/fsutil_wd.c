#include <extern.h>
#include <filesys.h>

#include <lib/string.h>

#include "fsutil_wd.h"

int fsutil_wd_open(int minor)
{
	return 0;
}

char tmp_buf[FSUTIL_BLOCK_SIZE];

int fsutil_wd_read(int minor, int rawflag)
{
	fsutil_img_read(udata.u_buf->bf_blk);
	bcopy(tmp_buf, (char * ) &udata.u_buf->bf_data, FSUTIL_BLOCK_SIZE);
}
