/*
 * consola.h
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/

#ifndef CONSOLA_H_
#define CONSOLA_H_

	typedef enum {
		Cerrar_Consola= 0,
		DIRECTORIO,
		ARCHIVOLOCAL,
		MDFS,
		BLOQUE,
		NODO
	} t_comando;

	//prototipos:
	void listarComandos(void);
	void* consola (void);

#endif
