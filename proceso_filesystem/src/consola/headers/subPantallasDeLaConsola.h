/*
 * subPantallasDeLaConsola.h
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/

#ifndef SUB_PANTALLAS_DE_LA_CONSOLA_H_
#define SUB_PANTALLAS_DE_LA_CONSOLA_H_

 	//DEFINICION DE PROTOTIPOS
 	int main (void);

 	void liberarParametros(char *, char *);
 	void liberarParametro1(char *);
	void liberarInstruccion(char **);

 	void comandosDeDirectorio(void);
 	void comandosDeArchivoLocal(void);
 	void comandosDelMDFS(void);
 	void comandosDeBloque(void);
 	void comandosDeNodo(void);

 	//DEFINICION DE TIPOS de comando
 	typedef enum {
		CREAR_DIRECTORIO = 'c',
		RENOMBRAR_DIRECTORIO = 'r',
		ELIMINAR_DIRECTORIO = 'e',
		MOVER_DIRECTORIO = 'm',
		IR_A_MENU_PRINCIPAL_D ='z',
		CERRAR_CONSOLA_D = 'q'
	} t_cmdDeDirectorio;

	typedef enum {
		MOVER_ARCHIVO = 'm',
		RENOMBRAR_ARCHIVO = 'r',
		ELIMINAR_ARCHIVO = 'e',
		COPIAR_AL_MDFS = 'c',
		IR_A_MENU_PRINCIPAL_ARC ='z',
		CERRAR_CONSOLA_ARC = 'q'
	} t_cmdDeArchivoLocal;

	typedef enum {
		FORMATEAR_MDFS = 'f',
		COPIAR_AL_FS = 'c',
		OBTENER_MD5 = 'o',
		IR_A_MENU_PRINCIPAL_M ='z',
		CERRAR_CONSOLA_M = 'q'
	} t_cmdDeMDFS;

	typedef enum {
		VER_BLOQUES_DE = 'v',
		COPIAR_BLOQUES_DE = 'c',
		BORRAR_BLOQUES_DE = 'b',
		IR_A_MENU_PRINCIPAL_B ='z',
		CERRAR_CONSOLA_B = 'q'
	} t_cmdDeBloque;

	typedef enum {
		AGREGAR_NODO = 'a',
		ELIMINAR_NODO = 'e',
		IR_A_MENU_PRINCIPAL_N ='z',
		CERRAR_CONSOLA_N = 'q'
	} t_cmdDeNodo;

#endif
