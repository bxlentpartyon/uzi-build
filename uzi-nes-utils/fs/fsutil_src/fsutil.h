#ifndef __FSUTIL_H__
#define __FSUTIL_H__

int ls_main(int argc, char **argv);
int mkdir_main(int argc, char **argv);

int fsutil_printf(const char *format, ...);
void fsutil_panic(char *msg);

#endif /* __FSUTIL_H__ */
