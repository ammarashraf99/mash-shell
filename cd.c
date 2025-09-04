#include "cd.h"

void cd(char ** _argv) {
	// cd command should not have a third argument after the targed directory
	if (_argv[2]) {
		printf("Invalid command: %s\n", _argv[2]);
		return;
	}
	if (chdir(_argv[1]) < 0)
		fprintf(stderr, "error: %s\n", strerror(errno));
}
