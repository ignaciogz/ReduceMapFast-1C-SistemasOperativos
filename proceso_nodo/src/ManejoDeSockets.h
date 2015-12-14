/*
 * ManejoDeSockets.h
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */


#include "Interfaz_Nodo_FileSystem.h"
#include "Interfaz_Nodo-Job.h"

#ifndef MANEJODESOCKETS_H_
#define MANEJODESOCKETS_H_

#include <netinet/in.h>
#include <arpa/inet.h>

//Biblioteca para el manejo de las estructuras de direccion y el envio de mensajes
#include "Interfaz_Nodo_FileSystem.h"

//Se define el enumerativo que hace referencia a los tipos de mensajes enviados entre los procesos
typedef enum {
				//Mensajes con file system
				ConexionNodo = 7,
				SeteoBloque = 8,
				PedidoArchivo = 9,
				EnvioArchivo = 10,
				//Mensajes con consola
				PedidoBloque = 11,
				EnvioBloque = 12,

				//Mensajes con Nodos
				PedidoArchivoAReducir = 13,
				PedidoReduceLocal = 14,
				EnvioArchivoAReducir = 15,

				//Mensajes con job
				SolicitudDeMap = 25,
				FinalizacionDeMap = 26,
				SolicitudDeReduceSinCombiner = 27,
				SolicitudDeReduceConCombiner = 28,
				FinalizacionDeReduce = 29,


} t_mensaje;

//Se define una estructura que representa la cabecera de un mensaje. Contiene informacion sobre el mensaje.
typedef struct
{
	t_mensaje tipoMensaje;
	int tamanioMensaje;
}t_header;

//Se define una estructura que representa una conexion
typedef struct
{
	int connectionSocket;
	struct sockaddr_in connectionAddr;
}t_connection;

//Dada un numero de puerto y una direccion de ip devuelve una estructura sockaddr_in con la direccion correspondiente
struct sockaddr_in inicializarDireccion(int puerto, char* ip);

//Dado un mensaje, su longitud, y el socket por el cual enviarlo, se asegura de enviar el mensaje completo
void sendAll(int sendSocket, void* message, int length);

//Dada una cabecera y el espacio de mememoria donde se guardara el mensaje, empaqueta la cabecera
void packHeader(t_header header, void* message);

//Dado un header y un paquete, desempaqueta las componentes del header y las copia en el mismo
void umpackHeader(t_header* header, void* package);

// Dado un socket, una conexion y el espacio de memoria donde empaquetalos, los copia al espacio de memoria
void packDataConnection (t_connection connection, void* data);

t_connection umpackDataConnection(void* data);

// Dado el nombre del espacio de datos y el espacio temporal lo empaca en el espacio de la conexion
void packDataFiles(char* espacioDatos, char* espacioTemporal, struct in_addr ipNodo ,void* data);

// Desempaqueta los nombres del espacio de datos y el directorioTemporal de un paquete de conexion
void umpackDataFiles (char* espacioDatos, char* directorioTemporal, struct in_addr* myip, void* data);

//Desempaqueta un numero de bloque
int umpackBlockNum(t_connection connection);

//Dado un bloque lo empaqueta en un mensaje luego de la cabecera
void packBlock (t_bloque_datos block, void* message);

//Desempaqueta un bloque de datos
t_bloque_datos umpackBlock(t_connection connection);

//Envia un bloque de datos
void sendBlock(t_bloque_datos block, struct in_addr ip, int puerto);

//Desempaqueta un nombre de archivo
char* umpackFileName(t_connection connection);

//Empaqueta la longitud del nombre del archivo y el nombre juncon con la longitud del contenido y el contenido de un archivo
void packFile(char* fileName, char* content, int length, void* message);

//Envia el contenido de un archivo y su nombre
void sendFileContent(char* fileName, char* content, struct in_addr ip, int puerto);

//Desempaqueta el contenido de un archivo
void umpackFileContent(t_connection connection, char* content, int length);

//Envia un archivo a ser reducido a otro nodo
void sendFileToReduce(char* fileName, char* content, struct in_addr ipJob, t_connection connection, int puertoNodo);


#endif /* MANEJODESOCKETS_H_ */
