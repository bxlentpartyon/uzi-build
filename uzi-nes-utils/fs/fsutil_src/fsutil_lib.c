#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fsutil.h"
#include "fsutil_lib.h"
#include "fsutil_time.h"
#include "fsutil_wd.h"

int fsutil_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void kprintf(char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void fsutil_panic(char *msg)
{
	printf("%s\n", msg);
	assert(0);
}

void fsutil_rdtime(void *tloc)
{
	struct fsutil_time_t *local_tloc = (struct fsutil_time_t *)tloc;
	time_t epoch_time;
	struct tm *time_info;

	epoch_time = time(&epoch_time);
	time_info = localtime(&epoch_time);

	local_tloc->t_time = ((time_info->tm_sec >> 1) |
			      (time_info->tm_min << 5) |
			      (time_info->tm_hour << 11));
	local_tloc->t_date = ((time_info->tm_mday) |
			      (time_info->tm_mon << 5) |
			      (time_info->tm_year << 9));

	return;
}

void fsutil_img_seek(unsigned short blk)
{
	off_t seek_off, seek_ret;

	seek_off = blk * FSUTIL_BLOCK_SIZE;
	seek_ret = lseek(img_fd, seek_off, SEEK_SET);
	if (seek_ret != seek_off)
		fsutil_panic("unknown seek error");
	else if (seek_ret < 0)
		fsutil_panic("failed image seek");
}

void fsutil_img_read(unsigned short blk)
{
	ssize_t read_ret;

	fsutil_img_seek(blk);

	read_ret = read(img_fd, tmp_buf, FSUTIL_BLOCK_SIZE);

	if (read_ret == 0)
		fsutil_panic("end of file during read");
	else if (read_ret < 0)
		fsutil_panic("failed image read");
	else if (read_ret != FSUTIL_BLOCK_SIZE)
		fsutil_panic("unknown read error");
}

void fsutil_img_write(unsigned short blk)
{
	ssize_t write_ret;

	fsutil_img_seek(blk);

	write_ret = write(img_fd, tmp_buf, FSUTIL_BLOCK_SIZE);

	if (write_ret < FSUTIL_BLOCK_SIZE)
		fsutil_panic("write failed");
}

char *fsutil_strcpy(char *dst, const char *src)
{
	return strcpy(dst, src);
}

char *fsutil_strcat(char *dst, const char *src)
{
	return strcat(dst, src);
}
