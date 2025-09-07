#include "vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char **g_varList = NULL;
static size_t g_count = 0;
static size_t g_capacity = 0;

void store_vars(char** _argv, Cmd cmd)
{
	int i = 0;
	if (cmd == EXPORT)
		i = 1; // bypassing "export" command
	while (_argv[i]) {
		if (g_count == g_capacity) {
			size_t new_capacity = (g_capacity == 0) ? 4 : g_capacity * 2;
			char** tmp = realloc(g_varList, new_capacity * sizeof(char*));
			if (!tmp) {
				fprintf(stderr, "error: realloc %s\n", strerror(errno));
				return;
			}
			g_varList = tmp;
			g_capacity = new_capacity;
		}
		// allocate and copy the string
		g_varList[g_count] = malloc( strlen(_argv[i]) + 1);
		if (!g_varList[g_count]) {
			fprintf(stderr, "error: malloc %s\n", strerror(errno));
			return;
		}
		strcpy(g_varList[g_count], _argv[i]);
		++g_count;
		printf("%s Stored\n", _argv[i]);
		++i;
	}
}


void free_vars()
{
	for (int i = 0; i < g_count; ++i) {
		/* printf("freeing %s\n", varList[i]); */
		free(g_varList[i]);
	}
	free(g_varList);
}

void print_vars()
{
	if (g_count == 0)
		return;
	int i = 0;
	while(g_varList[i]) {
		printf("%s\n", g_varList[i++]);
	}
}

char* get_var(char* buf)
{
	char cpy[SML_BUF_SIZE];
	for (int i = 0; i < g_count; ++i) {
		strcpy(cpy, g_varList[i]);
		strtok(cpy, "=");
		if (!strcmp(buf+1, cpy)) {
			return &g_varList[i][strlen(buf+1)+1];
		}
	}
	return getenv(buf+1);
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


void export_vars(char **_argv) 
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
