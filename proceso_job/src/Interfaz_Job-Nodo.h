/*
 * Interfaz_Jon-Nodo.h
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */


#ifndef SRC_INTERFAZ_JOB_NODO_H_
#define SRC_INTERFAZ_JOB_NODO_H_


#include "ManejoMensajesJob.h"


//Empaqueta los datos del mensaje de pedido de map para un bloque dado
void empaquetarPedidoDeMap(void* mensaje, t_header cabecera, char* archivoMap, long tamanioScript, char* scriptMap, bloque_t* bloque);

//Envia la solicitud de map a los bloques de un nodo que aun no hay sino notificados del pedido de map
void enviarSolicitudDeMap(nodo_t nodo, char* archivoMap);

//Devuelve la cantidad de archivos que se van a reducir, o lo q es equivalente la cantidad de bloques que fueron mapeados
int obtenerCantidadDeArchivosAReducir(t_list* listaDeNodos);

//Envia a un nodo la solicitud de reduce
int enviarSolicitudDeReduce(t_list* listaDeNodos, combiner_t combiner, struct in_addr ipNodoReducer, int puertoNodoReducer, char* archivoReduce, char* archivoResultado);

#endif /* SRC_INTERFAZ_JOB_NODO_H_ */
