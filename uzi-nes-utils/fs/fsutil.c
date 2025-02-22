#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linux_fs.h"

struct filesys *filsys;
char sb_buf[DUMMY_BLOCKSIZE] =  { 0 };

#define error(...)	fprintf(stderr, "error: " __VA_ARGS__);

void usage(void)
{
	fprintf(stderr, "usage: fsutil [-h] fs_image\n");
}

#define SHELL_BUF_MAX		255
#define SHELL_BUF_NULL		1
#define SHELL_BUF_LEN		(SHELL_BUF_MAX + 	\
				 SHELL_BUF_NULL)

char shell_buf[SHELL_BUF_LEN];

void clear_to_newline(void)
{
	char c;
	while ((c = getchar()) != '\n');
}

enum scan_ret_state {
	BUFFER_OK,
	BUFFER_TOO_LONG,
	BUFFER_EOF,
};

char *scan_line(enum scan_ret_state *state)
{
	char c;
	unsigned int shell_buf_pos = 0;
	bool newline = false;

	*state = BUFFER_OK;
	bzero(shell_buf, SHELL_BUF_LEN);

	do {
		c = getchar();

		if (c != '\n' && c != EOF) {
			shell_buf[shell_buf_pos++] = c;

			if (shell_buf_pos > SHELL_BUF_MAX) {
				clear_to_newline();
				*state = BUFFER_TOO_LONG;
				return NULL;
			}
		} else if (c == EOF) {
			*state = BUFFER_EOF;
			return NULL;
		} else
			newline = true;
	} while (!newline);

	return shell_buf;
}

void shell(void)
{
	bool keep_going = true;
	char *input_str;
	unsigned long input_len;
	enum scan_ret_state state;

	while (keep_going) {
		printf("shell> ");
		input_str = scan_line(&state);

		if (!input_str) {
			switch(state) {
				case BUFFER_TOO_LONG:
					printf("Maximum input length exceeded\n");
					continue;
				case BUFFER_EOF:
					keep_going = false;
					break;
				default:
					printf("Bad return state from scan_line: %u\n", state);
					break;
			}
		} else {
			input_len = strlen(input_str);
			printf("input len %d\n", input_len);
		}
	}
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

	shell();

	close(fd);

	return EXIT_SUCCESS;
}
