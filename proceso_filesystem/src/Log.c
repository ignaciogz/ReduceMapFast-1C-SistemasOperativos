/*
 ============================================================================
 Name        : Log.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include "headers/socketsFs.h"
#include "headers/manejoDeDatos.h"
#include "headers/Log.h"


extern int totalBloquesLibres;
extern t_log* logFs;

pthread_mutex_t loggerMutex = PTHREAD_MUTEX_INITIALIZER;

void logearLecturaDeArchivo(char* nombreArchivo)
{
	char* message;

	message = malloc (strlen("Lectura del archivo : ") + strlen(nombreArchivo) + 2);

	string_append(&message, "Lectura del archivo : ");
	string_append(&message, nombreArchivo);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearEscrituraDeArchivo(char* nombreArchivo)
{
	char* message;

	message = malloc (strlen("Escritura del archivo: ") + strlen(nombreArchivo) + 2);

	string_append(&message, "Escritura del archivo: ");
	string_append(&message, nombreArchivo);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearConexionNodo(t_estado_nodo* conexionNodo)
{	//variables
	char* message;
	int tamanio;

	char*Ip=string_itoa(conexionNodo->datosNodo.connectionAddr.sin_addr.s_addr);
	char*Puerto=string_itoa(conexionNodo->datosNodo.connectionAddr.sin_port);
	char*bloquesLibres = string_itoa((cantBloquesLibres(conexionNodo->estadoBloques)));
	tamanio = bitarray_get_max_bit(conexionNodo->estadoBloques);
	char*totalBloques = string_itoa(tamanio);

	message = malloc (strlen("Conexion de Nodo: ") + strlen(Ip)+ strlen(Puerto)+ strlen("Total de bloques:")+strlen(totalBloques)+strlen("BloquesLibres:")+strlen(bloquesLibres) + 6);

	string_append(&message, "Conexion de Nodo: ");
	string_append(&message, Ip);
	string_append(&message, Puerto);
	string_append(&message, "\n");
	string_append(&message, "Total de bloques:");
	string_append(&message, totalBloques);
	string_append(&message, "\n");
	string_append(&message, "BloquesLibres:");
	string_append(&message, bloquesLibres);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);
	free(Ip);
	free(Puerto);
	free(totalBloques);
	free(bloquesLibres);

	return;
}

void logearDesconexionNodo(t_estado_nodo* conexionNodo)
{	//variables
	char* message;
	int tamanio;

	char*Ip=string_itoa(conexionNodo->datosNodo.connectionAddr.sin_addr.s_addr);
	char*Puerto=string_itoa(conexionNodo->datosNodo.connectionAddr.sin_port);
	char*bloquesLibres = string_itoa((cantBloquesLibres(conexionNodo->estadoBloques)));
	tamanio=bitarray_get_max_bit(conexionNodo->estadoBloques);
	char*totalBloques = string_itoa(tamanio);

	message = malloc (strlen("Desconexion de Nodo: ") + strlen(Ip)+ strlen(Puerto)+ strlen("Total de bloques:")+strlen(totalBloques)+strlen("BloquesLibres:")+strlen(bloquesLibres) + 6);

	string_append(&message, "Desconexion de Nodo: ");
	string_append(&message, Ip);
	string_append(&message, Puerto);
	string_append(&message, "\n");
	string_append(&message, "Total de bloques:");
	string_append(&message, totalBloques);
	string_append(&message, "\n");
	string_append(&message, "BloquesLibres:");
	string_append(&message, bloquesLibres);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);
	free(Ip);
	free(Puerto);
	free(totalBloques);
	free(bloquesLibres);

	return;
}
void logearYMostrarConexion(int puerto, char* ip, char* proceso)
{
	char* message = string_new();
	char* strPuerto = string_itoa(puerto);

	string_append(&message, "Conexion de proceso: ");
	string_append(&message, proceso);
	string_append(&message, " con ip: ");
	string_append(&message, ip);
	string_append(&message, " y puerto: ");
	string_append(&message, strPuerto);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(strPuerto);
	free(message);

	return;
}

void logearYMostrarDesconexion(int puerto, char* ip, char* proceso)
{
	char* message = string_new();
	char* strPuerto = string_itoa(puerto);

	string_append(&message, "Desconexion de proceso: ");
	string_append(&message, proceso);
	string_append(&message, " con ip: ");
	string_append(&message, ip);
	string_append(&message, " y puerto: ");
	string_append(&message, strPuerto);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(strPuerto);
	free(message);

	return;
}



void logearEspacioTotalSistema()
{
	char* message;

	char*bloquesTotales=string_itoa(cantBloquesTotal());
	int espacioLibre = TAMANIO_BLOQUE*totalBloquesLibres;
	char*bloquesLibres=string_itoa(espacioLibre);
	message = malloc (strlen("Espacio Total Sistema: ") + strlen(bloquesTotales)+ strlen("Espacio Libre: ")+strlen(bloquesLibres) + 4);


	string_append(&message, "Espacio Total Sistema: ");
	string_append(&message, bloquesTotales);
	string_append(&message, "\n");
	string_append(&message, "Espacio Libre: ");
	string_append(&message, bloquesLibres);
	string_append(&message, "\n");

	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);

	return;
}

void logearBloqueDeArchivo(char* nomArchivo,t_copiaX* copia)
{	//variables
	char* message;


	char*Ip=string_itoa(copia->IP);
	char*Puerto=string_itoa(copia->puerto);
	char*nroBloque = string_itoa(copia->numBloque);

	message = malloc (strlen("El archivo: ") + strlen(nomArchivo) + strlen(Ip)+ strlen(Puerto)+ strlen("Posee los siguientes bloques: ") + strlen(nroBloque) + 4);

	string_append(&message, "El archivo: ");
	string_append(&message, nomArchivo);
	string_append(&message, "\n");
	string_append(&message, "Posee los siguientes bloques: ");
	string_append(&message, Ip);
	string_append(&message, Puerto);
	string_append(&message, nroBloque);
	string_append(&message, "\n");


	pthread_mutex_lock( &loggerMutex );

	log_info(logFs, message);

	pthread_mutex_unlock( &loggerMutex );

	free(message);
	free(Ip);
	free(Puerto);
	free(nroBloque);


	return;
}

