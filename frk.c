#include "frk.h"

void frk(char **_argv)
{
	pid_t pid;
	if ((pid = fork()) < 0) {
		fprintf(stderr, "failed fork: %s\n", strerror(errno));
	} else if (pid == 0) { // Child
		/* printf("CHILD: pid = %d, ppid = %d\n", getpid(), getppid()); */
		if (execvp(_argv[0], _argv) < 0) {
			fprintf(stderr, "Invalid command: (%s) %s\n", _argv[0], strerror(errno));
			exit(1);
		}
		exit(0);
	} else { // Parent
		int status;
		/* printf("PARENT: pid = %d, child = %d\n", getpid(), pid); */
		wait(&status);
		/* printf("PARENT: pid = %d, child status = %d\n", getpid(), WEXITSTATUS(status)); */
	}
}
