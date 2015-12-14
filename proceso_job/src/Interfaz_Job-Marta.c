/*
 * Interfaz_Job-Marta.c
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */


#include "Comunes_Job.h"
#include "Interfaz_Job-Marta.h"

void notificarMarta(struct in_addr ipMarta, int puertoMarta, t_mensaje tipoDeMensaje, t_log* logger, t_log* loggersin)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in direccionMarta;

	direccionMarta.sin_addr = ipMarta;
	direccionMarta.sin_family = AF_INET;
	direccionMarta.sin_port = puertoMarta;
	memset(&direccionMarta.sin_zero, '\0', 8);

	t_header cabecera;

	cabecera.tipoMensaje = tipoDeMensaje;
	cabecera.tamanioMensaje = sizeof(t_header);

	void* mensaje = malloc(sizeof(t_header));

	packHeader(cabecera, mensaje);


	if(connect(sock, (struct sockaddr*) &direccionMarta, sizeof(struct sockaddr_in)) == -1)
	{
		log_error(logger, "connect notificacion a MaRTA");
		exit(1);
	}

	sendAll(sock, mensaje, cabecera.tamanioMensaje);
	jlog_Cabecera(loggersin, "Marta", cabecera.tipoMensaje);


}

int buscarNodo(t_list* listaDeNodos, struct in_addr ip, int puerto)
{
	nodo_t* nodo;
	int index = 0;
	char* ipBuscada = string_new();
	string_append(&ipBuscada, inet_ntoa(ip));

	while(index < list_size(listaDeNodos))
	{
		nodo = list_get(listaDeNodos, index);
		if(strcmp(ipBuscada, inet_ntoa(nodo->ip)) == 0)
		{
			if(puerto == nodo->puerto)
				return index;
		}
		index++;
	}

	return -1;
}

int recibirBloquesAMapear(int sock, t_list* listaDeNodos)
{
	nodo_t nodo;
	nodo_t* nodoAuxiliar = NULL;
	bloque_t bloque;
	int cantidadBloquesAMapear;
	int largoNombreArchivo;
	int index;
	int sockBloque;
	struct sockaddr_in direccionNodo;

	direccionNodo.sin_family = AF_INET;
	memset(&direccionNodo.sin_zero, '\0', 8);

	recv(sock, (int*) & cantidadBloquesAMapear, sizeof(int), 0);

	int i;
	for(i = 0; i < cantidadBloquesAMapear; i++)
	{
		recv(sock, (struct in_addr*) &nodo.ip, sizeof(struct in_addr), 0);
		recv(sock, (int*) & nodo.puerto, sizeof(int), 0);
		recv(sock, (int*) &bloque.numeroBloque, sizeof(int), 0);
		recv(sock, (int*) & largoNombreArchivo, sizeof (int), 0);
		bloque.archivoResultado = malloc(largoNombreArchivo);
		recv(sock, bloque.archivoResultado, sizeof(char) * largoNombreArchivo, 0);

		bloque.estado = SinMapear;

		sockBloque = socket(AF_INET, SOCK_STREAM, 0);
		if(sockBloque == -1)
		{
			perror("socket nodo");
			return -1;
		}

		direccionNodo.sin_addr = nodo.ip;
		direccionNodo.sin_port = nodo.puerto;

		if(connect(sockBloque, (struct sockaddr*) &direccionNodo, sizeof(struct sockaddr_in)) == -1)
		{
			perror("connect nodo");
			return -1;
		}

		bloque.socket = sockBloque;

		index = buscarNodo(listaDeNodos, nodo.ip, nodo.puerto);

		if(index >= 0)
		{
			nodoAuxiliar = list_get(listaDeNodos, index);
			list_add(nodoAuxiliar->bloquesAOperar, &bloque);
			list_replace(listaDeNodos, index, nodoAuxiliar);
		}
		else
		{
			nodo.bloquesAOperar = list_create();
			list_add(nodo.bloquesAOperar, &bloque);
		}

	}

	return 0;
}

int enviarAMartaDatosDelJob(datosJob_t job, t_log* logger, t_log* loggersin)
{
	int sockMarta = socket(AF_INET, SOCK_STREAM, 0);
	if( sockMarta == -1)
	{
		log_error(logger,"socket marta");
		exit(1);
	}

	struct sockaddr_in direccionMarta;

	direccionMarta.sin_addr = job.ipMarta;
	direccionMarta.sin_family = AF_INET;
	direccionMarta.sin_port = htons(job.puertoMarta);
	memset(&direccionMarta.sin_zero, '\0', 8);

	int tamanio = sizeof(struct sockaddr);

	t_header cabecera;

	cabecera.tipoMensaje = datosJob;
	cabecera.tamanioMensaje = sizeof(t_header) + sizeof(int) + sizeof(combiner_t);

	void* mensaje = malloc(cabecera.tamanioMensaje);

	int cantidadArchivosAOperar = list_size(job.archivosAOperar) + 1;

	char* nombreArchivo;
	int largoNombreArchivo;

	long desplazamiento = sizeof(t_header);

	memcpy(mensaje + desplazamiento, (int*) &cantidadArchivosAOperar, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	int i;
	for(i = 0; i < (cantidadArchivosAOperar -1); i++)
	{
		nombreArchivo = list_get(job.archivosAOperar, i);
		largoNombreArchivo = strlen(nombreArchivo) + 1;

		cabecera.tamanioMensaje = cabecera.tamanioMensaje + sizeof(int) + (largoNombreArchivo * sizeof(char));

		mensaje = realloc(mensaje, cabecera.tamanioMensaje);

		memcpy(mensaje + desplazamiento, (int*) &largoNombreArchivo, sizeof(int));
		desplazamiento = desplazamiento + sizeof(int);

		memcpy(mensaje + desplazamiento, nombreArchivo, largoNombreArchivo * sizeof(char));
		desplazamiento = desplazamiento + (largoNombreArchivo * sizeof(char));
	}

	nombreArchivo = job.archivoResultado;
	largoNombreArchivo = strlen(nombreArchivo) + 1;

	cabecera.tamanioMensaje = cabecera.tamanioMensaje + sizeof(int) + (largoNombreArchivo * sizeof(char));

	mensaje = realloc(mensaje, cabecera.tamanioMensaje);

	memcpy(mensaje + desplazamiento, (int*) &largoNombreArchivo, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	memcpy(mensaje + desplazamiento, nombreArchivo, largoNombreArchivo * sizeof(char));
	desplazamiento = desplazamiento + (largoNombreArchivo * sizeof(char));

	memcpy(mensaje+ desplazamiento, (combiner_t*) &job.combiner, sizeof(combiner_t));

	packHeader(cabecera, mensaje);

	if(connect(sockMarta, (struct sockaddr*) &direccionMarta,(socklen_t) tamanio) == -1)
	{
		perror("connect marta");
		log_error(logger, "connect marta");
		exit(1);
	}
	jlog_Conexion(logger, "Marta", inet_ntoa(job.ipMarta), job.puertoMarta);

	sendAll(sockMarta, mensaje, cabecera.tamanioMensaje);
	jlog_Cabecera(loggersin, "Marta", cabecera.tipoMensaje);

	return sockMarta;
}
