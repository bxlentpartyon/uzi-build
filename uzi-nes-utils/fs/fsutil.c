#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "linux_fs.h"

struct filesys *filsys;
char sb_buf[DUMMY_BLOCKSIZE] =  { 0 };

#define error(...)	fprintf(stderr, "error: " __VA_ARGS__);

void usage(void)
{
	fprintf(stderr, "usage: fsutil [-h] fs_image\n");
}

int main(int argc, char **argv)
{
	int opt, fd;
	char *fs_image;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch(opt) {
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc) {
		error("Expected fs_image argument after options\n");
		usage();
		exit(EXIT_FAILURE);
	}

	fs_image = argv[optind];
	printf("Mounting UNIX filesystem image: %s\n", fs_image);

	fd = open(fs_image, O_RDWR);
	if (fd < 0) {
		error("Can't open file %s\n", fs_image);
		exit(EXIT_FAILURE);
	}

	close(fd);

	return EXIT_SUCCESS;
}
