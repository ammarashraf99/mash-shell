#include "nano.h"

#include "frk.h"
#include "parser.h"
#include "vars.h"
#include "pwd.h"
#include "echo.h"
#include "cd.h"
#include "ext.h"

char g_input_buf[BIG_BUF_SIZE];


void clean_argv(char** _argv) 
{
	int i = 1;
	while(_argv[i]) {
		_argv[i][-1] = ' ';
		++i;
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

#define MAX_FILES_TO_REDIRECT 16

char** in_redirection(char** _argv)
{
	char* out_file_name = NULL;
	char* error_file_name = NULL;
	int i = 0;
	int out_popped_list[MAX_FILES_TO_REDIRECT];
	int out_list_counter=0;
	int error_popped_list[MAX_FILES_TO_REDIRECT];
	int error_list_counter=0;
	while(_argv[i]) {
		char* c = strchr(_argv[i], '>');
		if (c) {
			if (TO_THE_LEFT) { // cmd> file
				if (IS_2) { // cmd 2> file
					pop_argv(_argv, i); // poping the 2>
					error_popped_list[error_list_counter++] = i+1;
				} else {
					c[0] = 0;
				}
			} else if (TO_THE_RIGHT) { // cmd >file
				++_argv[i];
				out_popped_list[out_list_counter++] = i;
				// pop file here
				--i;
			} else if (IN_THE_MIDDLE) {  // cmd > file
				pop_argv(_argv, i);
				out_popped_list[out_list_counter++] = i;
				--i;
			} else if (SQUISHED) {  // cmd>file
				if (IS_2) { // cmd 2>file
					_argv[i] += 2;
					error_popped_list[error_list_counter++] = i+1;

				} else {
					*c = ' '; 
					clean_argv(_argv);
					free(_argv);
					_argv = make_argv(g_input_buf);
				}
			}
		}
		++i;
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
		/* reset_fds(); */
	} // for(;;)
} // main

