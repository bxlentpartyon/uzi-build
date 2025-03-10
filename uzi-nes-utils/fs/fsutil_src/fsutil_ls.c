#include <unix.h>

#include "fsutil.h"

static char *s;

void dout(int n)
{
	n %= 100;
	*s++ = n / 10 + '0';
	*s++ = n % 10 + '0';
}

char *ctime(time_t *t)
{
	static char str[24];

	s = str;
	*s = '\0';

	dout((t->t_time & 0xf800) >> 11);
	*s++ = ':';
	dout((t->t_time & 0x07e0) >> 5);
	*s++ = ':';
	dout(t->t_time & 0x001f * 2);
	*s++ = ' ';
	dout((t->t_date & 0x01e0) >> 5);
	*s++ = '/';
	dout(t->t_date & 0x001f);
	*s++ = '/';
	dout((t->t_date & 0xfe00) >> 9);
	*s = '\0';

	return (str);
}

int ls(char *path)
{
	struct direct buf;
	struct stat statbuf;
	char dname[128];
	int d;

	d = _open(path, 0);
	if (d < 0) {
		fsutil_printf("ls: can't open %s\n", path);
		return;
	}

	while (_read(d, (char *)&buf, 16) == 16) {
		if (buf.d_name[0] == '\0')
			continue;

		if (path[0] != '.' || path[1]) {
			fsutil_strcpy(dname, path);
			fsutil_strcat(dname, "/");
		} else
			dname[0] = '\0';
		fsutil_strcat(dname, buf.d_name);

		if (_stat(dname, &statbuf) != 0) {
			fsutil_printf("ls: can't stat %s\n", dname);
			continue;
		}

		if ((statbuf.st_mode & F_MASK) == F_DIR)
			fsutil_strcat(dname, "/");

		fsutil_printf("%-6d %-15s",
			      (statbuf.st_mode & F_CDEV) ?
			      statbuf.st_rdev :
			      512 * statbuf.st_size.o_blkno +
			      statbuf.st_size.o_offset, dname);

		fsutil_printf("  0%-6o %-2d %-5d %s\n", statbuf.st_mode,
			      statbuf.st_nlink, statbuf.st_ino,
			      ctime(&statbuf.st_mtime));
	}
	_close(d);
}

int ls_main(int argc, char **argv)
{
	int i;

	if (argc == 1) {
		ls(".");
	} else {
		for (i = 0; i < argc; i++)
			ls(argv[i]);
	}

	return 0;
}
