/**
 *  
 *  file: redirection_parser.c
 *
 *  description: responsible for parsing redirection operators from a command line,
 *               and setting file descriptors accordingly
 *
 * author: Ammar Ashraf
 * date: 2025-09-05
 *
 **/

#include "redirection_parser.h"

/* global variables definitions */
char g_input_buf[BIG_BUF_SIZE];
int g_saved_out_fd = INVALID_FD;
int g_saved_err_fd = INVALID_FD;
int g_saved_in_fd  = INVALID_FD;

/* static functions */
static void register_files(char **, struct ToBePopped*, int);
static void set_out_fd(char*);
static void set_err_fd(char*);
static void handle_squished(char ***, const int *const, char*, struct ToBePopped*, int*, int);
static void handle_to_right_2(char **, const int *const, struct ToBePopped *, int *);
static void handle_in_middle(char **, int *, struct ToBePopped *, int *, int);
static void handle_to_right(char **, int *i, struct ToBePopped *, int *, int);
static void handle_to_left(char **, int * const, char*, struct ToBePopped *, int *, int);
static void handle_in_middle_2(char** , const int *const, struct ToBePopped *, int *);

/**
 * clean_argv - cleaning argv to be able to pass it to make_argv
 * 
 * replacing each terminating null with space ' '
 */
void clean_argv(char** _argv) 
{
	int i = 1;
	while(_argv[i]) {
		_argv[i][-1] = ' ';
		++i;
	}
}

/**
 * reset_fds - resetting the out, in, and error redirected file descriptors
 *
 * closing the current open file if it exists and reseting to the saved fd
 */
void reset_fds() {
	if (g_saved_out_fd >= 0) { // >=0 [valid fd]
		close(STDOUT_FILENO);
		dup2(g_saved_out_fd, STDOUT_FILENO);
		g_saved_out_fd = INVALID_FD;
	}
	if (g_saved_err_fd >= 0) {
		close(STDERR_FILENO);
		dup2(g_saved_err_fd, STDERR_FILENO);
		g_saved_err_fd = INVALID_FD;
	}
	if (g_saved_in_fd >= 0) {
		close(STDIN_FILENO);
		dup2(g_saved_in_fd, STDIN_FILENO);
		g_saved_in_fd = INVALID_FD;
	}
}

/**
 * pop_argv - removing one argument from argv list
 *
 * @_argv: argument list
 * @index: index to the element to be popped
 *
 * only moving the subsequent arguments one position back
 * and not resizing anything
*/
char* pop_argv(char **_argv, int index)
{
	char* ret = _argv[index];
	int i = index;
	while (_argv[i] && _argv[i+1]) {
		_argv[i] = _argv[i+1];
		++i;
	}
	_argv[i] = NULL;
	return ret;
}

/**
 * set_out_fd - redirecting the out to filename
 *
 * @filename: pointer to the name of the popped argv
 *
 * opening the file and dup2 to the outfd (STDOUT_FILENO) after
 * saving a global fd pointing the same file table entry
 */
static void set_out_fd(char* filename)
{
	g_saved_out_fd = dup(STDOUT_FILENO);
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, REDIRECTION_OPEN_MODE);
	if (fd >= 0) { // valid fd
		dup2(fd, STDOUT_FILENO);
	} else if (errno == EFAULT){
		fprintf(stderr, "mash: syntax error near unexpected token `newline`\n");
		close(fd);
		longjmp(jmpbuffer,1);
	} else {
		fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		close(fd);
		longjmp(jmpbuffer,1);
	}
	close(fd);
}

/**
 * set_err_fd - redirecting the err to filename
 *
 * @filename: pointer to the name of the popped argv
 *
 * opening the file and dup2 to the errfd (STDERR_FILENO) after
 * saving a global fd pointing the same file table entry
 */
static void set_err_fd(char *filename)
{
	g_saved_err_fd = dup(STDERR_FILENO);
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, REDIRECTION_OPEN_MODE);
	if (fd >= 0) { // valid fd
		dup2(fd, STDERR_FILENO);
	} else if (errno == EFAULT) {
		fprintf(stderr, "mash: syntax error near unexpected token `newline`\n");
		close(fd);
		longjmp(jmpbuffer,1);
	} else {
		fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		close(fd);
		longjmp(jmpbuffer,1);
	}
	close(fd);
}

