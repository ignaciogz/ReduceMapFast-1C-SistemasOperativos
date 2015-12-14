/*
 * comunes_job.c
 *
 *  Created on: 28/6/2015
 *      Author: utnso
 */

#include "Comunes_Job.h"

void comAbortarProceso(char* MensajeError)
{
	error_show(MensajeError);
	exit(0);
}

int comTamanioArchivo(char* archivo)
{
	long int tamanio;
	FILE* fileDescriptor = fopen(archivo, "r");
	if(fileDescriptor != NULL){
		fseek(fileDescriptor, 0L, SEEK_END);
		tamanio = ftell(fileDescriptor);
		fclose(fileDescriptor);
		return tamanio;
	}
	else{
		fprintf(stderr, "%s no pudo ser abierto.", archivo);
	    return -1;
	}
}
