#ifndef __FSUTIL_LIB_H__
#define __FSUTIL_LIB_H__

typedef void fsutil_FILE;

int fsutil_printf(const char *format, ...);
void fsutil_panic(char *msg);
void fsutil_rdtime(void *tloc);
void fsutil_img_seek(unsigned short blk);
void fsutil_img_read(unsigned short blk);
void fsutil_img_write(unsigned short blk);
char *fsutil_strcpy(char *dst, const char *src);
char *fsutil_strcat(char *dst, const char *src);
fsutil_FILE *fsutil_fopen(const char *pathname, const char *mode);
int fsutil_fclose(fsutil_FILE *stream);
unsigned long fsutil_fread(void *ptr, unsigned long size, unsigned long nmemb, fsutil_FILE *stream);
unsigned long fsutil_fwrite_stdout(void *ptr, unsigned long size, unsigned long nmemb);

extern int img_fd;

#endif /* __FSUTIL_LIB_H__ */