/**
 * set_in_fd - redirecting the in from filename
 *
 * @filename: pointer to the name of the popped argv
 *
 * opening the file and dup2 to the infd (STDIN_FILENO) after
 * saving a global fd pointing the same file table entry
 */
static void set_in_fd(char *filename)
{
	g_saved_in_fd = dup(STDERR_FILENO);
	int fd = open(filename, O_RDONLY);
	if (fd >= 0) { // valid fd
		dup2(fd, STDIN_FILENO);
	} else if (errno == EFAULT) {
		fprintf(stderr, "mash: syntax error near unexpected token `newline`\n");
		close(fd);
		longjmp(jmpbuffer,1);
	} else {
		fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		close(fd);
		longjmp(jmpbuffer,1);
	}
	close(fd);
}

/**
 * register_out_err_files - registers output and error redirection files
 *
 * @_argv: the argument list of the user command
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @to_pop_count: a counter of how many on the array of structs
 *
 * popping out all marked arguments from _argv and setting the last one 
 * in the command line to be the one to be directed to
 */
static void register_files(char **_argv, struct ToBePopped* to_be_popped, int to_pop_count)
{
	int out_first_flag = 1;
	int err_first_flag = 1;
	int in_first_flag = 1;
	while (to_pop_count >= 0) {
		if (to_be_popped[to_pop_count].type == OUT_REDIRECTION) {
			if (out_first_flag) {
				set_out_fd(pop_argv(_argv,to_be_popped[to_pop_count].index));
				--to_pop_count;
				out_first_flag = 0;
			} else {
				pop_argv(_argv,to_be_popped[to_pop_count].index);
				--to_pop_count;
			}
		} else if (to_be_popped[to_pop_count].type == ERR_REDIRECTION) {
			if (err_first_flag) {
				set_err_fd(pop_argv(_argv, to_be_popped[to_pop_count].index));
				--to_pop_count;
				err_first_flag = 0;
			} else {
				pop_argv(_argv, to_be_popped[to_pop_count].index);
				--to_pop_count;
			}
		} else if (to_be_popped[to_pop_count].type == IN_REDIRECTION) {
			if (in_first_flag) {
				set_in_fd(pop_argv(_argv, to_be_popped[to_pop_count].index));
				--to_pop_count;
				in_first_flag = 0;
			} else {
				pop_argv(_argv, to_be_popped[to_pop_count].index);
				--to_pop_count;
			}
		}
	}
}

/**
 * handle_in_middle_2 - handles the in the middle err op case
 *
 * @_argv: the argument list of the user command
 * @i: the index to the processed argument in _argv
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @to_pop_count: a counter of how many on the array of structs [should reference the caller variable]
 *
 *
 * processes the case where the error operator '2>' is in between both arguments
 * separated with spaces e.g., $ cmd 2> file
 */
static void handle_in_middle_2(char** _argv, const int *const i, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	pop_argv(_argv, *i); // poping the the whole argument (2>)
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = ERR_REDIRECTION;
	++(*to_pop_count);
}

/**
 * handle_to_left - handles the to the left operator case
 *
 * @_argv: the argument list of the user command
 * @i: the index to the processed argument in _argv
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @c: pointer to the 'operator' location
 * @to_pop_count: a counter of how many on the array of structs [should reference the caller variable]
 *
 * processes the case where the redirection operator is alighned to the left
 * between the two arguments e.g., $ cmd> file
 */
static void handle_to_left(char **_argv,  int * const i,
			   char* c, struct ToBePopped *to_be_popped, int *to_pop_count, int in_or_out)
{
	c[0] = 0; // terminates at the redirection operator
	to_be_popped[*to_pop_count].index = *i+1; // this one is guaranteed to be done so inc i
	to_be_popped[*to_pop_count].type = in_or_out;
	++(*to_pop_count);
	/* --(*i); */
}

/**
 * handle_to_right - handles the to the right operator case
 *
 * @_argv: the argument list of the user command
 * @i: the index to the processed argument in _argv
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @to_pop_count: a counter of how many on the array of structs [should reference the caller variable]
 *
 * processes the case where the operator (e.g.'>') is alighned to the right
 * between the two arguments e.g., $ cmd >file
 */
static void handle_to_right(char **_argv, int *i, struct ToBePopped *to_be_popped,
			    int *to_pop_count, int in_or_out)
{
	++_argv[*i];
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = in_or_out;
	++(*to_pop_count);
	--(*i); // the same position needs to be proccessed agian incase the
	        // argument still has another operator
}

