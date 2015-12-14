/*
 * ManejoMensajesJob.c
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */


#include "Comunes_Job.h"
#include "ManejoMensajesJob.h"
#include "Interfaz_Job-Marta.h"
#include "Interfaz_Job-Nodo.h"

int agregarSocketsASet(fd_set* set, t_list* listaDeNodos)
{
	int maxfd = 0;
	nodo_t* nodo;
	bloque_t* bloque;
	int i,j;
	for(i = 0; i < list_size(listaDeNodos); i++)
		nodo = list_get(listaDeNodos, i);
	{
		for(j = 0; j > list_size(nodo->bloquesAOperar); j++)
		{
			bloque= list_get(nodo->bloquesAOperar, j);
			FD_SET(bloque->socket, set);
			if(bloque->socket > maxfd)
				maxfd = bloque->socket;
		}

	}

	return maxfd;
}

void enviarPedidosdeMap(envioMap_t datosMap)
{
	nodo_t* nodo;

	int cantidadNodos = list_size(datosMap.listaDeNodos);

	int i;
	for(i = 0; i < cantidadNodos; i++)
	{
		nodo = list_get(datosMap.listaDeNodos, i);
		enviarSolicitudDeMap(*nodo, datosMap.archivoMaper);
	}

	return;
}

int manejarSolicitudes(datoSolicitud_t solicitud, t_log* logger, t_log* loggersin)
{
	struct in_addr ipNodoReducer;
	int puertoNodoReducer;
	int socketNodoReducer;
	envioMap_t datosMap;

	switch(solicitud.cabecera.tipoMensaje)
	{
		case pedidoMapping: recibirBloquesAMapear(solicitud.sock, solicitud.listaDeNodos);
							datosMap.archivoMaper = solicitud.job.archivoMap;
							datosMap.listaDeNodos = solicitud.listaDeNodos;
							enviarPedidosdeMap(datosMap);
							break;

		case FinalizacionDeMap: notificarMarta(solicitud.job.ipMarta, solicitud.job.puertoMarta, operacionMapExitosa, logger, loggersin);
							break;

		case pedidoReduce:	recv(solicitud.sock, (struct in_addr*) &ipNodoReducer, sizeof(struct in_addr), 0);
							recv(solicitud.sock, (int*) &puertoNodoReducer, sizeof(int), 0);
							socketNodoReducer = enviarSolicitudDeReduce(solicitud.listaDeNodos, solicitud.job.combiner, ipNodoReducer, puertoNodoReducer, solicitud.job.archivoReduce, solicitud.job.archivoResultado);
							return socketNodoReducer;
							break;

		case FinalizacionDeReduce: notificarMarta(solicitud.job.ipMarta, solicitud.job.puertoMarta, operacionReduceExitosa, logger, loggersin);
		break;

	}

	return 0;
}

void packHeader(t_header header, void* message)
{
	int offset;

	memcpy(message, (t_mensaje*) &header.tipoMensaje, sizeof(t_mensaje));//copio el tipo de mansaje al mensaje
	offset = sizeof(t_mensaje); //aumento el desplazamiento

	memcpy(message + offset, (int*) &header.tamanioMensaje, sizeof(int));//copio la longitud del mensaje (sin contar la cabecera) al mensaje

	return;
}

void sendAll(int sendSocket, void* message, int length)
{
	int offset = 0; // desplazamiento
	int sent = 0;	//cantidad de bytes enviados

	do{

		do{
			sent = send(sendSocket, message + offset, length, 0); //se envia el mensaje y se registra la cantidad de bytes enviados
			//perror("Error:");
		}while(sent < 0); //mientras no se mande nada seguir intentando.

	offset = offset + sent; //se le suma al desplazamiento la cantidad de bytes enviados
	length = length - sent; //se le resta a la longitud la cantidad de bytes enviados

	}while(length != 0 ); //cuando la longitud sea 0 significa que se envio el mensaje entero

	return;
}

