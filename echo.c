#include "echo.h"

void echo(char **_argv)
{
	int i = 1; // bypassing "echo" command
	while (_argv[i]) {
		printf("%s ", _argv[i++]);
	}
	printf("\n");
}
