/*
 * ManejoMensajesJob.h
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */

#ifndef SRC_MANEJOMENSAJESJOB_H_
#define SRC_MANEJOMENSAJESJOB_H_

#include <commons/collections/list.h>

typedef enum {

	//Mensajes con Nodo
	SolicitudDeMap = 25,
	FinalizacionDeMap = 26,
	SolicitudDeReduceSinCombiner = 27,
	SolicitudDeReduceConCombiner = 28,
	FinalizacionDeReduce = 29,

	//Mensajes con Marta
	ErrorSolicitud = 4,
	datosJob= 16,
	pedidoMapping = 17,
	operacionMapExitosa = 18,
	operacionMapErronea = 19,
	pedidoReduce = 20,
	operacionReduceExitosa = 21,
	operacionReduceErronea = 22,
	FinDeOperacion = 23,
	Error = 24

}t_mensaje;

typedef struct
{
	t_mensaje tipoMensaje;
	int tamanioMensaje;
}t_header;

typedef enum{ConCombiner,
			SinCombiner
}combiner_t;

typedef enum{Mapeado,
			SinMapear
}mapeado_t;

typedef struct{
	struct in_addr ipMarta;
	int puertoMarta;
	char* archivoMap;
	char* archivoReduce;
	char* archivoResultado;
	t_list* archivosAOperar;
	combiner_t combiner;
}datosJob_t;

typedef struct{
	struct in_addr ip;
	int puerto;
	t_list* bloquesAOperar;
}nodo_t;

typedef struct{
	int numeroBloque;
	char* archivoResultado;
	int socket;
	mapeado_t estado;
}bloque_t;

typedef struct{
	t_header cabecera;
	datosJob_t job;
	t_list* listaDeNodos;
	int sock;
}datoSolicitud_t;

typedef struct{
	t_list* listaDeNodos;
	char* archivoMaper;
}envioMap_t;

//Agrega los sockets de los nodos al set y devuelve el mayor file descriptor;
int agregarSocketsASet(fd_set* set, t_list* listaDeNodos);

//Realiza los envios de map sin hacer de todos los nodos registrados en la lista de nodos
void enviarPedidosdeMap(envioMap_t datosMap);

//Funcion que mandeja los envios y recepciones de solicitudes
int manejarSolicitudes(datoSolicitud_t solicitud, t_log* logger, t_log* loggersin);

//Empaqueta la cabecera al comienzo de un mensaje
void packHeader(t_header header, void* message);

//Envia los datos de un mensaje asegurandose de que se entregue completo
void sendAll(int sendSocket, void* message, int length);

//Devuelve el tamaño en bytes del contenido de un archivo
long tamanioArchivo(char* archivo);

//Obtiene el contenido de un archivo y lo almacena en la variable contenido
void getFileContent (char* directorioArchivo, char* contenido);

//Empaqueta en un mensaje los datos de un bloque que va a ser reducido (bloque = archivo resultado de map de ese bloque)
//devuelve la posicion desde donde empezar a copiar el proximo dato (desplazamiento aumentado)
long empaquetarBloqueAReducir(void* mensaje, struct in_addr ip, int puerto, char* archivoAReducir, long desplazamiento);

//Empaqueta en un mensaje los datos del reduce cuya longitud se conocen desde un principio
void empaquetarDatosFijosDelReduce(void* mensaje, char* archivoReduce, char* scriptReduce, long tamanioScript, char* archivoResultado, int cantidadArchivosAReducir);

#endif /* SRC_MANEJOMENSAJESJOB_H_ */