/**
 * handle_in_middle - handles the in the middle operator case
 *
 * @_argv: the argument list of the user command
 * @i: the index to the processed argument in _argv
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @to_pop_count: a counter of how many on the array of structs [should reference the caller variable]
 *
 * processes the case where the error operator '2>' is in between both arguments
 * separated with spaces e.g., $ cmd 2> file
 */
static void handle_in_middle(char **_argv, int *i, struct ToBePopped *to_be_popped,
			     int *to_pop_count, int in_or_out)
{
	pop_argv(_argv, *i); // pop the whole argument which represent a operator
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = in_or_out;
	++(*to_pop_count);
	--(*i); // the same position needs to be proccessed agian the next loop beacuse it
	        // has been replaced with following on after popping it
}

/**
 * handle_to_right_2 - handles the err redirection case to the right
 *
 * @_argv: the argument list of the user command
 * @i: pointer to the index to the processed argument in _argv in the calling function
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @to_pop_count: a counter of how many on the array of structs [should reference the caller variable]
 *
 * processes the case wehre the err operator '2>' is alighned to the right betweent
 * two arguments e.g., $ Cmd 2>file
 */
static void handle_to_right_2(char **_argv, const int *const i, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	_argv[*i] += 2; // move the argument pointer two positions to skip the '2>' operator
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = ERR_REDIRECTION;
	++(*to_pop_count);
}

/**
 * handle_squished - handles the squished operator case
 *
 * @_argv: the argument list of the user command. taken by pointer (char***) because it changes 
 *         it's address after calling make_argv which reallocates to fit the new size
 * @i: the index to the processed argument in _argv
 * @c: pointer to the 'operator' location
 * @to_be_popped: an array of structs holding a record of arguments to be
 *                popped
 * @to_pop_count: a counter of how many on the array of structs [should reference the caller (parse_IO_redirections) variable]
 *
 * processes the case where the operator is stuck to both arguments
 *                   surrounding it e.g., $ cmd>file
 */
static void handle_squished(char ***_argv, const int *const i,
			    char* c, struct ToBePopped *to_be_popped, int *to_pop_count, int in_or_out)
{
	*c = ' '; // replaces the redirection operator with a space
	clean_argv(*_argv);
	free(*_argv);
	*_argv = make_argv(g_input_buf);
	/* the reconstruced argv now has one more argument after splitting the processed arggument*/
	to_be_popped[*to_pop_count].index = (*i)+1; // the next argument is put to be popped due to the resizing
	to_be_popped[*to_pop_count].type = in_or_out;
	++(*to_pop_count);
}

/**
 * parse_IO_redirections - parses and processes all redirections
 *
 * @_argv: the argument list of the user command.
 *
 * parses the three kinds of redirection operators (>, 2>, <).
 *
 */
char** parse_IO_redirections(char** _argv)
{
	struct ToBePopped to_be_popped[STACK_MAX_CAP];
	int in_or_out;
	int to_pop_count = 0;
	int i = 0;
	while(_argv[i]) {
		char *c;
		if ( (c = strchr(_argv[i], '>')) ) {
			in_or_out = OUT_REDIRECTION;
		} else if ((c = strchr(_argv[i], '<'))){
			in_or_out = IN_REDIRECTION;
		}
		if (c != NULL) {
			if (TO_THE_LEFT) {
				if (IS_2) {
					handle_in_middle_2(_argv, &i, to_be_popped, &to_pop_count);
				} else {
					handle_to_left(_argv, &i, c, to_be_popped, &to_pop_count, in_or_out);
				}
			} else if (TO_THE_RIGHT) {
				handle_to_right(_argv, &i, to_be_popped, &to_pop_count, in_or_out);
			} else if (IN_THE_MIDDLE) {
				handle_in_middle(_argv, &i, to_be_popped, &to_pop_count, in_or_out);
			} else if (SQUISHED) {
				if (IS_2) {
					handle_to_right_2(_argv, &i, to_be_popped, &to_pop_count);
				} else {
					handle_squished(&_argv, &i, c, to_be_popped, &to_pop_count, in_or_out);
				}
			}
		}
		++i;
	}
	--to_pop_count; // fixing the last extra increment
	register_files(_argv, to_be_popped, to_pop_count);

	return _argv;
}
