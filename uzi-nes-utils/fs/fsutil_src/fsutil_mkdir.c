#include <extern.h>

#include "fsutil.h"

int mkdir(char *path)
{

	char dot[100];

	if (_mknod(path, 040000 | 0777, 0) != 0) {
		fsutil_printf("mkdir: mknod error %d\n", udata.u_error);
		return (-1);
	}

	fsutil_strcpy(dot, path);
	fsutil_strcat(dot, "/.");
	if (_link(path, dot) != 0) {
		fsutil_printf("mkdir: link dot error %d\n", udata.u_error);
		return (-1);
	}

	fsutil_strcpy(dot, path);
	fsutil_strcat(dot, "/..");
	if (_link(".", dot) != 0) {
		fsutil_printf("mkdir: link dotdot error %d\n", udata.u_error);

		return (-1);
	}

	return (0);
}

int mkdir_main(int argc, char **argv)
{
	if (argv[1])
		return mkdir(argv[1]);
}
