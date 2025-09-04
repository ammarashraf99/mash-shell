#include "pwd.h"

void pwd(char **_argv)
{
	// pwd command should not have a second argument
	if (_argv[1]) {
		printf("Invalid command: %s\n", _argv[1]);
		return;
	}
	char buf[CWD_BUF_SIZE];
	if (!getcwd(buf, CWD_BUF_SIZE)) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
	}
	printf("Current working directory : %s\n", buf);
}
