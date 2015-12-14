/*
 * Interfaz_Job-Marta.h
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */

#ifndef SRC_INTERFAZ_JOB_MARTA_H_
#define SRC_INTERFAZ_JOB_MARTA_H_

#include "ManejoMensajesJob.h"
#include "Log_Job.h"

//Funcion generica que sirve para notificar a marta o bien el final de una operacion o el fallo en la misma, solo debe pasarse el tipo de mensaje como parametro
void notificarMarta(struct in_addr ipMarta, int puertoMarta, t_mensaje tipoDeMensaje, t_log* logger, t_log* loggersin);

//Busca un nodo en la lista de nodos que van a ejecutar operacioens, en caso de estar devuelve el indice de la lista en la que se encuentr el nodo
//en caso de no estar devuelve -1
int buscarNodo(t_list* listaDeNodos, struct in_addr ip, int puerto);

//Recibe los bloques que manda marta de un archivo y los coloca en la lista de nodos que van a ejecutar operaciones, el envio de pedido de maps a los nodos debe
//hacerse posteriormente
int recibirBloquesAMapear(int sock, t_list* listaDeNodos);

//Envia a marta los datos del proceso para que le devuelva los bloques a mapear, devuelve el socket que esta conectado a marta;
int enviarAMartaDatosDelJob(datosJob_t job, t_log* logger, t_log* loggersin);


#endif /* SRC_INTERFAZ_JOB_MARTA_H_ */
