#include "history.h"
#include <stdlib.h>
#include "utils.h"

char* path_history(void){
	char* path = calloc(1024, sizeof(char));
    
	char* var_path = getenv(ENV_VARIABLE_HISTORY);
	if(var_path){
		strcat(path,var_path);
    } else {
		char* home_path = getenv(HOME_ENV_VARIABLE);
		strcat(path,home_path);
		char* history_file_path = HISTFILE_DEFAULT;
        strcat(path,"/");
		strcat(path,history_file_path);
	}
	return path;
}

void save_history(char* cmd){
	if (!cmd) return;
	char* path = path_history();
	
	FILE* history_file = fopen(path,"a+");
	if (!history_file) perror("ERROR al abrir el archivo de historia de comandos");
	else{
		char enter = '\n';
		fwrite(cmd, sizeof(char), strlen(cmd), history_file);
		fwrite(&enter, sizeof(char), 1, history_file);
		fclose(history_file);
	}
	free(path);
	return;
}

char* find_command(FILE* history_file, int n){	
	int cant_lineas = count_lines(history_file);
	int saltar = (cant_lineas - n > 0? cant_lineas - n : 0);
	rewind(history_file);

	char* linea = NULL;
	size_t size = 0;
	//avanzo hasta las que quiero
	int i = 0;
	while (getline(&linea,&size,history_file) > 0 && i < saltar){
		free(linea);
		linea = NULL;
		size = 0;
		i ++;
	}
	size_t length = strlen(linea);
	linea[length - 1] = '\0';
	return linea;
}


void get_lines(char** almacenador, FILE* history_file){
	char* linea = NULL;
	size_t size = 0;
	int pos = 0;
	int res = 0;
	while(res = getline(&linea, &size, history_file) != -1 ){
		linea[strlen(linea)-1] = '\0';
		strncpy(almacenador[pos], linea, size);
		free(linea);
		linea = NULL;
		size = 0;
		pos++;
	}
	free(linea);
}

void show_history(FILE* history_file,int n){
	if (!n) return;
	if (n == ALL_HISTORY){
		char c = 0;
		fread(&c,sizeof(char),1,history_file);
		while (!feof(history_file)){
			write(STDOUT_FILENO,&c,1);
			fread(&c,sizeof(char),1,history_file);
		}
	} else {
		char* linea = NULL;
		size_t size = 0;
		
		int cant_lineas = count_lines(history_file);
		rewind(history_file);

		// ahora me adelanto hasta la que tengo que empezar a mostrar	
		int lineas_innecesarias = (cant_lineas > n)? cant_lineas - n : 0;
		int contador = 0;
		while (lineas_innecesarias > contador && getline(&linea,&size,history_file) > 0){
			free(linea);
			linea = NULL;
			size = 0;
			contador ++;
		}
		free(linea);
		//ya estoy parada sobre las que tengo que mostrar, las imprimo.
		char c = 0;
		fread(&c,sizeof(char),1,history_file);
		while (!feof(history_file)){
			write(STDOUT_FILENO,&c,1);
			fread(&c,sizeof(char),1,history_file);
		}
	}
	return;
}