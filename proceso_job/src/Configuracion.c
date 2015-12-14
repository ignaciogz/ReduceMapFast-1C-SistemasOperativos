/*
 * Configuracion.c
 *
 *  Created on: 28/5/2015
 *      Author: utnso
 */

/*Common libraries */
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/error.h>
#include <commons/log.h>
#include <commons/process.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/txt.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "Configuracion.h"


void terminarProceso(char* MensajeError)
{
	error_show(MensajeError); //Muestra un mensaje de error alertando del problema
	printf("\n");
	exit(0); //Finaliza el proceso
}

int obtener_int(t_config* configuracion, char* clave)
{
	int valor;
	if (config_has_property(configuracion, clave) == true) //Comprueba que el archivo de configuracion contenga la clave solicitada.
	{
		valor = config_get_int_value(configuracion, clave); // se le asigna a la variable valor el valor correspondiente a la clave solicitada
		if (valor == NULL)
			terminarProceso("Error: Archivo de configuracion sin valores definidos.");
	}
	else
		terminarProceso("Error: Archivo de configuracion incompleto.");

	return valor;
}


char* obtener_string(t_config* configuracion, char* clave)
{
	char* valor;
	if (config_has_property(configuracion, clave) == true) //Comprueba que el archivo de configuracion contenga la clave solicitada.
	{
		valor = config_get_string_value(configuracion, clave); // se le asigna a la variable valor el valor correspondiente a la clave solicitada
		if (valor == NULL)
			terminarProceso("Error: Archivo de configuracion sin valores definidos.");
	}
	else
		terminarProceso("Error: Archivo de configuracion incompleto.");

	return valor;
}


