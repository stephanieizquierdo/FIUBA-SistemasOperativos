#include "exec.h"


void run_redir(struct execcmd* r);
void close_if_valid(int fd);
void run_pipes(struct pipecmd *p, struct cmd *cmd);
// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i<eargc; i++){
		int pos = block_contains(eargv[i], '=');

		char key[BUFLEN];
		char value[BUFLEN];

		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, pos);

		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	if (flags == READ){
		return open(file, O_RDONLY, O_CLOEXEC);
	} else if (flags == WRITE){
		return open(file, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, S_IWUSR|S_IRUSR);
	} else {
		return -1;
	}
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{	
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		// Your code here
		e = (struct execcmd*) cmd;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		printf_debug("ERROR: Fallo execvp");
		exit(-1);
		break;

	case BACK: {
		// runs a command in background
		//
		// Your code here
		b = (struct backcmd*) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd*) cmd;
		run_redir(r);
		break;
		}
	
	case PIPE: {
		// pipes two commands
		//
		// Your code here
		p = (struct pipecmd*) cmd;
		run_pipes(p, cmd);
		
		// free the memory allocated
		// for the pipe tree structure
		//free_command(parsed_pipe);   //I do it inside of run_pipes

		break;
		}
	}
}

void run_redir(struct execcmd* r){
	int fd_in = -1;
	int fd_out = -1;
	int fd_err = -1;

	if(strlen(r->in_file) > 0){
		fd_in = open_redir_fd(r->in_file, READ);
		if(fd_in == -1){
			printf_debug("ERROR: no se pudo abrir el archivo de entrada");
			exit(-1);
		}
		dup2(fd_in, STDIN_FILENO);
	}
	if (strlen(r->out_file) > 0){
		fd_out = open_redir_fd(r->out_file, WRITE);
		if(fd_out == -1){
			printf_debug("ERROR: no se pudo abrir el archivo de salida");
			exit(-1);
		}
		dup2(fd_out, STDOUT_FILENO);
	}
	if (strlen(r->err_file) > 0) {
		if(block_contains(r->err_file, '&') >= 0){ //caso: 2>&1
			dup2(fd_out, STDERR_FILENO);
		} else {
			fd_err = open_redir_fd(r->err_file, WRITE);
			if(fd_err == -1){
				printf_debug("ERROR: no se puede abrir el archivo de error");
				exit(-1);
			}
			dup2(fd_err, STDERR_FILENO);
		}
		
	}
	close_if_valid(fd_in);
	close_if_valid(fd_out);
	close_if_valid(fd_err);
	execvp(r->argv[0], r->argv);
}

void close_if_valid(int fd){
	if(fd != -1){
		close(fd);
	}
	return;
}

void run_pipes(struct pipecmd *p, struct cmd *cmd){
	int fd[2];
	if(pipe(fd) == -1){
		printf_debug("ERROR: pipe no se puede crear");
		return;
	}
	int pid_left = fork();
	if(pid_left == -1){
		printf_debug("ERROR: Fork fallido");
		close(fd[WRITE]);
		close(fd[READ]);
		free_command(cmd);
		exit(-1);
	}
	if(pid_left == 0){ //es el padre aka izquierdo
		close(fd[READ]);
		dup2(fd[WRITE], STDOUT_FILENO);
		close(fd[WRITE]);
		exec_cmd(p->leftcmd);
	} else {
		int pid_right = fork();
		if(pid_right == -1){
			printf_debug("ERROR: Fork fallido");
			close(fd[WRITE]);
			close(fd[READ]);
			free_command(cmd);
			exit(-1);
		}
		if(pid_right == 0){
			struct cmd* aux = p->rightcmd;
			free_command(p->leftcmd);
			free(cmd);

			close(fd[WRITE]);
			dup2(fd[READ], STDIN_FILENO);
			close(fd[READ]);

			exec_cmd(aux);
		} else {
			close(fd[READ]);
			close(fd[WRITE]);
			waitpid(pid_right, NULL, 0);
			waitpid(pid_left, NULL, 0);			
			free_command(cmd);
			exit(0);
		}
	}
}
