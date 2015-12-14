#ifndef MENSAJES_H_
#define MENSAJES_H_


#include <netinet/in.h>
#include <arpa/inet.h>
#include "socketsMaRTA.h"


typedef struct{
	t_cabecera cabecera;
	t_conexion conexion;
	t_list* listaJobs;
	t_list* listaNodos;
	struct sockaddr_in direccionFs;
}t_parametro;

typedef struct{
	t_list* listaJobs;
	t_list* listaNodos;
	pthread_t hilo;
}t_parametroPantalla;

void atenderMensaje(t_parametro* parametros);


#endif /* MENSAJES_H_ */
