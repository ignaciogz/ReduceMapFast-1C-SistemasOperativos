/*
 * LoggerNodo.h
 *
 *  Created on: 7/7/2015
 *      Author: utnso
 */


#ifndef LOGGERNODO_H_
#define LOGGERNODO_H_

#include <commons/log.h>
#include <string.h>

typedef enum {NodoNuevo = 0, NodoNoNuevo} estadoNodo;

extern t_log* logNodo;

void logearLecturaDeBloque(int numeroBloque);

void logearEscrituraDeBloque(int numeroBloque);

void logearLecturaDeArchivoTermporal(char* nombreArchivo);

void logearEscrituraDeArchivoTemporal(char* nombreArchivo);

void logearYMostrarParametrosDeConfiguracion(char* ipFs, char* ipNodo, char* espacioDeDatos, char* directorioTemporal, int puertoFs, int puertoNodo, estadoNodo nodoNuevo);

void logearYMostrarConexion(int puerto, char* ip, char* proceso);

void logearYMostrarSolicitudMap(char* ip);

void logearYMostrarSolicitudReduce(char* ip);

void logearYMostrarFinalizacionDeMap(char* ip);

void logearYMostrarFinalizacionDeReduce(char* ip);

void logearYMostrarSolicitudReduceLocal(char* ip);

void logearYMostrarFinalizacionDeReduceLocal(char* ip);

#endif /* LOGGERNODO_H_ */
