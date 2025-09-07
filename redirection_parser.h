#ifndef REDIRECTION_PARSER_H
#define REDIRECTION_PARSER_H

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "mash.h"
#include "parser.h"

#define INVALID_FD -1
#define STACK_MAX_CAP 256

#define REDIRECTION_OPEN_MODE  S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR
#define TO_THE_LEFT        c > _argv[i] && c[1] == 0
#define TO_THE_RIGHT       c == _argv[i] && c[1] != 0
#define IN_THE_MIDDLE      c == _argv[i] && c[1] == 0
#define SQUISHED           c > _argv[i] && c[1] != 0
#define IS_2   _argv[i][0] == '2' && ( c - _argv[i]) == 1


extern char g_input_buf[BIG_BUF_SIZE];
extern int g_saved_out_fd;
extern int g_saved_err_fd;

						     
struct ToBePopped {
	int index;
	enum {
		OUT_REDIRECTION, ERR_REDIRECTION, IN_REDIRECTION,
	}type;
};

void reset_fds();
void clean_argv(char** _argv);
char* pop_argv(char **_argv, int index);
char** parse_IO_redirections(char** _argv);

#endif
