#include "nano.h"

#include "frk.h"
#include "parser.h"
#include "vars.h"
#include "pwd.h"
#include "echo.h"
#include "cd.h"
#include "ext.h"




int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	char buf[BIG_BUF_SIZE];
	Cmd cmd;
	for (;;) {
		printf("%s$ ", getcwd(buf, CWD_BUF_SIZE));
		fgets(buf, BIG_BUF_SIZE, stdin);
		buf[strlen(buf) -1] = 0;
		if (strlen(buf) == 0)
			continue;
		char **_argv = make_argv(buf); // Building tokens
		if (!_argv) { // rejecting the user input if a command argument size is large
			continue;
		}

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

		parse_dollar_sign(_argv);

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
	} // for(;;)
} // main

