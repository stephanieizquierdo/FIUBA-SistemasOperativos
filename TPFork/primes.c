#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define EXITO 0
#define ERROR -1

#define ARGNUM 1
#define CANT_CORRECTA 2
#define PESCRITURA 1
#define PLECTURA 0

void primes(int lectura);

void primes(int lectura){
    int primo; //el primero es primo
    if(read(lectura, &primo, sizeof(primo)) <= 0){
        close(lectura);
        return;
    }
    printf("primo %i\n", primo);

    int pipe_nuevo[2];
    if(pipe(pipe_nuevo) == ERROR){
        printf("No se pudo crear pipe nuevo");
        return;
    }
    int nuevo = fork();
    if(nuevo == ERROR){
        perror("ERROR: no se pudo forkear\n");
        return;
    }
    if(nuevo){
        close(pipe_nuevo[PLECTURA]);
        int num;
        while(read(lectura, &num, sizeof(num)) > 0){ 
            if(num%primo != 0){
                if(write(pipe_nuevo[PESCRITURA], &num, sizeof(int))==ERROR){
                    perror("ERROR: no se pudo escribir los numeros\n");
                }
            }
        }
        close(lectura);
        close(pipe_nuevo[PESCRITURA]);
        wait(0);
    } else {
        close(pipe_nuevo[PESCRITURA]);
        close(lectura);
        primes(pipe_nuevo[PLECTURA]);
    }
    exit(0);
}

int main(int argc, char** argv){
    if(argc != CANT_CORRECTA){
    	printf("Argumentos inválidos.");
        return ERROR;
    }
    int max_num = atoi(argv[ARGNUM]);
    if(max_num<2){
        printf("Numero inválido. Debe ser igual o mayor a 2");
        return ERROR;
    }
    int pipe_inicial[2];
    if(pipe(pipe_inicial) == ERROR){
        printf("No se pudo crear pipe inicial");
        return ERROR;
    }
    int actual = fork();
    if(actual == ERROR){
        perror("ERROR: no se pudo forkear\n");
        close(pipe_inicial[PLECTURA]);
        close(pipe_inicial[PESCRITURA]);
        return ERROR;
    }
    if(actual){ //soy padre
        close(pipe_inicial[PLECTURA]);
        for(int i = 2; i <= max_num; i++){
            if(write(pipe_inicial[PESCRITURA], &i, sizeof(i)) == ERROR){
                perror("ERROR: no se pudo escribir los numeros\n");
            }
        }
        close(pipe_inicial[PESCRITURA]);
        wait(0);
    } else { //soy hijo
        close(pipe_inicial[PESCRITURA]);
        primes(pipe_inicial[PLECTURA]);
    }
    return EXITO;
}