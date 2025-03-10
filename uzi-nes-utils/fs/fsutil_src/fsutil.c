#include <fcntl.h>
#include <malloc.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fsutil.h"
#include "fsutil_filesys.h"
#include "fsutil_lib.h"

int img_fd;
static bool shell_waiting = false;

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
	while ((c = getchar()) != '\n') ;
}

enum scan_ret_state {
	BUFFER_OK,
	BUFFER_TOO_LONG,
	BUFFER_EOF,
};

void clear_shell_buf(void)
{
	bzero(shell_buf, SHELL_BUF_LEN);
}

char *scan_line(enum scan_ret_state *state)
{
	char c;
	unsigned int shell_buf_pos = 0;
	bool newline = false;

	*state = BUFFER_OK;
	clear_shell_buf();

	do {
		shell_waiting = true;
		c = getchar();
		shell_waiting = false;

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
#define SHELL_PROMPT "shell> "

void shell(void)
{
	char *input_str, *token;
	unsigned long input_len;
	int shell_argc, argv_buf_size;
	char **shell_argv;
	enum scan_ret_state state;
	int i;

	bool keep_going = true;

	while (keep_going) {
		printf(SHELL_PROMPT);
		input_str = scan_line(&state);

		if (!input_str) {
			switch (state) {
			case BUFFER_TOO_LONG:
				printf("Maximum input length exceeded\n");
				continue;
			case BUFFER_EOF:
				printf("\n");
				keep_going = false;
				break;
			default:
				printf("Bad return state from scan_line: %u\n",
				       state);
				break;
			}
		} else {
			argv_buf_size = 8;
			shell_argc = 0;
			shell_argv = calloc(argv_buf_size, sizeof(char *));

			token = strtok(input_str, SHELL_CMD_TOKEN);
			while (token != NULL) {
				printf("token %s\n", token);
				if (shell_argc == argv_buf_size) {
					argv_buf_size *= 2;
					shell_argv =
					    reallocarray(shell_argv,
							 argv_buf_size,
							 sizeof(char *));
				}

				shell_argv[shell_argc++] = token;
				token = strtok(NULL, SHELL_CMD_TOKEN);
			}

			if (!shell_argv[0])
				continue;

			for (i = 0; i < shell_argc; i++)
				printf("argv[%d]: %s\n", i, shell_argv[i]);

			if (strcmp(shell_argv[0], "ls") == 0)
				ls_main(shell_argc, shell_argv);
			else if (strcmp(shell_argv[0], "mkdir") == 0)
				mkdir_main(shell_argc, shell_argv);
			else if (strcmp(shell_argv[0], "get") == 0)
				get_main(shell_argc, shell_argv);
		}
	}
}

void sigint_handler(int sig)
{
	signal(sig, SIG_IGN);

	/*
	 * Ctrl-C while waiting for the shell causes it to reset to a new line.
	 * Outside of that, we just ignore it.
	 */
	if (shell_waiting) {
		clear_shell_buf();
		printf("\n" SHELL_PROMPT);
		fflush(stdout);
	}

	signal(sig, sigint_handler);
}

int main(int argc, char **argv)
{
	int opt;
	char *fs_image;

	signal(SIGINT, sigint_handler);

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
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

	img_fd = open(fs_image, O_RDWR);
	if (img_fd < 0) {
		error("Can't open file %s\n", fs_image);
		exit(EXIT_FAILURE);
	}

	fs_init(0);

	shell();

	fs_exit();

	close(img_fd);

	return EXIT_SUCCESS;
}
