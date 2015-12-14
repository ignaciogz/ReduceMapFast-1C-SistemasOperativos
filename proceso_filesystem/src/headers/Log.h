/*
 * Log.h
 *
 *  Created on: 17/7/2015
 *      Author: utnso
 */

#ifndef LOG_H_
#define LOG_H_

#include "socketsFs.h"


void logearLecturaDeArchivo(char* nombreArchivo);
void logearEscrituraDeArchivo(char* nombreArchivo);
void logearConexionNodo(t_estado_nodo* conexionNodo);
void logearDesconexionNodo(t_estado_nodo* conexionNodo);
void logearYMostrarConexion(int puerto, char* ip, char* proceso);
void logearEspacioTotalSistema();
void logearBloqueDeArchivo(char* nomArchivo,t_copiaX* copia);
void logearYMostrarDesconexion(int puerto, char* ip, char* proceso);

#endif /* LOG_H_ */
