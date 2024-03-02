#include "defs.h"
#include "readline.h"
#include "history.h"
#include "utils.h"
#include "nonCanonical.h"
static char buffer[BUFLEN];

//------------------------------------visual-------------------------------------------//

static void clear_character(void){
	write(STDOUT_FILENO, "\b \b", 3);
}
static void clear_terminal(int largo_linea){
	for (int i = 0; i < largo_linea; i++) {
		clear_character();
	}
}
static void show_in_terminal(char* linea){
	printf("%s", linea);
}
//------------------------------------buffer------------------------------------------//

static void limpiar_buffer(){
	memset(buffer,0,sizeof(buffer));
}

static int actualizar_buffer(char* linea_a_poner){
	limpiar_buffer();
	memcpy(buffer,linea_a_poner, strlen(linea_a_poner));
	return strlen(linea_a_poner);
}
//-------------------------------desplazamientos-------------------------------------//

static int desplazar(char* comando, int size_linea_actual){
	clear_terminal(size_linea_actual);
	show_in_terminal(comando);
	size_linea_actual = actualizar_buffer(comando);
	
	return size_linea_actual;
}

//-----------------------------------------------------------------------------------//

static void manejar_desplazamiento(int* c, int* i, int* cant_comandos_mostrados,char** comandos, int cant_comandos){
	if(!comandos || cant_comandos == 0 ){
		limpiar_buffer();
		*i = 0;
		buffer[0] = '\0';
		return;
	}
	if (*c == UP){
		if(cant_comandos > 0){ //tengo cosas para mostrar
			if(*cant_comandos_mostrados == cant_comandos){// si ya mostre todos
				*cant_comandos_mostrados = 0; //seteo el contador para comenzar denuevo
			}
			(*cant_comandos_mostrados)++;
			*i = desplazar(comandos[cant_comandos - (*cant_comandos_mostrados)], *i);
		}
	} else if (*c == DOWN){
		if(cant_comandos && (*cant_comandos_mostrados) > 1){ //tengo cosas para mostrar
			(*cant_comandos_mostrados)--;
			*i = desplazar(comandos[cant_comandos - (*cant_comandos_mostrados)],*i);
		} else {
			clear_terminal(*i);
			*i = 0;
			buffer[0] = '\0';
		}
	}		
}
static int es_imprimible(int c){
	return (c > 31? 1: 0);
}
//-----------------------------------------------------------------------------------//

// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *promt)
{
	int i = 0, c = 0;

#ifndef SHELL_NO_INTERACTIVE
	fprintf(stdout, "%s %s %s\n", COLOR_RED, promt, COLOR_RESET);
	fprintf(stdout, "%s", "$ ");
#endif
	memset(buffer, 0, BUFLEN);
	
	char** comandos = NULL;
	int cant_comandos = 0;
	//Abro historial y me guardo todos los comandos para ir mostrandolos;
	char* path = path_history();
	FILE* history = fopen(path, "r");
	if(history){
		cant_comandos = count_lines(history);
		rewind(history);
		comandos = calloc(cant_comandos, sizeof(char*));
		for(int i = 0; i < cant_comandos; i++){
			comandos[i] = calloc(1, BUFLEN *sizeof(char));
		}
		get_lines(comandos, history);
		//comandos tiene todas las lineas.
		fclose(history);
	}
	free(path);	
	
	int cant_comandos_mostrados = 0;

	set_input_mode();	// NONCANONICAL

	c = getchar();
	while (c != END_LINE && c != VEOF) {
		if(c == ESCAPE){
			c = getchar();
			if(c == '['){
				c = getchar();
				manejar_desplazamiento(&c, &i, &cant_comandos_mostrados, comandos, cant_comandos);
			}
			//Se permitiran solo los desplazamientos para arriba y abajo.
			//otros escape codes seran ignorados.
		} else if (c == BACKSPACE){
			if(i != 0){
				buffer[i] = '\0';
				i--;
				buffer[i] = '\0';
				clear_character();
			}
		} else {
			if(es_imprimible(c)){
				write(STDOUT_FILENO,&c,1);
				buffer[i++] = c;
			}
		}
		c = getchar();
	}
	for (int i = 0; i < cant_comandos; i++) {
		free(comandos[i]);
	}
	free(comandos);
	
	// if the user press ctrl+D
	// just exit normally
	if (c == VEOF){
		return NULL;
	}

	buffer[i] = END_STRING;

	return buffer;
}
