/*
 ============================================================================
 Name        : Proceso_Nodo.c
 Author      : Juan Cruz Reines
 Version     :
 Copyright   : Your copyright notice
 Description : Proceso Nodo - Tp Sistemas operativos 1c2015
 ============================================================================
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
#include <sys/socket.h>/*biblioteca que contiene las funciones para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> /*biblioteca que contiene la funcion memset()*/
#include <pthread.h>
#include <errno.h>

#include "Configuracion.h"
#include "ManejoDeSockets.h"
#include "ManejoSolicitudes.h"
#include "Interfaz_Nodo-Job.h"
#include "LoggerNodo.h"

t_list* reducePendientes;
t_log* logNodo;

int main(int argc, char* argv[])
{

	t_config* configuracion = config_create("../Archivos/Nodo_Prueba.cfg");
	if(configuracion == NULL)
		terminarProceso("No se pudo crear la configuracion");
	//Asigna las variables de tipo entero
	int puerto_FS = obtener_int(configuracion, "PUERTO_FS");
	int puerto_NODO = obtener_int (configuracion, "PUERTO_NODO");

	//Asigna las variables de tipo char*
	char* aux = obtener_string(configuracion, "NODO_NUEVO");
	estadoNodo estado;
	//Paso el estado del nodo a su correspondiende enumerativo
	if(strcmp(aux, "SI") == 0)
		estado = NodoNuevo;
	else
		estado = NodoNoNuevo;

	char* nombreEspacioDatos = obtener_string(configuracion, "ARCHIVO_BIN");
	char* directorio_temporal = obtener_string(configuracion, "DIR_TEMP");
	char* espacio_datos = string_new();
	string_append(&espacio_datos, "../Archivos/");
	string_append(&espacio_datos, nombreEspacioDatos);

	//Asigna las variables  de tipo in_addr
	char* ip_FS = obtener_string(configuracion, "IP_FS");
	char* ip_NODO = obtener_string(configuracion, "IP_NODO");


	//Creo el logger del nodo

	logNodo = log_create("../Archivos/LoggerNodo.txt", "Proceso Nodo", false, LOG_LEVEL_INFO);

	logearYMostrarParametrosDeConfiguracion(ip_FS, ip_NODO, espacio_datos, directorio_temporal, puerto_FS, puerto_NODO, estado);

	//creo la lista global de reduce definida en interfaz nodo job
	reducePendientes = list_create();

	//Crea y configura los sockets para la comunicacion

	int socketFd, socketNodo;

	struct sockaddr_in myAddr, fileSystemAddr;


	if((socketNodo = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
			perror("socket");
			exit(1);
	}

	if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
			perror("socket");
			exit(1);
	}
	//Inicializa la estructura de direccion del nodo
	myAddr = inicializarDireccion(puerto_NODO, ip_NODO);

	//Inicializa la estructura de direccion del file system
	fileSystemAddr = inicializarDireccion(puerto_FS, ip_FS);

	//Conecto al socket al FileSystem para notificar conexion del nodo
	if(connect(socketFd, (struct sockaddr *)&fileSystemAddr, sizeof(struct sockaddr_in)) == -1)
		{
			perror("connect");
			exit(1);
		}

	printf("Conexion con file system establecida");
	printf("\n");
	printf("\n");

	//Creo el mensaje de notificacion al file system sobre la conexion del nodo

	t_header cabecera;

	cabecera.tipoMensaje = ConexionNodo;
	cabecera.tamanioMensaje = sizeof(estadoNodo)+ sizeof(long) + sizeof(int);

	void* mensaje = malloc(sizeof(t_header) + cabecera.tamanioMensaje);

	packHeader(cabecera, mensaje);
	//Si el nodo es nuevo o no
	memcpy(mensaje + sizeof(t_header),(estadoNodo*) &estado, sizeof(estadoNodo));
	long tamanioEspacioDeDatos = tamanioArchivo(espacio_datos);
	//el tamaño del archivo del espacio de datos
	memcpy(mensaje + sizeof(t_header) + sizeof(estadoNodo),(long*) &tamanioEspacioDeDatos, sizeof(long));
	//el puerto en donde va a estar escuchando el nodo
	memcpy(mensaje + sizeof(t_header) + sizeof(estadoNodo) + sizeof(long), (int*)&myAddr.sin_port, sizeof(int));

	//Envio el mensaje al file system

	sendAll(socketFd, mensaje, sizeof(t_header) + cabecera.tamanioMensaje);

	close(socketFd);

	printf("Notificacion de conexion de nodo enviado al file system");
	printf("\n");
	printf("\n");

	//Quedo a la espera de solicitudes de otros procesos

	//Ligo el socket al puerto pasado por el archivo de configuracion
	if( (bind(socketNodo, (struct sockaddr*)&myAddr, (socklen_t) sizeof(struct sockaddr)) == -1))
	{
		perror("bind");
		exit(1);
	}

	//Dejo al socket escuchando nuevas conexiones
	if ( listen(socketNodo, 10) == -1)
	{
		perror("listen");
		exit(1);
	}

	//Defino la estructura para las conexiones entrantes
	t_connection newConnection;

	int socklen = sizeof(struct sockaddr);

	while(1)
	{
		//Acepto las solicitudes entrantes
		newConnection.connectionSocket = accept(socketNodo, (struct sockaddr*) &newConnection.connectionAddr,(socklen_t*) &socklen);

		t_data* data = malloc(sizeof(t_data));

		data->connection = newConnection;
		data->directorioTemporal = directorio_temporal;
		data->espacioDatos = espacio_datos;
		data->fsPuerto = puerto_FS;
		data->miPuerto = puerto_NODO;
		inet_aton(ip_FS, &(data->fsIp));
		inet_aton(ip_NODO, &(data->miIp));


		//Defino el  hilo que se encargara de la solicitud
		pthread_t newThread;

		//Creo el hilo nuevo
		pthread_create( &newThread, NULL, (void*)&receiveRequest, (void*) data);

	}

	//config_destroy(configuracion);

	return 0;
}
