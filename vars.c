#include "vars.h"


char **varList = NULL;
size_t g_count = 0;
size_t capacity = 0;

void store_vars(char** _argv, Cmd cmd)
{
	int i = 0;
	if (cmd == EXPORT)
		i = 1; // bypassing "export" command
	while (_argv[i]) {
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
		varList[g_count] = malloc( strlen(_argv[i]) + 1);
		if (!varList[g_count]) {
			fprintf(stderr, "error: malloc %s\n", strerror(errno));
			return;
		}
		strcpy(varList[g_count], _argv[i]);
		++g_count;
		printf("%s Stored\n", _argv[i]);
		++i;
	}
}

void export(char **_argv) 
{
	int i = 1; // bypassing "export" command
	if (!has_assign(_argv,EXPORT)) {
		return;
	}
	store_vars(_argv, EXPORT);
	while(_argv[i]) {
		putenv(_argv[i]);
		++i;
	}
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
	return getenv(buf+1);
}
