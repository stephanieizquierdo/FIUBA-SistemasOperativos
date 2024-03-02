#ifndef HISTORY
#define HISTORY

#include "defs.h"
#include <limits.h>

/*
 * Devuelve la ruta hacia el archivo donde se encuentra el historial de comandos ejecutados.
 * Esta funcion aloca memoria. Debera posteriormente hacer free de lo retornado por esta funcion.
 */
char* path_history(void);

/*
 * El comando se guardara en el archivo de historial de comandos
 */
void save_history(char* cmd);

/*
 * Muestra el historial apartir del comando enecimo. Si n es ALL_HISTORY(-1) muestra todos.
 */
void show_history(FILE* history_file, int n);

/*
* Devuelve el comando a ejecutar segun el event designator.
*/
char* find_command(FILE* history_file, int n);

/*
 * Llena el amacenador con todas las lineas de history_file
 */
void get_lines(char** almacenador, FILE* history_file);

#endif /*HISTORY*/