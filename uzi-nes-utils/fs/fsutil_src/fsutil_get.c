#include <extern.h>
#include <unix.h>

#include "fsutil_lib.h"

get(char *arg, int binflag)
{
	fsutil_FILE *fp;
	int d;
	char cbuf[512];
	int nread;

	fp = fsutil_fopen(arg, binflag ? "rb" : "r");
	if (fp == NULL) {
		fsutil_printf("Source file not found\n");
		return (-1);
	}

	d = _creat(arg, 0666);
	if (d < 0) {
		fsutil_printf("Cant open unix file error %d\n", udata.u_error);
		return (-1);
	}

	for (;;) {
		nread = fsutil_fread(cbuf, 1, 512, fp);
		if (nread == 0)
			break;
		if (_write(d, cbuf, nread) != nread) {
			fsutil_printf("_write error %d\n", udata.u_error);
			fsutil_fclose(fp);
			_close(d);
			return (-1);
		}
	}
	fsutil_fclose(fp);
	_close(d);
	return (0);
}

void get_main(int argc, char **argv)
{
	if (argc != 2) {
		fsutil_printf("usage: get <file>\n");
		return;
	}

	if (argv[1]) {
		get(argv[1], 0);
	}
}
