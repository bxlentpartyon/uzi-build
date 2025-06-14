#include <unix.h>

#include "fsutil.h"
#include "fsutil_lib.h"

do_mknod(char *path, char *modes, char *devs)
{
	int mode;
	int dev;

	mode = -1;
	sscanf(modes, "%o", &mode);
	if (mode == -1) {
		fsutil_printf("mknod: bad mode\n");
		return (-1);
	}

	if ((mode & F_MASK) != F_BDEV && (mode & F_MASK) != F_CDEV) {
		fsutil_printf("mknod: mode is not device\n");
		return (-1);
	}

	dev = -1;
	sscanf(devs, "%d", &dev);
	if (dev == -1) {
		fsutil_printf("mknod: bad device\n");
		return (-1);
	}

	if (_mknod(path, mode, dev) != 0) {
		perror("_mknod");
		return (-1);
	}

	return(0);
}


mknod_main(int argc, char **argv)
{
	if (argc != 4) {
		fsutil_printf("usage: mknod path modes devs\n");
		return 1;
	}

	return do_mknod(argv[1], argv[2], argv[3]);
}
