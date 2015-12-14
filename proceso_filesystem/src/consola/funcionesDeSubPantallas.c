/*
 * funcionesDeSubPantallas.c
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/
#include "headers/funcionesDeSubPantallas.h"
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LONGITUD_DE_ENTRADA_POR_TECLADO 2000

/*FUNCIONES DE SUBPANTALLA:*/
void mostrarHeaderDeLaConsola(void){
	printf("______COMANDOS DISPONIBLES______\n\n");
	printf("Presione 'z' para regresar al menu principal\n");
	printf("Presione 'q' para cerrar la consola\n\n");
}

char** solicitarInstruccionAlUsuario(void){
	//char instruccionIngresada[LONGITUD_DE_ENTRADA_POR_TECLADO];
	//char delimitador[]= " ";

	char * instruccionIngresada = malloc(LONGITUD_DE_ENTRADA_POR_TECLADO * sizeof(char));
	char * delimitador = " ";
	//fflus(0); esta funcion libera el buffer.
	printf("Ingrese un comando para ejecutarlo:\n");
	scanf("%[ a-zA-Z/.-_]", instruccionIngresada);

	char** instruccion;
	instruccion = string_split(instruccionIngresada, delimitador);
	return instruccion;
}

///////////////////////////////////////////////////////////
char* getMemoriaParaParametro(char * parametroX){
	return (char *)malloc(sizeof(char) * strlen(parametroX) + 1);
}
///////////////////////////////////////////////////////////

char getComando(char ** instruccion){
	return instruccion[0][0];
}

char* getParametro1(char ** instruccion){
	return instruccion[1];
}

char* getParametro2(char ** instruccion){
	return instruccion[2];
}

char* getParametro3(char ** instruccion){
	return instruccion[3];
}

char* getParametro4(char ** instruccion){
	return instruccion[4];
}
///////////////////////////////////////////////////////////
void liberar4Parametros(char * p1, char * p2, char * p3, char * p4){
	free(p1);
	free(p2);
	free(p3);
	free(p4);
}

void liberar3Parametros(char * p1, char * p2, char * p3){
	free(p1);
	free(p2);
	free(p3);
}

void liberar2Parametros(char * p1, char * p2){
	free(p1);
	free(p2);
}

void liberar1Parametro(char * p1){
	free(p1);
}

void liberarInstruccion(char ** instruccion){
	//string_iterate_lines(instruccion, free);
	free(instruccion);
}
/*FIN DE FUNCIONES DE SUBPANTALLA:*/
