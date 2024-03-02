//Enunciado: Se pide implementar una versión simplificada de: ls -al
//uso: ./ls
//Experiencia: Este desafio se pudo encarar facilmente con la info de la pag de la materia, el find y el man para el caso de enlaces simbolicos.
//Tuve que investigar mas en profundidad dentro del manual el struct st_mode para poder imprimir correctamente la informacion como ls -al

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define ERROR -1
#define DIR_ACTUAL "."

//Funcion necesaria dado que st_mode necesita enmascararse para poder ser printeado correctamente
void cargar_permisos(char* permisos, mode_t modo);

void cargar_permisos(char* permisos, mode_t modo){ //hecho con la informacion de: man 7 inode
    permisos[0] = (modo & S_IRUSR)? 'r' : '-'; //permisos para el usuario
    permisos[1] = (modo & S_IWUSR)? 'w' : '-';
    permisos[2] = (modo & S_IXUSR)? 'x' : '-';

    permisos[3] = (modo & S_IRGRP)? 'r' : '-'; //permisos para el grupo
    permisos[4] = (modo & S_IWGRP)? 'w' : '-';
    permisos[5] = (modo & S_IXGRP)? 'x' : '-';

    permisos[6] = (modo & S_IROTH)? 'r' : '-'; // permisso para otros
    permisos[7] = (modo & S_IWOTH)? 'w' : '-';
    permisos[8] = (modo & S_IXOTH)? 'x' : '-';

    permisos[9] = '\0';
}

int main(){ //comienzo como si fuera un find
    DIR* dir = opendir(DIR_ACTUAL);
    if(!dir){
        perror("ERROR: No se puede abrir el directorio");
        return ERROR;
    }
    int dir_fd = dirfd(dir);
    DIR* actual_dir = fdopendir(dir_fd);
    struct dirent* lectura = readdir(actual_dir);

    while(lectura){
        struct stat sb;
        if(stat(lectura->d_name, &sb) != 0){  //Necesario para la info a imprimir
            perror("ERROR: Stat. ");
            closedir(actual_dir);
            closedir(dir);
            return ERROR;
        }
        char permisos[10]; 
        memset(permisos,0,10);
        cargar_permisos(permisos, sb.st_mode); //ver comentario de la firma

		if(lectura->d_type == DT_LNK){ //es un enlace simbolico
            char nombre_link[PATH_MAX];
            memset(nombre_link, 0, PATH_MAX);

            if(readlinkat(dir_fd, lectura->d_name, nombre_link, PATH_MAX) > 0){ //pone en nombre_link la ruta de a lo que alude el enlace simbolico.
                printf("%c%s %ld %s -> %s\n", lectura->d_type, permisos, (long)sb.st_uid, lectura->d_name, nombre_link);
            }
        } else {
            printf("%c%s %ld %s\n", lectura->d_type, permisos, (long)sb.st_uid, lectura->d_name);
        }
		lectura = readdir(actual_dir);
	}
    closedir(actual_dir);
    closedir(dir);

    return 0;
}