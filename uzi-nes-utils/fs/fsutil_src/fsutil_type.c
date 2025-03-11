#include <extern.h>
#include <unix.h>

#include "fsutil_lib.h"

int type(char *arg)
{
	int d;
	char cbuf[512];
	int nread;

	d = _open(arg, 0);
	if (d < 0) {
		printf("Cant open unix file error %d\n", udata.u_error);
		return (-1);
	}

	for (;;) {
		if ((nread = _read(d, cbuf, 512)) == 0)
			break;

		fsutil_fwrite_stdout(cbuf, 1, nread);
	}
	_close(d);
	return (0);
}

int type_main(int argc, char **argv)
{
	if (argc != 2) {
		fsutil_printf("usage: type <file>\n");
		return -1;
	}

	if (*argv[1])
		type(argv[1]);
}
