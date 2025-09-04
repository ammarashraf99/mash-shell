#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "nano.h"


char **varList = NULL;
size_t g_count = 0;
size_t capacity = 0;



int main(int argc, char *argv[]) {
	char buf[BUFSIZ];
	Cmd cmd;
	for (;;) {
		printf("Simple shell $ ");
		fgets(buf, BIG_BUF_SIZE, stdin); // Getting input from user
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
		else if (!strcmp(_argv[0], "source"))
			cmd = SOURCE;
		else if (has_assign(_argv))
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
		case SOURCE:
			// source();
			break;
		case ASIGN:
			store_vars(_argv[0]);
			break;
		case FRK:
			frk(_argv);
			break;
		} // switch
		free(_argv);
	} // for(;;)
} // main

void frk(char **_argv)
{
	pid_t pid;
	if ((pid = fork()) < 0) {
		fprintf(stderr, "failed fork: %s\n", strerror(errno));
	} else if (pid == 0) { // Child
		printf("CHILD: pid = %d, ppid = %d\n", getpid(), getppid());
		if (execvp(_argv[0], _argv) < 0) {
			fprintf(stderr, "failed exec: (%s) %s\n", _argv[0], strerror(errno));
			exit(1);
		}
		exit(0);
	} else { // Parent
		int status;
		printf("PARENT: pid = %d, child = %d\n", getpid(), pid);
		wait(&status);
		printf("PARENT: pid = %d, child status = %d\n", getpid(), WEXITSTATUS(status));
	}
}

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

void echo(char **_argv)
{
	int i = 1;
	while (_argv[i]) {
		printf("%s ", _argv[i++]);
	}
	printf("\n");
}

void cd(char ** _argv) {
	// cd command should not have a third argument after the targed directory
	if (_argv[2]) {
		printf("Invalid command: %s\n", _argv[2]);
		return;
	}
	if (chdir(_argv[1]) < 0)
		fprintf(stderr, "error: %s\n", strerror(errno));
}

char** make_argv(char buf[])
{
	char** _argv = NULL;
	int count = 0;
	char* tokptr = strtok(buf, " ");
	if (tokptr == NULL)
		return NULL;
	++count;
	_argv = realloc(_argv, count*sizeof(char*));
	_argv[count-1] = tokptr;
	while (tokptr) {
		tokptr = strtok(NULL, " ");
		if (tokptr) {
			++count;
			_argv = realloc(_argv, count*sizeof(char*));
			_argv[count-1] = tokptr;
		}
		if (strlen(_argv[count-1]) > SML_BUF_SIZE) {
			printf("too large argument\n");
			free(_argv);
			return NULL;
		}
	}
	_argv = realloc(_argv, (++count)*sizeof(char*));
	_argv[count-1] = NULL;

	return _argv;
}

int has_assign(char** _argv)
{
	if (_argv[1]) {
		return 0;
	}	
	char tmp[BIG_BUF_SIZE]; 
	strcpy(tmp, _argv[0]);
	if (strtok(tmp, "=")) {
		if (strtok(NULL, " ")) {
			return 1;
		}
	}
	return 0;
}

void store_vars(char* buf)
{
	if (g_count == capacity) {
		size_t new_capacity = (capacity == 0) ? 4 : capacity * 2;
		char** tmp = realloc(varList, new_capacity * sizeof(char*));
		if (!tmp) {
			fprintf(stderr, "error: realloc %s\n", strerror(errno));
			return;
		}
		varList = tmp;
		capacity = new_capacity;
	}

	// allocate and copy the string
	varList[g_count] = malloc( strlen(buf) + 1);
	if (!varList[g_count]) {
		fprintf(stderr, "error: malloc %s\n", strerror(errno));
		return;
	}
	strcpy(varList[g_count], buf);
	++g_count;
	printf("%s Stored\n", buf);
}

void free_vars()
{
	for (int i = 0; i < g_count; ++i) {
		/* printf("freeing %s\n", varList[i]); */
		free(varList[i]);
	}
	free(varList);
}

void print_vars()
{
	if (g_count == 0)
		return;
	int i = 0;
	while(varList[i]) {
		printf("%s\n", varList[i++]);
	}
}

char* get_var(char* buf)
{
	char cpy[SML_BUF_SIZE];
	for (int i = 0; i < g_count; ++i) {
		strcpy(cpy, varList[i]);
		strtok(cpy, "=");
		if (!strcmp(buf+1, cpy)) {
			return &varList[i][strlen(buf+1)+1];
		}
	}
	return NULL;
}


void parse_dollar_sign(char **_argv)
{
	int i = 0;
	while(_argv[i]) {
		if (_argv[i][0] == '$') { // What is the max of _argv[i] ????
			char* var = get_var(_argv[i]);
			if (var) {
				_argv[i] = malloc(strlen(var)+1);
				strcpy(_argv[i], var);
			} else {
				_argv[i]  = malloc(sizeof(" "));
				strcpy(_argv[i], " ");
			}
		}
		++i;
	}
}
