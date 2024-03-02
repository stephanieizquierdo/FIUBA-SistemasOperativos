#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

#define EXITO 0
#define ERROR -1

#define PESCRITURA 1
#define PLECTURA 0

void mensajes_primero(int* pipeIda, int* pipeVuelta);
int manejar_padre(int* pipeIda, int* pipeVuelta, int actual);
int manejar_hijo(int* ida, int* vuelta, int actual);

void mensajes_primero(int* pipeIda, int* pipeVuelta){
    printf("Hola, soy PID %d \n", getpid());
    printf("\t - primer pipe me devuelve: [%d, %d]\n", pipeIda[PLECTURA], pipeIda[PESCRITURA]);
    printf("\t - segundo pipe me devuelve: [%d, %d]\n", pipeVuelta[PLECTURA], pipeVuelta[PESCRITURA]);
    return;
}

int manejar_padre(int* pipeIda, int* pipeVuelta, int actual){
    printf("Donde fork me devuelve: %d \n", actual);
    printf("\t - getpid me devuelve: %d \n", getpid()); 
    printf("\t - getppid me devuelve: %d \n", getppid());
    
    close(pipeIda[PLECTURA]);
    close(pipeVuelta[PESCRITURA]);

    int numero = rand();        
    printf("\t - random me devuelve: %i \n", numero);
    printf("\t - envio valor %i a través de fd= %d\n", numero, pipeIda[PESCRITURA]);
    
    if(write(pipeIda[PESCRITURA], &numero, sizeof(int)) == ERROR){
        perror("ERROR: El padre no pudo escribir");
        return ERROR;
    }
    close(pipeIda[PESCRITURA]);
    
    int padre_recibe;
    if(read(pipeVuelta[PLECTURA], &padre_recibe, sizeof(int)) < 1){
        perror("ERROR: El padre no pudo leer");
        return ERROR;
    }

    printf("Hola, de nuevo PID %d\n", getpid());
    printf("\t - recibi valor %i via fd= %d \n", padre_recibe, pipeVuelta[PLECTURA]);
    close(pipeVuelta[PLECTURA]);
    
    return EXITO;
}

int manejar_hijo(int* ida, int* vuelta, int actual){
    printf("Donde fork me devuelve: %d \n", actual);
    printf("\t - getpid me devuelve: %d \n", getpid()); 
    printf("\t - getppid me devuelve: %d \n", getppid());
    
    close(ida[PESCRITURA]);
    close(vuelta[PLECTURA]);

    int recibido;
    
    if(read(ida[PLECTURA], &recibido, sizeof(int)) < 1){
        perror("ERROR: el hijo no pudo leer");
        return ERROR;
    }
    printf("\t - recibo valor %i a través de fd= %d\n", recibido, ida[PLECTURA]);
    close(ida[PLECTURA]);

    printf("\t - reenvío valor en fd= %d y termino\n", vuelta[PESCRITURA]);
    if(write(vuelta[PESCRITURA], &recibido, sizeof(int)) == ERROR){
        perror("ERROR: el hijo no buedo escribir");
        return ERROR;
    }
    close(vuelta[PESCRITURA]);
    return EXITO;
}

int main(){
    int ida[2];
    if(pipe(ida) == ERROR){
        perror("ERROR: no se pudo abrir el pipe ida \n");
        return ERROR;
    }
    int vuelta[2];
    if(pipe(vuelta) == ERROR){
        perror("ERROR: no se pudo abrir el pipe vuelta \n");
        return ERROR;
    }
    srand(time(NULL));
    mensajes_primero(ida, vuelta);
    
    int actual = fork();
    if(actual == ERROR){
        perror("ERROR: no se pudo forkear\n");
        close(ida[PLECTURA]);
        close(ida[PESCRITURA]);
        close(vuelta[PLECTURA])
        close(vuelta[PESCRITURA]);
        return ERROR;
    }
    
    if(actual){
        if(manejar_padre(ida, vuelta, actual) == ERROR){
            exit(ERROR);
        }

    } else {
        if(manejar_hijo(ida, vuelta, actual) == ERROR){
            exit(ERROR);
        }
    }
    return EXITO;
}
