/*
 * funcionesDeSubPantallas.h
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/

#ifndef FUNCIONES_DE_SUBPANTALLAS_H_
#define FUNCIONES_DE_SUBPANTALLAS_H_

	void mostrarHeaderDeLaConsola(void);
	char** solicitarInstruccionAlUsuario(void);
	char getComando(char**);
	char* getParametro1(char**);
	char* getParametro2(char**);
	char* getParametro3(char**);
	char* getParametro4(char**);

	char* getMemoriaParaParametro(char *);

	void liberar4Parametros(char *, char *, char *, char *);
	void liberar3Parametros(char *, char *, char *);
	void liberar2Parametros(char *, char *);
	void liberar1Parametro(char *);
	void liberarInstruccion(char **);

#endif
