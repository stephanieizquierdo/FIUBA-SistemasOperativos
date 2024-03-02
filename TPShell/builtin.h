#ifndef BUILTIN_H
#define BUILTIN_H

#include "defs.h"

extern char promt[PRMTLEN];

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);


int history(char* cmd);

/* Esta funcion aloca memoria para el puntero devuelto.
 * 
 * Dado el comando se fija si es un event designator.
 * En caso de que lo sea devuelve el comando al cual hace referencia. sino devuelve NULL.
 */
char* event_designators(char *cmd, int* debo_guardar);

#endif  // BUILTIN_H