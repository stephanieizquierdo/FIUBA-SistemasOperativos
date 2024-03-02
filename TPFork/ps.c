//Implemente el equivalente a: ps -eo pid,comm,state,ppid,pgrp,session
//Uso: ./ps
//Experiencia:
//Investigue mucho en como imprimir prolijamente la salida como si fueran columnas pero no salio como lo esperaba, por eso decidi printear como listados.
//Me parecio sencillo este desafio seguiendo lo que pusieron en la pag de la materia y el apartado del lab unix, no mucho m'as que comentar.

#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define ERROR -1

int main(){
    DIR* dir = opendir("/proc");
    if(!dir) {
        perror("ERROR: No se puede abrir el directorio");
        return ERROR;
    }
    struct  dirent* lectura = readdir(dir);
    while(lectura){
        if (isdigit(*lectura->d_name)){
            char statDir[5+1+256+5]; //Ruta: /proc/[PID]/stat
            char* pid = lectura->d_name;
            sprintf(statDir,"/proc/%d/stat", pid);

            FILE *statfile = fopen(statDir, "r");
            if (statfile){ // Sigo las instrucciones del manual, proc(5)
                int pid;
                char comm[1024];
                char state;
                int ppid;
                int pgrp;
                int session;

                if(fscanf(statfile, "%d %s %c %d %d %d", &pid, comm, &state, &ppid, &pgrp, &session) != EOF) {
                    printf("%d %s\n\t\tEstado del proceso: %c\n\t\tID del padre: %d\n\t\tID del grupo padre: %d\n\t\tSession: %d\n\n", pid, comm, state, ppid, pgrp, session);
                }
                fclose(statfile);
            }
        }
        lectura = readdir(dir);
    }
    closedir(dir);

    return 0;    
}