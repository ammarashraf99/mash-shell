#include "redirection_parser.h"

char g_input_buf[BIG_BUF_SIZE];
int g_saved_out_fd = INVALID_FD;
int g_saved_err_fd = INVALID_FD;

static void register_files(char **_argv, struct ToBePopped* to_be_popped, int to_pop_count);
static void set_out_fd(char* filename);
static void set_err_fd(char* filename);
static void handle_squished(char ***_argv, const int *const i, char* c, struct ToBePopped *to_be_popped, int *to_pop_count);
static void handle_to_right_2(char **_argv, const int *const i, struct ToBePopped *to_be_popped, int *to_pop_count);
static void handle_in_middle(char **_argv, int *i, struct ToBePopped *to_be_popped, int *to_pop_count);
static void handle_to_right(char **_argv, int *i, struct ToBePopped *to_be_popped, int *to_pop_count);
static void handle_to_left(char **_argv, const int * const i, char* c, struct ToBePopped *to_be_popped, int *to_pop_count);
static void handle_in_middle2(char** _argv, const int *const i, struct ToBePopped *to_be_popped, int *to_pop_count);

// static void set_in_fd(char* filename);

void clean_argv(char** _argv) 
{
	int i = 1;
	while(_argv[i]) {
		_argv[i][-1] = ' ';
		++i;
	}
}

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
}

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


static void set_out_fd(char* filename)
{
	g_saved_out_fd = dup(STDOUT_FILENO);
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, REDIRECTION_OPEN_MODE);
	dup2(fd, STDOUT_FILENO);
	close(fd);
}

static void set_err_fd(char* filename)
{
	g_saved_err_fd = dup(STDERR_FILENO);
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, REDIRECTION_OPEN_MODE);
	dup2(fd, STDERR_FILENO);
	close(fd);
}

static void register_files(char **_argv, struct ToBePopped* to_be_popped, int to_pop_count)
{
	int out_first_flag = 1;
	int err_first_flag = 1;
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
		} else {
			if (err_first_flag) {
				set_err_fd(pop_argv(_argv, to_be_popped[to_pop_count].index));
				--to_pop_count;
				err_first_flag = 0;
			} else {
				pop_argv(_argv, to_be_popped[to_pop_count].index);
				--to_pop_count;
			}
		}
	}
}

static void handle_in_middle2(char** _argv, const int *const i, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	pop_argv(_argv, *i); // poping the 2>
	// check the size of to_pop_count if it reaches STACK_MAX_CAP
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = ERR_REDIRECTION;
	++(*to_pop_count);
}

static void handle_to_left(char **_argv, const int * const i, char* c, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	c[0] = 0;
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = OUT_REDIRECTION;
	++(*to_pop_count);
}

static void handle_to_right(char **_argv, int *i, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	++_argv[*i];
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = OUT_REDIRECTION;
	++(*to_pop_count);
        --(*i);
}

static void handle_in_middle(char **_argv, int *i, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	pop_argv(_argv, *i);
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = OUT_REDIRECTION;
	++(*to_pop_count);
	--(*i);
}

static void handle_to_right_2(char **_argv, const int *const i, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	_argv[*i] += 2;
	to_be_popped[*to_pop_count].index = *i;
	to_be_popped[*to_pop_count].type = ERR_REDIRECTION;
	++(*to_pop_count);
}

static void handle_squished(char ***_argv, const int *const i, char* c, struct ToBePopped *to_be_popped, int *to_pop_count)
{
	*c = ' '; 
	clean_argv(*_argv);
	free(*_argv);
	*_argv = make_argv(g_input_buf);
	to_be_popped[*to_pop_count].index = (*i)+1;
	to_be_popped[*to_pop_count].type = OUT_REDIRECTION;
	++(*to_pop_count);
}

char** parse_IO_redirections(char** _argv)
{
	struct ToBePopped to_be_popped[STACK_MAX_CAP];
	int to_pop_count = 0;
	int i = 0;
	while(_argv[i]) {
		char* c = strchr(_argv[i], '>');
		if (c) {
			if (TO_THE_LEFT) { 
				if (IS_2) {
					handle_in_middle2(_argv, &i, to_be_popped, &to_pop_count);
				} else {
					handle_to_left(_argv, &i, c, to_be_popped, &to_pop_count);
				}
			} else if (TO_THE_RIGHT) {
				handle_to_right(_argv, &i, to_be_popped, &to_pop_count);
			} else if (IN_THE_MIDDLE) {
				handle_in_middle(_argv, &i, to_be_popped, &to_pop_count);
			} else if (SQUISHED) {
				if (IS_2) {
					handle_to_right_2(_argv, &i, to_be_popped, &to_pop_count);
				} else {
					handle_squished(&_argv, &i, c, to_be_popped, &to_pop_count);
				}
			}
		}
		++i;
	}
	--to_pop_count;
	register_files(_argv, to_be_popped, to_pop_count);
	return _argv;
}
