/*
 * comunes_job.h
 *
 *  Created on: 27/6/2015
 *      Author: utnso
 */

#ifndef COMUNES_JOB_H_
#define COMUNES_JOB_H_

//Common libraries
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

//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h> /*biblioteca que contiene las funciones para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>  /*biblioteca que contiene la funcion memset()*/
#include <sys/stat.h>
#include <fcntl.h> /*biblioteca que contiene la funcion open()*/
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h> /*biblioteca que contiene la funcion mmap() y munmap()*/

#include "Log_Job.h"

#define BACKLOG 10

typedef enum{NO, SI} t_combiner;

typedef struct {
	char* ipMarta; /*<Dir ip de la computadora que ejecuta MaRTA*/
	int puertoMarta; /*<Puerto donde escucha MaRTA*/
	char* mapper; /*<Dirección a rutina de mapping*/
	char* reduce; /*<Dirección a rutina de reducer*/
	t_combiner combiner; /*<Especifica si soporta combiner*/
	t_list* archivos;/*<Lista de rutas de archivos del job*/
	char* resultado;/*<Ruta a archivo de resultados*/
}t_job;


void comAbortarProceso(char* MensajeError);
int comTamanioArchivo(char* archivo);

#endif /* COMUNES_JOB_H_ */
