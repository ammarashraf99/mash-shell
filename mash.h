#ifndef NANO_H
#define NANO_H

#define CWD_BUF_SIZE 256
#define BIG_BUF_SIZE 1024
#define SML_BUF_SIZE 256

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>


extern jmp_buf g_jmpbuffer;
extern char g_input_buf[BIG_BUF_SIZE];

typedef enum {
	ECHO,PWD,CD,EXT,ASIGN,PRINT_ARGS,EXPORT,FRK,IGN,
}Cmd;


#endif
