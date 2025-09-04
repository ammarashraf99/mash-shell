#ifndef NANO_H
#define NANO_H

#define CWD_BUF_SIZE 256
#define BIG_BUF_SIZE 1024
#define SML_BUF_SIZE 256

char** make_argv(char buf[]);
void echo(char **);
void pwd(char**);
void cd(char**);
void ext(char**);
void frk(char**);

void store_vars(char *buf);
int has_assign(char **buf);
void free_vars();
char* get_var(char* buf);
void parse_dollar_sign(char **_argv);

void print_vars();

typedef enum {
	ECHO,PWD,CD,EXT,ASIGN,PRINT_ARGS,SOURCE,FRK,
}Cmd;

#endif