long tamanioArchivo(char* archivo)
{
	long tamanio;

	FILE* fileDescriptor = fopen(archivo, "r"); //se abre el archivo solicitado para lectura


	fseek(fileDescriptor, 0, SEEK_END); //se posiciona el cursor del archivo sobre el final

	tamanio = ftell(fileDescriptor); //se devuelve la posicion en bytes en la que esta posicionado el cursor

	fclose(fileDescriptor);

	return tamanio;
}

void getFileContent (char* directorioArchivo, char* contenido)
{

	FILE* fileDescriptor = fopen(directorioArchivo, "r"); //se abre el archivo solicitado para lectura
	long tamanioArchivo;

	fseek(fileDescriptor, 0, SEEK_END); //se posiciona el cursor del archivo sobre el final
	tamanioArchivo = ftell(fileDescriptor); //se devuelve la posicion en bytes en la que esta posicionado el cursor

	contenido = malloc(tamanioArchivo);

	char* aux = (char*) mmap (0, tamanioArchivo, PROT_WRITE, MAP_SHARED, fileDescriptor, 0); // se copia a memoria el contenido del archivo

	fclose (fileDescriptor); // se cierra el archivo solicitado

	memcpy(contenido, aux, tamanioArchivo); //copio el contenido del archivo a la variable contenido

	munmap(aux, tamanioArchivo); //Libero la memoria mapeada

	return;
}

long empaquetarBloqueAReducir(void* mensaje, struct in_addr ip, int puerto, char* archivoAReducir, long desplazamiento)
{
	int largoNombreArchivo = strlen(archivoAReducir) + 1;

	//Empaqueto el ip del nodo
	memcpy(mensaje + desplazamiento , (struct in_addr*) &ip, sizeof(struct in_addr));
	desplazamiento = desplazamiento + sizeof(struct in_addr);

	//Empaqueto el puerto del nodo
	memcpy(mensaje + desplazamiento , (int*) &puerto, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	//Empaqueto el puerto del nodo
	memcpy(mensaje + desplazamiento , (int*) &largoNombreArchivo, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	//Empaqueto el nombre del archivo a reducir
	memcpy(mensaje + desplazamiento, archivoAReducir, sizeof(char) * largoNombreArchivo);
	desplazamiento = desplazamiento + sizeof(char) * largoNombreArchivo;

	return desplazamiento;
}

void empaquetarDatosFijosDelReduce(void* mensaje, char* archivoReduce, char* scriptReduce, long tamanioScript, char* archivoResultado, int cantidadArchivosAReducir)
{
	long desplazamiento = sizeof(t_header);
	int largoNombreArchivo = strlen(archivoReduce) + 1;

	//Empaqueto el largo del nombre del archivo reduce
	memcpy(mensaje + desplazamiento, (int*) &largoNombreArchivo, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	//Empaqueto el nombre del archivo reduce
	memcpy(mensaje + desplazamiento, archivoReduce, sizeof(char) * largoNombreArchivo);
	desplazamiento = desplazamiento + sizeof(char) * largoNombreArchivo;

	//Empaqueto el largo del contenido del archivo reduce
	memcpy(mensaje + desplazamiento, (long*) &tamanioScript, sizeof(long));
	desplazamiento = desplazamiento + sizeof(long);

	//Empaqueto el contenido del archivo reduce
	memcpy(mensaje + desplazamiento, scriptReduce, sizeof(char) * tamanioScript);
	desplazamiento = desplazamiento + sizeof(char) * tamanioScript;

	largoNombreArchivo = strlen(archivoResultado) + 1;

	//Empaqueto el largo del nombre del archivo reduce
	memcpy(mensaje + desplazamiento, (int*) &largoNombreArchivo, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	//Empaqueto el nombre del archivo reduce
	memcpy(mensaje + desplazamiento, archivoResultado, sizeof(char) * largoNombreArchivo);
	desplazamiento = desplazamiento + sizeof(char) * largoNombreArchivo;

	//Empaqueto la cantidad de archivos a reducir
	memcpy(mensaje + desplazamiento, (int*) &cantidadArchivosAReducir, sizeof(int));



	return;
}
