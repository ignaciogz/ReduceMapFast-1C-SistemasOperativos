/*
 * socketsFs.h
 *
 *  Created on: 5/6/2015
 *      Author: utnso
 */
#ifndef HEADERS_SOCKETSFS_H_
#define HEADERS_SOCKETSFS_H_

#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define TAMANIO_BLOQUE 20971520

typedef enum{
	//Mensajes con Marta
	ConexionFileSystem = 0,
	MartaSolicitudBloques = 1,
	RespuestaSolicitudBloques = 2,
	MartaPedidoArchTerminado = 3,
	ErrorSolicitud = 4,
	nodoNuevo = 5,
	cambioDeEstado = 6,

	//Mensajes con Nodo
	ConexionNodo= 7,
	SetBloque = 8,
	getFileContent = 9,
	EnvioArchFinal = 10,
	GetBloque = 11,
	RecibirBloque = 12
	}t_mensaje;

typedef struct
{
	int connectionSocket;
	struct sockaddr_in connectionAddr;
}t_connection;

typedef struct
{
	t_mensaje tipoMensaje;
	int tamanioMensaje;
}t_header;

typedef enum{
	CONECTADO =0,
	DESCONECTADO
}t_estado;

typedef enum{
	NodoNuevo=0,
	NodoNoNuevo
}t_boleano;

typedef struct{
	t_connection datosNodo;
	t_estado estado;
	t_boleano nuevo;
	long espacioTotal;
	t_bitarray* estadoBloques;
}t_estado_nodo;

typedef struct{
	t_boleano nuevo;
	long espacioNodo;
}t_conexionNodo;

typedef struct{
		long int IP;
		int puerto;
		int numBloque;
}t_copiaX;

typedef struct{
	struct in_addr Ip;
	t_bitarray* bitarray;
}t_nodo;

typedef struct{
	t_connection conexion;
	t_header cabecera;
}t_request;


//Dada un numero de puerto,devuelve una estructura sockaddr_in con la direccion
struct sockaddr_in inicializarDireccionFs(int puerto);
//Dado IP y puerto arbitrario devuelve la estructura sockaddr_in completa
struct sockaddr_in inicializarDireccion(int puerto, char* IP);
//Dada una cabecera y el espacio de memoria donde se guardara el mensaje, empaqueta la cabecera
void packHeader(t_header header, void* message);
//Dado un header y un paquete, desempaqueta las componentes del header y las copia en el mismo
void umpackHeader(t_header* header, void* package);
// Dado un socket, una conexion y el espacio de memoria donde empaquetalos, los copia al espacio de memoria
void packDataConnection (t_connection connection, void* data);
//Dado un bolque de datos, desempaqueta los componentes del sockaddr y el socket
t_connection umpackDataConnection(void* data);
//recibe si el nodo es nuevo y su tamanio
t_conexionNodo umpackNodo(t_connection conexion);
//Crea un registro nodo para luego almacenarlo en la lista
t_estado_nodo *nodo_create(t_connection conexion, t_conexionNodo nodo);
//Cambia el estado del Nodo a Conectado
void conectarNodo(t_estado_nodo* nodo);
//Cambia el estado del Nodo a Desconectado
void desconectarNodo(t_estado_nodo* nodo);
//Recibe el nombre de archivo de la solicitud marta
char* umpackMartaConnection(t_connection connection,t_header header, t_dictionary *nodosHistorial);
//Almacena en un paquete el diccionnario y la cant nodos pasada por config
void packDiccionarioNodo(t_dictionary *nodoHistorial, void *data);
//Dado un paquete devuelve el diccionario de nodos
t_dictionary* umpackDiccionarioNodo(void *data);
//Guarda en el paquete data el socket del FS
void packsockFs(int sock, void *data);
//Devuelve de un paquete el socket del FS
int umpacksockFs(void *data);
//Empaqueta en un mensaje el tamanio y nombre de archivo y devuelve el mensaje
void packFileContent(int tamanioNombre,char* nombreArch, void* message);
//Se encarga de enviar todos los bytes de un mensaje
int sendall(int s, char* buf, int* len);
//EMpaqueta los datos  en el mensaje
void packSolicitudMarta(void* message,int indice,int tamNombreArch,char* nombreArch, int cantCopias);
//Empaqueta ip y job en caso de q no exista archivo en db
void packSolicitudErrorMarta(void* message,struct in_addr Ip_Job,int Puerto_Job);
//saca de un paquete y retorna un struct con los datos
t_estado_nodo umpackDatosNodo(void* data);
//almacena en paquete los datos para atender una solicitud
void packDataRequest (t_header cabecera	,t_connection connection, void* data);
//Desempaqueta la solicitud
t_request umpackDataRequest(void* data);
//almacena los Ip de los nodos conectados en mensaje
void packIpPuertoNodo(void* mensaje, int offset,struct in_addr* Ip);
//Se crea el archivo donde se guarda resultado final
void createFile(int fileLength, char* path, char* content);
//Verifica si el nodo se encuentra desconectado, en ese caso le avisa a marta y lo conecta
void verificarConexionNodo(t_estado_nodo* nodoDicc);

#endif /* HEADERS_SOCKETSFS_H_ */
