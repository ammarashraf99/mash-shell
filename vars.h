#ifndef VARS_H
#define VARS_H

#include "mash.h"



void store_vars(char** _argv, Cmd cmd);
void free_vars();
void print_vars();
char* get_var(char* buf);
int has_assign(char **_argv, Cmd cmd);
void parse_dollar_sign(char **_argv);
void export_vars(char **_argv);

#endif
