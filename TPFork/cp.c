//Uso: ./cp <fileoriginal> <filedestino>
//Experiencia: Para este desafio tuve que investigar en profundida en el manual y ademas me sirvieron los tips de la pag de la materia.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>

#define CANT_CORRECTA_ARGS 3
#define FILE_ORIGEN 1
#define FILE_DESTINO 2
#define ERROR -1

int main(int argc, char *argv[]){
    if(argc != CANT_CORRECTA_ARGS){
        printf("ERROR: Cantidad incorrecta de argumentos\n");
        exit(ERROR);
    }
    char* origen = argv[FILE_ORIGEN];
    char* destino = argv[FILE_DESTINO];

    int fd_origen = open(origen, O_RDONLY);
    if (fd_origen == ERROR) {
        perror("ERROR: No se pudo abrir el archivo de origen");
        exit(ERROR);
    }
                                    //Flags:lectoescritura,creat: si no existe lo crea, EXCL: si existe falla y  truncamiento.
    int fd_destino = open(destino, O_RDWR | O_CREAT | O_EXCL |O_TRUNC, S_IRUSR | S_IWUSR);//modos de permiso: en este caso solo pongo al usuario el permiso de leer y escribir. ya que luego se setearan todos los permisos como el original
    if (fd_destino == ERROR) {
        close(fd_origen);
        perror("ERROR: No se pudo crear archivo destino");
        return ERROR;
    }

    struct stat sb;  //Necesario para saber el tamanio del archivo
    if (fstat(fd_origen, &sb) ==  ERROR) { 
        close(fd_destino);
        close(fd_origen);
        perror("Error en stat del archivo de entrada");
        return ERROR;
    }

    if(!sb.st_size){
        close(fd_destino);
        close(fd_origen);
        return ERROR;
    }

    origen = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd_origen, 0); //mapeo de memoria del origen: solo lectura y es privado es decir que otros procesos no peuden verlo
    if (origen == MAP_FAILED){
        printf("ERROR: No se pudo reservar memoria en el virtual address space para el archivo origen\n");
        close(fd_destino);
        close(fd_origen);
        return ERROR;
    }
    if(ftruncate(fd_destino, sb.st_size) == ERROR){
        printf("ERROR: No se pudo truncar el archivo destino\n");
        close(fd_destino);
        close(fd_origen);
        return ERROR;
    }
    
    destino = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_destino, 0);//mapeo de lectoescritura y tiene que ser compartido para que pueden verlo otros procesos y efectuar realmente el cambio, que lo vea el sistema.
    if(destino == MAP_FAILED){
        printf("ERROR: No se pudo reservar memoria en el virtual address space para el archivo destino\n");
        close(fd_destino);
        close(fd_origen);
        return ERROR;
    }
    memcpy(destino, origen, sb.st_size);// Copio bytes del origen al destino
    fchmod(fd_destino, sb.st_mode); //copio los permisos. Ejemplo: si origen es solo lectura entonces destino tambien

    munmap(destino, sb.st_size);
    munmap(origen, sb.st_size);
    close(fd_destino);
    close(fd_origen);

    return 0;
}
