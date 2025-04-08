#include <extern.h>

#include "fsutil.h"

int chmod(char *path, char *modes)
{
	int mode;
	int ret;
	int _chmod();

	mode = -1;
	fsutil_printf("scanning %s\n", modes);
	ret = sscanf(modes, "%o", &mode);
	fsutil_printf("DEBUG ret %d\n", ret);
	if (mode == -1) {
		printf("chmod: bad mode\n");
		return (-1);
	}
	if (_chmod(path, mode)) {
		printf("_chmod: error %d\n", udata.u_error);
		return (-1);
	}
}

int chmod_main(int argc, char **argv)
{
	if (argc != 3) {
		fsutil_printf("usage: chmod <mode> <file>\n");
		return;
	}

	return chmod(argv[2], argv[1]);
}
