#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mash.h"
#include "vars.h"


char** make_argv(char buf[]);
int has_assign(char **_argv, Cmd cmd);
void parse_dollar_sign(char **_argv);


#endif
