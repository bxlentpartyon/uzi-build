#ifndef __FSUTIL_LIB_H__
#define __FSUTIL_LIB_H__

int fsutil_printf(const char *format, ...);
void fsutil_panic(char *msg);
void fsutil_rdtime(void *tloc);
void fsutil_img_seek(unsigned short blk);
void fsutil_img_read(unsigned short blk);
void fsutil_img_write(unsigned short blk);
char *fsutil_strcpy(char *dst, const char *src);
char *fsutil_strcat(char *dst, const char *src);

extern int img_fd;

#endif /* __FSUTIL_LIB_H__ */
