#include "runcmd.h"
#include "history.h"

int status = 0;
struct cmd *parsed_pipe;


//Run cmd normal, sin saber de historial.
static int run_cmd_without_history(char* cmd) {
	pid_t p;
	struct cmd *parsed;

	if (cmd[0] == END_STRING){
		return 0;
	}
	printf("\n");

	if (cd(cmd)){
		return 0;
	}

	if (exit_shell(cmd)){
		return EXIT_SHELL;
	}
	if (pwd(cmd)){
		return 0;
	}
	if(history(cmd)){
		return 0;
	}
	parsed = parse_line(cmd);

	if ((p = fork()) == 0) {
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
	}

	parsed->pid = p;

	if(parsed->type == BACK){
		print_back_info(parsed);
		waitpid(p, &status, WNOHANG);
	}else {
		waitpid(p, &status, 0);
	}
	print_status_info(parsed);

	free_command(parsed);
	return 0;
}

// runs the command in 'cmd'
//DESAFIO: ahora ademas se tienen que guardar los comandos (si corresponde) para el historial
//        y manejar el caso en que el comando sea un event designator
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	//me fijo si es un event designator
	int should_save = 0;
	int r = -1;
	char* new_cmd = event_designators(cmd, &should_save);
	if(new_cmd){
		printf("\n%s", new_cmd);
		if(should_save){
			r = run_cmd(new_cmd); 
			free(new_cmd);
			return r;
		} else {
			r = run_cmd_without_history(new_cmd);
			free(new_cmd);
			return r;
		}
	}

	// if the "enter" key is pressed
	// just print the promt again
	if (cmd[0] == END_STRING){
		return 0;
	}
	
	//guardo el cmd
	save_history(cmd);
	printf("\n");

	// "cd" built-in call
	if (cd(cmd)){
		return 0;
	}
	// "exit" built-in call
	if (exit_shell(cmd)){
		return EXIT_SHELL;
	}
	// "pwd" buil-in call
	if (pwd(cmd)){
		return 0;
	}
	//"history"
	if(history(cmd)){
		return 0;
	}
	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {
		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
	}

	// store the pid of the process
	parsed->pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is
	//		going to be run in the 'back'
	// - print info about it with
	// 	'print_back_info()'
	//
	// Your code here
	if(parsed->type == BACK){
		print_back_info(parsed);
		waitpid(p, &status, WNOHANG);
	}else {
		// waits for the process to finish
		waitpid(p, &status, 0);
	}
	print_status_info(parsed);

	free_command(parsed);
	if(new_cmd) free(new_cmd);
	return 0;
}
