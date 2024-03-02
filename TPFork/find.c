#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

#define _GNU_SOURCE //Necesario para strcasestr
#define CANT_MAX_ARG 3
#define EXITO 0
#define ERROR -1
#define DIR_ACTUAL "."
#define DIR_INM_SUP ".."

//Compara segun si es case sensitive o no
char* comparar(char* path_actual, char* palabra, int es_case_sensitive);

void find(int dir_fd, char* path_actual, char* palabra, int es_case_sensitive);

//verifica los argumentos ingresados
int verificacion(int argc, char *argv[]);

char* comparar(char* path_actual, char* palabra, int es_case_sensitive){
	if(es_case_sensitive){
		return strstr(path_actual, palabra);
	} else {
		return strcasestr(path_actual, palabra);
	}
}

void find(int dir_fd, char* path_actual, char* palabra, int es_case_sensitive){
	DIR* actual_dir = fdopendir(dir_fd);

	char nuevo_path[PATH_MAX];
    memset(nuevo_path, 0, PATH_MAX);
	
	struct dirent* lectura;
    while(lectura = readdir(actual_dir)){
		if((strcmp(lectura->d_name, DIR_ACTUAL) != 0) && (strcmp(lectura->d_name, DIR_INM_SUP) != 0)){
			
			strncpy(nuevo_path, path_actual, PATH_MAX);

			if(path_actual[0] != '\0'){
				strcat(nuevo_path, "/");
			}
			strcat(nuevo_path, lectura->d_name);
			
			if(comparar(lectura->d_name, palabra, es_case_sensitive)){
                printf("%s\n", nuevo_path);
			}
			int prox_fd = openat(dir_fd, lectura->d_name, O_DIRECTORY);
			if(prox_fd != ERROR){
				find(prox_fd, nuevo_path, palabra, es_case_sensitive);
			}
		}
	}
	closedir(actual_dir);
}

int verificacion(int argc, char *argv[]){
	int es_case_sensitive;
	if(argc == 1 || argc > CANT_MAX_ARG){
		printf("ERROR: cantidad de argumentos incorrecta\n");
		return ERROR;
	} else if (argc == CANT_MAX_ARG) {
		if(strcmp(argv[1],"-i")==0){
			es_case_sensitive = 0;
		} else {
			printf("ERROR: argumentos no validos\n");
			return ERROR;
		}
	} else {
		es_case_sensitive = 1;
	}
	return es_case_sensitive;
}

int main(int argc, char *argv[]) {
	char* palabra;

	int modo_sensitive = verificacion(argc, argv);
	if(modo_sensitive == ERROR){
		return ERROR;
	}
	if(modo_sensitive){
		palabra = argv[1];
	} else {
        palabra = argv[2];
    }

	DIR* dir = opendir(DIR_ACTUAL);
	if(!dir){
        perror("ERROR: No se puede abrir el directorio");
		return ERROR;
    }
	int dir_fd = dirfd(dir);

	char path[PATH_MAX];
    memset(path, 0, sizeof(path));

	find(dir_fd, path, palabra, modo_sensitive);
	closedir(dir);

	return 0;
}