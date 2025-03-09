#include <stdio.h>

#include "fsutil.h"

int ls(char *path)
{
	printf("debug %s\n", path);
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

