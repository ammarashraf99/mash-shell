#include "parser.h"
#include "mash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


/**
 * clean_argv - cleaning argv to be able to pass it to make_argv
 * 
 * replacing each terminating null with space ' ' to be ready to rebuilt the
 * argument list again.
 */
void clean_argv(char** _argv) 
{
	int i = 1;
	while(_argv[i]) {
		_argv[i][-1] = ' ';
		++i;
	}
}


/**
 * pop_argv - removing one argument from argv list
 *
 * @_argv: argument list
 * @index: index to the element to be popped
 *
 * only moving the subsequent arguments one position back
 * and not resizing anything
*/
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
