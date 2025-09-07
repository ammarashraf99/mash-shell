#ifndef NANO_H
#define NANO_H

#define CWD_BUF_SIZE 256
#define BIG_BUF_SIZE 1024
#define SML_BUF_SIZE 256

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>


extern jmp_buf jmpbuffer;

typedef enum {
	ECHO,PWD,CD,EXT,ASIGN,PRINT_ARGS,EXPORT,FRK,IGN,
}Cmd;

/* void pwd(char**); */
/* void cd(char**); */
/* void ext(char**); */
/* void frk(char**); */

/* void store_vars(char **, Cmd); */
/* int has_assign(char **, Cmd); */
/* void free_vars(); */
/* char* get_var(char* buf); */
/* void parse_dollar_sign(char **_argv); */

/* void print_vars(); */


#endif
