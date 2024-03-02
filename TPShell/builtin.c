#include "builtin.h"
#include "utils.h" //to reutilice block_contains
#include "history.h"

#include <stdio.h>

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return (strcmp(cmd, "exit") == 0);
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if(strcmp(cmd, "cd") == 0 ){
		int pos = block_contains(cmd, ' ');

		if(pos != -1){
			char* ptr = strchr(cmd, ' ');
			if(chdir(ptr + 1) == -1){
				perror("ERROR: cd fallo");
				return 0;
			}
			char* cwd = getcwd(NULL, 0);
            if(!cwd){
                perror("ERROR: cd fallo");
                return 0;
            }
			memset(promt, 0, PRMTLEN);
			strncpy(promt, cwd, ARGSIZE);
			free(cwd);
			return 1;

		} else {
			char* home_dir = getenv("HOME");
			if(chdir(home_dir) == -1){
				perror("ERROR: cd a secas fallo");
				return 0;
			}
			memset(promt, 0, PRMTLEN);
			strncpy(promt, home_dir, ARGSIZE);
			return 1;
		}    
	}
	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if(strcmp(cmd,"pwd") == 0){
		char* cwd = getcwd(NULL, 0);
		if(!cwd){
			perror("ERROR: pwd fallo");
		} else {
			printf("%s\n", cwd);
			free(cwd);
			return 1;
		}
	}
	return 0;
}

int history(char* cmd) {
	char* cmd_history = strstr(cmd,"history");
	if(cmd_history){
		char* path  = path_history();
		FILE* history_file = fopen(path, "r");
		if(!history_file) return 0;
		
		if(strcmp(cmd, "history") == 0){ // o sea solo ingresaron history a secas
			show_history(history_file, ALL_HISTORY);		
		} else { //es history con algo mas
			int n = atoi(cmd + strlen("history "));
			show_history(history_file, n);
		}
		fclose(history_file);
		free(path);
		return 1;
	} 
	return 0;
}

char* event_designators(char *cmd, int* debo_guardar){
	if (!cmd) return NULL;

	if (cmd[0] == '!'){
		char* path = path_history();
		FILE* history_file = fopen(path,"r");
		if (!history_file)return NULL;

		char* res = NULL;
		if (strcmp(cmd, "!!")==0|| strcmp(cmd, "!-1") == 0) { //caso !! o !-1 (son sinonimos)
			res = find_command(history_file,1);
			(*debo_guardar) = 0;
		}
		else if (cmd[1] == '-') {  //caso !-n
			int n = atoi(cmd + 2);
			res = find_command(history_file,n);
			(*debo_guardar) = 1;
		}
		fclose (history_file);
		free(path);
		return res;
	}
	return NULL;
}