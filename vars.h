#ifndef VARS_H
#define VARS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mash.h"
#include "parser.h"



void export(char **_argv);
void store_vars(char** _argv, Cmd cmd);
void free_vars();
void print_vars();
char* get_var(char* buf);


#endif
