#include "nano.h"

#include "frk.h"
#include "parser.h"
#include "vars.h"
#include "pwd.h"
#include "echo.h"
#include "cd.h"
#include "ext.h"


char* pop_argv(char **_argv, int index) {
	char* ret = _argv[index];
	int i = index;
	while (_argv[i] && _argv[i+1]) {
		_argv[i] = _argv[i+1];
		++i;
	}
	_argv[i] = NULL;
	return ret;
}

void parse_IO_redirections(char** _argv)
{
	int i = 0;
	while(_argv[i]) {
		char* c = strchr(_argv[i], '>');
		if (c) {
			if (c > _argv[i]) {
				*c = 0;
			} else if (c == _argv[i] && _argv[i][1] != 0) {
				++_argv[i];
			} else if (c == _argv[i]) {
				pop_argv(_argv, i);
			}
		}
		++i;
	}
}

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	char buf[BIG_BUF_SIZE];
	Cmd cmd;
	for (;;) {
		printf("%s $ ", getcwd(buf, CWD_BUF_SIZE));
		fgets(buf, BIG_BUF_SIZE, stdin);
		buf[strlen(buf) -1] = 0;
		if (strlen(buf) == 0)
			continue;


		char **_argv = make_argv(buf); // Building tokens
		if (!_argv) { // rejecting the user input if a command argument size is large
			continue;
		}
		parse_dollar_sign(_argv);

		parse_IO_redirections(_argv);

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
		/* reset_fds(); */
	} // for(;;)
} // main

