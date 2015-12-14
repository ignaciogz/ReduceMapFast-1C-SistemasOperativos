#ifndef comandosEjecutablesSecundarios_H_
#define comandosEjecutablesSecundarios_H_

#include <commons/bitarray.h>
#include <commons/txt.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../headers/socketsFs.h"
#include "../../headers/codigoMD5.h"
#include "../../headers/manejoDeDatos.h"
#include "../../mongodb/headers/mongodb.h"

#define TAMANIO_BLOQUE 20971520

//Secundarias Utilies:
	char* obtenerContenidoDelBloque(int sockConsola);
	struct sockaddr_in inicializarDireccionConsola(int puerto, long int IP);
	char * quitarBarraCerosSiFueraNecesario(char * contenido);
	int bloquesDisponibles(char * nombreDelArchivo);

//Las Cuatro principales:
	void copiarAlFS(char* nombreDelArchivo, char* rutaDeAlmacenamiento);
	void obtenerMD5(char* nombreDelArchivo);

	t_nodo nodoConPrioridadV2(char ** ipHistorial, int cantidadTotalDeCopias);
	bool noSeRepiteV2(char* Ip_Lista, char ** ipHistorial, int cantidadTotalDeCopias);
#endif
