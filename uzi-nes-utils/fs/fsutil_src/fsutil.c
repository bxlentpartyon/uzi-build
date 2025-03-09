#include <fcntl.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <unix.h>

#include "fsutil.h"
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

#define SHELL_CMD_TOKEN " "

void shell(void)
{
	char *input_str, *token;
	unsigned long input_len;
	int shell_argc, argv_buf_size;
	char **shell_argv;
	enum scan_ret_state state;

	bool keep_going = true;

	while (keep_going) {
		printf("shell> ");
		input_str = scan_line(&state);

		if (!input_str) {
			switch(state) {
				case BUFFER_TOO_LONG:
					printf("Maximum input length exceeded\n");
					continue;
				case BUFFER_EOF:
					printf("\n");
					keep_going = false;
					break;
				default:
					printf("Bad return state from scan_line: %u\n", state);
					break;
			}
		} else {
			argv_buf_size = 8;
			shell_argc = 0;
			shell_argv = calloc(argv_buf_size, sizeof(char *));

			token = strtok(input_str, SHELL_CMD_TOKEN);
			while (token != NULL){
				printf("token %s\n", token);
				if (shell_argc == argv_buf_size) {
					argv_buf_size *= 2;
					shell_argv = reallocarray(shell_argv, argv_buf_size, sizeof(char *));
				}

				shell_argv[shell_argc++] = token;
				token = strtok(NULL, SHELL_CMD_TOKEN);
			}

			if (strcmp(shell_argv[0], "ls"))
				ls_main(shell_argc, shell_argv);
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
