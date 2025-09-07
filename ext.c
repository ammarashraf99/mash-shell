#include "ext.h"
#include "vars.h"

void ext(char **_argv)
{
	// exit command should not have a second argument
	if (_argv[1]) {
		printf("Invalid command: %s\n", _argv[1]);
		return;
	}
	free_vars();
	printf("Good Bye\n");
	exit(0);
}
