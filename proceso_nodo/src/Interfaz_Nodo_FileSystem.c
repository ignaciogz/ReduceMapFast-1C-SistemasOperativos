/*
 * Interfaz_Nodo-FileSystem.c
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

/*Standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /*biblioteca que contiene la funcion memset()*/
#include <pthread.h> /*biblioteca que contiene las funciones para manejo de hilos*/
#include <sys/socket.h>/*biblioteca que contiene las funciones para manejo de sockets*/
#include <sys/mman.h> /*biblioteca que contiene la funcion mmap() y munmap()*/
#include <unistd.h> /*biblioteca que contiene la funcion sysconf(_SC_PAGE_SIZE) y close()*/
#include <fcntl.h> /*biblioteca que contiene la funcion open()*/
#include <errno.h>
/*bibliotecas necesarias para manejar las ip con inet_aton() y que contienen las structs para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Interfaz_Nodo_FileSystem.h"
#include "LoggerNodo.h"

//Defino el semaforo mutex para controlar las escrituras y lecturas sobre el espacio de datos
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



void getBloque(t_bloque_datos* bloqueDeseado, char* espacio_datos) {

	pthread_mutex_lock( &mutex );

	logearLecturaDeBloque(bloqueDeseado->numero_bloque);

	int fileDescriptor = open(espacio_datos, O_RDONLY); // se abre el espacio de datos para lectura
	char* mapped; // se define la variable sobre la que se mapeara el archivo

	/*se copia el contenido del archivo para el bloque solicitado en memoria*/
	mapped = (char*) mmap(0, tamanio_bloque, PROT_READ, MAP_SHARED,
			fileDescriptor, bloqueDeseado->numero_bloque * tamanio_bloque);

	close(fileDescriptor); // se cierra el espacio de datos
	pthread_mutex_unlock( &mutex );

	memcpy(bloqueDeseado->datos, mapped, tamanio_bloque);/*Se copia la informacion del archivo en la variable datos del bloque que se desea obtener*/

	munmap(mapped, tamanio_bloque); //libero los datos mapeados
	return;
}


void setBloque(t_bloque_datos bloqueACopiar, char* espacio_datos) {

	pthread_mutex_lock( &mutex );

	logearEscrituraDeBloque(bloqueACopiar.numero_bloque);

	char* mapped;
	int fileDescriptor = open(espacio_datos, O_RDWR); // se abre el espacio de datos para escritura

	mapped = (char*) mmap(0, tamanio_bloque, PROT_WRITE, MAP_SHARED,
			fileDescriptor, bloqueACopiar.numero_bloque * tamanio_bloque); //se copa a memoria la porcion de archivo que sera escrito
	memcpy(mapped, bloqueACopiar.datos, tamanio_bloque);// se copia al area mapeada los datos del bloque de datos


	munmap(mapped, tamanio_bloque); //libero los datos mapeados

	close(fileDescriptor); //ciero el file descriptor del archivo
	pthread_mutex_unlock( &mutex );

	return;
}


char* getFileContent (char* nombre_Archivo, char* directorio_temporal)
{
	logearLecturaDeArchivoTermporal( nombre_Archivo);

	char* contenido;

	char* directorioArchivo = string_new();//defino una variable que contendra la ruta del directorio a consultar
	string_append(&directorioArchivo, directorio_temporal);
	string_append(&directorioArchivo, nombre_Archivo); // concateno el directorio del espacio temporal con el nombre del archivo

	long tamanioArchivo;

	FILE* descriptor = fopen(directorioArchivo, "r");

	fseek(descriptor, 0, SEEK_END); //se posiciona el cursor del archivo sobre el final
	tamanioArchivo = ftell(descriptor); //se devuelve la posicion en bytes en la que esta posicionado el cursor

	fclose(descriptor);

	int fileDescriptor = open(directorioArchivo, O_RDWR); //se abre el archivo solicitado para lectura

	contenido = (char*) malloc(tamanioArchivo);

	char* aux = malloc(tamanioArchivo);

	aux = (char*) mmap (0, tamanioArchivo, PROT_WRITE, MAP_SHARED, fileDescriptor, 0); // se copia a memoria el contenido del archivo


	memcpy(contenido, aux, tamanioArchivo); //copio el contenido del archivo a la variable contenido

	munmap(aux, tamanioArchivo); //Libero la memoria mapeada

	close (fileDescriptor); // se cierra el archivo solicitado

	return contenido;
}
