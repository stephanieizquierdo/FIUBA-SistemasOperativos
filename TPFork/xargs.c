#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define EXITO 0
#define ERROR -1

#ifndef NARGS
#define NARGS 4
#endif

extern char* strdup(const char*);
//Args debe estar cargado con los argumentos a pasar al comando(args[0])
void lanzar(char** args);

//Libera la memoria alocada para el vector y reinicia el contador
void reiniciar(char** args, int* contador);

void lanzar(char** args){
    int actual = fork();
    if(actual == ERROR){
        perror("ERROR: no se pudo forkear\n");
        return;
    }
    if(actual){ //padre 
        wait(0);
    } else if (!actual) {
        int res = execvp(args[0], args);
        if(res == ERROR){
            perror("ERROR: No se pudo ejecutar el comando\n");
            exit(ERROR);
        }
    } else {
        perror("ERROR: No se pudo ejecutar el comando\n");
        exit(ERROR);
    }
}

void reiniciar(char** args, int* contador){
    for (int i = 1; i < NARGS + 1; i++) {
        free(args[i]);
    }
    *contador = 1;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("ERROR: Cantidad incorrecta de argumentos");
        exit(ERROR);
    }
    char* args[NARGS + 2]; //en arg[0] va el comando y al final el NULL
    args[0] = strdup(argv[1]);
    args[NARGS + 1] = NULL;

    char* linea = NULL;
    size_t largo = 0;
    int contador = 1;

    while(getline(&linea, &largo, stdin) > 0){
        linea[strlen(linea)-1] = '\0';
        args[contador] = strdup(linea);
        contador++;
        if(contador == NARGS+1){
            lanzar(args);
            reiniciar(args, &contador);
        }
    }
    if(contador < NARGS+1 && contador > 1){
        contador--;
        int i = contador + 1;
        while(i < NARGS+1){
            args[i] = NULL;
            i++;
        }
        lanzar(args);
        reiniciar(args, &contador);
    }

    free(args[0]);
    free(linea);
    exit(EXITO);
}
