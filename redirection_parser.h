#ifndef REDIRECTION_PARSER_H
#define REDIRECTION_PARSER_H


#include "mash.h"



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
char** parse_IO_redirections(char** _argv);


#endif
