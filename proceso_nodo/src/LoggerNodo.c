/*
 * LoggerNodo.c
 *
 *  Created on: 7/7/2015
 *      Author: utnso
 */

#include "LoggerNodo.h"
#include <commons/string.h>
#include <pthread.h>
#include <stdlib.h>

pthread_mutex_t loggerMutex = PTHREAD_MUTEX_INITIALIZER;

void logearLecturaDeBloque(int numeroBloque)
{
	char* message = string_new();
	char* numBloque = string_itoa(numeroBloque);

	string_append(&message, "Lectura del bloque: ");
	string_append(&message, numBloque);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);

	pthread_mutex_unlock( &loggerMutex );

	free(numBloque);
	free(message);

	return;
}

void logearEscrituraDeBloque(int numeroBloque)
{
	char* message = string_new();
	char* numBloque = string_itoa(numeroBloque);

	string_append(&message, "Escritura del bloque: ");
	string_append(&message, numBloque);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	printf("%s", message);
	printf("\n");

	log_info(logNodo, message);

	pthread_mutex_unlock( &loggerMutex );

	free(numBloque);
	free(message);

	return;
}

void logearLecturaDeArchivoTermporal(char* nombreArchivo)
{
	char* message = string_new();

	string_append(&message, "Lectura del archivo temporal: ");
	string_append(&message, nombreArchivo);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearEscrituraDeArchivoTemporal(char* nombreArchivo)
{
	char* message = string_new();

	string_append(&message, "Escritura del archivo temporal: ");
	string_append(&message, nombreArchivo);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearYMostrarParametrosDeConfiguracion(char* ipFs, char* ipNodo, char* espacioDeDatos, char* directorioTemporal, int puertoFs, int puertoNodo, estadoNodo nodoNuevo)
{
	char* message = string_new();
	char* strPuertoFs =  string_itoa(puertoFs);
	char* strPuertoNodo =  string_itoa(puertoNodo);

	pthread_mutex_lock( &loggerMutex );

	printf("=====================================================================\n");
	printf("               Valores de configuracion del nodo\n");
	printf("=====================================================================\n");

	string_append(&message, "La ip de este nodo es: ");
	string_append(&message, ipNodo);
	string_append(&message, "\n");
	log_info(logNodo, message);
	printf("%s",message);
	printf("---------------------------------------------------------------------\n");
	free(message);

	message = string_new();
	string_append(&message, "El puerto donde este nodo esta escuchando es: ");
	string_append(&message, strPuertoNodo);
	string_append(&message, "\n");
	log_info(logNodo, message);
	printf("%s",message);
	printf("---------------------------------------------------------------------\n");
	free(message);

	if(nodoNuevo == NodoNuevo)
	{
		message = string_new();
		string_append(&message, "Este nodo es nuevo");
		string_append(&message, "\n");
		log_info(logNodo, message);
		printf("%s",message);
		printf("---------------------------------------------------------------------\n");
		free(message);
	}
	else
	{
		message = string_new();
		string_append(&message, "Este nodo no es nuevo");
		string_append(&message, "\n");
		log_info(logNodo, message);
		printf("%s",message);
		printf("---------------------------------------------------------------------\n");
		free(message);
	}

	message = string_new();
	string_append(&message, "La ip del file system es : ");
	string_append(&message, ipFs);
	string_append(&message, "\n");;
	log_info(logNodo, message);
	printf("%s",message);
	printf("---------------------------------------------------------------------\n");
	free(message);

	message = string_new();
	string_append(&message, "El puerto donde el file system esta escuchando es: ");
	string_append(&message, strPuertoFs);
	string_append(&message, "\n");
	log_info(logNodo, message);
	printf("%s",message);
	printf("---------------------------------------------------------------------\n");
	free(message);

	message = string_new();
	string_append(&message, "El espacio de datos de este nodo es ");
	string_append(&message, espacioDeDatos);
	string_append(&message, "\n");
	log_info(logNodo, message);
	printf("%s",message);
	printf("---------------------------------------------------------------------\n");
	free(message);

	message = string_new();
	string_append(&message, "El directorio temporal de este nodo es ");
	string_append(&message, directorioTemporal);
	string_append(&message, "\n");
	log_info(logNodo, message);
	printf("%s",message);
	printf("---------------------------------------------------------------------\n");
	free(message);

	printf("\n");
	printf("\n");

	pthread_mutex_unlock( &loggerMutex );

	free(strPuertoFs);
	free(strPuertoNodo);

	return;
}

void logearYMostrarConexion(int puerto, char* ip, char* proceso)
{
	char* message = string_new();
	char* strPuerto = string_itoa(puerto);

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Conexion de proceso: ");
	string_append(&message, proceso);
	string_append(&message, " con ip: ");
	string_append(&message, ip);
	string_append(&message, " y puerto: ");
	string_append(&message, strPuerto);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);
	printf("%s",message);

	pthread_mutex_unlock( &loggerMutex );

	free(strPuerto);
	free(message);

	return;
}

void logearYMostrarSolicitudMap(char* ip)
{
	char* message = string_new();

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Solicitud de map de job con ip: ");
	string_append(&message, ip);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);
	printf("%s",message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearYMostrarSolicitudReduce(char* ip)
{
	char* message = string_new();

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Solicitud de reduce de job con ip: ");
	string_append(&message, ip);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);
	printf("%s",message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearYMostrarFinalizacionDeMap(char* ip)
{
	char* message = string_new();

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Finalizacion de map de job con ip: ");
	string_append(&message, ip);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);
	printf("%s",message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearYMostrarFinalizacionDeReduce(char* ip)
{
	char* message = string_new();

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Finalizacion de reduce de job con ip: ");
	string_append(&message, ip);
	string_append(&message, "\n");

	printf("%s",message);

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearYMostrarSolicitudReduceLocal(char* ip)
{
	char* message = string_new();

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Solicitud de reduce local pedida por el nodo con ip: ");
	string_append(&message, ip);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);
	printf("%s",message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearYMostrarFinalizacionDeReduceLocal(char* ip)
{
	char* message = string_new();

	printf("---------------------------------------------------------------------\n");
	string_append(&message, "Finalizacion de reduce local pedido por el nodo con ip: ");
	string_append(&message, ip);
	string_append(&message, "\n");

	printf("%s",message);

	pthread_mutex_lock( &loggerMutex );

	log_info(logNodo, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

return;
}
