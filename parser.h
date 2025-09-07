#ifndef PARSER_H
#define PARSER_H


char** make_argv(char buf[]);
void clean_argv(char** _argv);
char* pop_argv(char **_argv, int index);


#endif
