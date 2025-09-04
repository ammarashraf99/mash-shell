#include "parser.h"
#include "nano.h"


char** make_argv(char buf[])
{
	/* char temp_buf[BIG_BUF_SIZE]; */
	/* strcpy(temp_buf, buf); */
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

int has_assign(char **_argv, Cmd cmd)
{
	int i = 0;
	if (cmd == EXPORT)
		i = 1; // bypassing "export" command
	while(_argv[i]) {
		char tmp[BIG_BUF_SIZE];
		strcpy(tmp, _argv[i]);
		if ( !(strtok(tmp, "=") && strtok(NULL, " ")) ) {
			return 0;
		}
		++i;
	}
	return 1;
}



void parse_dollar_sign(char **_argv)
{
	int i = 0;
	while(_argv[i]) {
		if (_argv[i][0] == '$') {
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
