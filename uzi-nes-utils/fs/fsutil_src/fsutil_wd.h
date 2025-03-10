#ifndef __FSUTIL_WD_H__
#define __FSUTIL_WD_H__

int fsutil_wd_open(int minor);
int fsutil_wd_read(int minor, int rawflag);
int fsutil_wd_write(int minor, int rawflag);
void fsutil_img_read(unsigned short blk);
void fsutil_img_write(unsigned short blk);

#define FSUTIL_BLOCK_SIZE 512
extern char tmp_buf[FSUTIL_BLOCK_SIZE];

#endif /* __FSUTIL_WD_H__ */
