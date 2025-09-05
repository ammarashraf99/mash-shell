#include "nano.h"
#include <fcntl.h>

#include "frk.h"
#include "parser.h"
#include "vars.h"
#include "pwd.h"
#include "echo.h"
#include "cd.h"
#include "ext.h"
#include <unistd.h>

#define INVALID_FD -1



char g_input_buf[BIG_BUF_SIZE];
int g_saved_out_fd = INVALID_FD;
int g_saved_err_fd = INVALID_FD;







#define REDIRECTION_OPEN_MODE  S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR
						     
void clean_argv(char** _argv) 
{
	int i = 1;
	while(_argv[i]) {
		_argv[i][-1] = ' ';
		++i;
	}
}

void reset_fds() {
	if (g_saved_out_fd >= 0) { // >=0 [valid fd]
		close(STDOUT_FILENO);
		dup2(g_saved_out_fd, STDOUT_FILENO);
		g_saved_out_fd = INVALID_FD;
	}
	if (g_saved_err_fd >= 0) {
		close(STDERR_FILENO);
		dup2(g_saved_err_fd, STDERR_FILENO);
		g_saved_err_fd = INVALID_FD;
	}

}

char* pop_argv(char **_argv, int index)
{
	char* ret = _argv[index];
	int i = index;
	while (_argv[i] && _argv[i+1]) {
		_argv[i] = _argv[i+1];
		++i;
	}
	_argv[i] = NULL;
	return ret;
}

#define TO_THE_LEFT        c > _argv[i] && c[1] == 0
#define TO_THE_RIGHT       c == _argv[i] && c[1] != 0
#define IN_THE_MIDDLE      c == _argv[i] && c[1] == 0
#define SQUISHED           c > _argv[i] && c[1] != 0

#define IS_2   _argv[i][0] == '2' && ( c - _argv[i]) == 1

/* #define MAX_FILES_TO_REDIRECT 16 */


struct ToBePopped {
	int index;
	enum {
		OUT_REDIRECTION, ERR_REDIRECTION,
	}type;
};

#define STACK_MAX_CAP 256


/* void inc_popped_indecies(struct ToBePopped *p_to_be_popped, int to_pop_count) */
/* { */
/* 	for (int i = 0; i < to_pop_count; ++i) { */
/* 		p_to_be_popped->index++; */
/* 	} */
/* } */


void set_out_fd(char* filename)
{
	g_saved_out_fd = dup(STDOUT_FILENO);
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, REDIRECTION_OPEN_MODE);
	dup2(fd, STDOUT_FILENO);
	close(fd);
}

void set_err_fd(char* filename)
{
	g_saved_err_fd = dup(STDERR_FILENO);
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, REDIRECTION_OPEN_MODE);
	dup2(fd, STDERR_FILENO);
	close(fd);
}


char** in_redirection(char** _argv)
{
	struct ToBePopped to_be_popped[STACK_MAX_CAP];
	int to_pop_count = 0;
	int i = 0;
	while(_argv[i]) {
		char* c = strchr(_argv[i], '>');
		if (c) {
			if (TO_THE_LEFT) { // cmd> file
				if (IS_2) { // cmd 2> file
					pop_argv(_argv, i); // poping the 2>
					to_be_popped[to_pop_count].index = i;
					to_be_popped[to_pop_count].type = ERR_REDIRECTION;
					++to_pop_count;
				} else {
					c[0] = 0;
					to_be_popped[to_pop_count].index = i;
					to_be_popped[to_pop_count].type = OUT_REDIRECTION;
					++to_pop_count;
				}
			} else if (TO_THE_RIGHT) { // cmd >file
				++_argv[i];
				to_be_popped[to_pop_count].index = i;
				to_be_popped[to_pop_count].type = OUT_REDIRECTION;
				++to_pop_count;
				// pop file here
				--i;
			} else if (IN_THE_MIDDLE) {  // cmd > file
				pop_argv(_argv, i);
				to_be_popped[to_pop_count].index = i;
				to_be_popped[to_pop_count].type = OUT_REDIRECTION;
				++to_pop_count;
				--i;
			} else if (SQUISHED) {  // cmd>file
				if (IS_2) { // cmd 2>file
					_argv[i] += 2;
					to_be_popped[to_pop_count].index = i;
					to_be_popped[to_pop_count].type = ERR_REDIRECTION;
					++to_pop_count;
				} else {
					*c = ' '; 
					clean_argv(_argv);
					free(_argv);
					_argv = make_argv(g_input_buf);
					to_be_popped[to_pop_count].index = i;
					to_be_popped[to_pop_count].type = OUT_REDIRECTION;
					++to_pop_count;
				}
			}
		}
		++i;
	}
	--to_pop_count;
	int out_first_flag = 1;
	int err_first_flag = 1;
	while (to_pop_count >= 0) {
		if (to_be_popped[to_pop_count].type == OUT_REDIRECTION) {
			if (out_first_flag) {
				set_out_fd(pop_argv(_argv,to_be_popped[to_pop_count].index));
				--to_pop_count;
				out_first_flag = 0;
			} else {
				pop_argv(_argv,to_be_popped[to_pop_count].index);
				--to_pop_count;
			}
		} else {
			if (err_first_flag) {
				set_err_fd(pop_argv(_argv, to_be_popped[to_pop_count].index));
				--to_pop_count;
				err_first_flag = 0;
			} else {
				pop_argv(_argv, to_be_popped[to_pop_count].index);
				--to_pop_count;
			}
		}
	}
	return _argv;
}





char** parse_IO_redirections(char** _argv)
{
	_argv = in_redirection(_argv);
}

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	Cmd cmd;
	for (;;) {
		printf("%s $ ", getcwd(g_input_buf, CWD_BUF_SIZE));
		fgets(g_input_buf, BIG_BUF_SIZE, stdin);
		g_input_buf[strlen(g_input_buf) -1] = 0;
		if (strlen(g_input_buf) == 0)
			continue;


		char **_argv = make_argv(g_input_buf); // Building tokens
		if (!_argv) { // rejecting the user input if a command argument size is large
			continue;
		}
		parse_dollar_sign(_argv);

		_argv = parse_IO_redirections(_argv);

		if (!strcmp(_argv[0], "echo"))
			cmd = ECHO;
		else if (!strcmp(_argv[0], "pwd"))
			cmd = PWD;
		else if (!strcmp(_argv[0], "cd"))
			cmd = CD;
		else if (!strcmp(_argv[0], "exit"))
			cmd = EXT;
		else if (!strcmp(_argv[0], "printvars"))
			cmd = PRINT_ARGS;
		else if (!strcmp(_argv[0], "export"))
			cmd = EXPORT;
		else if (has_assign(_argv, IGN))
			cmd = ASIGN;
		else
			cmd = FRK;


		switch(cmd) {
		case ECHO:
			echo(_argv);
			break;
		case PWD:
			pwd(_argv);
			break;
		case CD:
			cd(_argv);
			break;
		case EXT:
			ext(_argv);
			break;
		case PRINT_ARGS:
			print_vars();
			break;
		case EXPORT:
			export(_argv);
			break;
		case ASIGN:
			store_vars(_argv, IGN);
			break;
		case FRK:
			frk(_argv);
			break;
		case IGN:
			printf("IGN\n");
			break;
		} // switch
		free(_argv);
		_argv = NULL;
		reset_fds();
	} // for(;;)
} // main

