/*
 * socketsFs.c
 *
 *  Created on: 5/6/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>/*biblioteca que contiene las funciones para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include "headers/socketsFs.h"
#include "headers/Log.h"
#include "headers/manejoDeDatos.h"

//Variables Globales
extern int cantNodosActivos;
extern int totalBloquesLibres;
extern t_list* listaIp;
extern t_dictionary * nodosHistorial;
extern int Puerto_Marta;
extern char *IP_Marta;
extern pthread_mutex_t historialNodos;
extern pthread_mutex_t historialIP;

//SEMAFOROS
pthread_mutex_t FileMutex = PTHREAD_MUTEX_INITIALIZER;


/*
struct sockaddr_in inicializarDireccionFs(int puerto)
{
	struct sockaddr_in direccion;

	direccion.sin_family = AF_INET;
	direccion.sin_port =htons(puerto);
	inet_aton("192.168.3.78",&(direccion.sin_addr.s_addr));
	memset(&(direccion.sin_zero),'\0', 8 );

	return direccion;
}
*/

struct sockaddr_in inicializarDireccion(int puerto, char* IP)
{
	struct sockaddr_in direccion;

	direccion.sin_family = AF_INET;
	direccion.sin_port =htons(puerto);
	inet_aton(IP,&(direccion.sin_addr));
	memset(&(direccion.sin_zero),'\0', 8 );

	return direccion;
}


void packHeader(t_header header, void* message)
{
	int offset;

	memcpy(message, (t_mensaje*) &header.tipoMensaje, sizeof(t_mensaje));
	offset = sizeof(t_mensaje);

	memcpy(message + offset, (int*) &header.tamanioMensaje, sizeof(int));

	return;
}
void umpackHeader(t_header* header, void* package)
{
	memcpy((t_mensaje*)&header->tipoMensaje, package, sizeof(t_mensaje));
	memcpy((int*) &header->tamanioMensaje, package, sizeof(int));
	return;
}

void packDataConnection (t_connection connection, void* data)
{
	int offset;
	offset= sizeof(t_dictionary)+ sizeof (int);
	memcpy(data + offset, (int*)&connection.connectionSocket, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, (int*)&connection.connectionAddr.sin_family, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, ( int*)&connection.connectionAddr.sin_port, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, (int*)&connection.connectionAddr.sin_addr, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, connection.connectionAddr.sin_zero, 8*sizeof(char));

	return;
}

t_connection umpackDataConnection(void* data)
{
	t_connection connection;

	int offset = 0;

	memcpy((int*)connection.connectionSocket, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((int*)&connection.connectionAddr.sin_family, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(( int*)&connection.connectionAddr.sin_port, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((long*)connection.connectionAddr.sin_addr.s_addr, data + offset, sizeof(long));
	offset = offset + sizeof(int);

	memcpy(connection.connectionAddr.sin_zero, data + offset, 8*sizeof(char));

	return connection;
}

t_conexionNodo umpackNodo(t_connection conexion){
	t_conexionNodo nodo;

		recv(conexion.connectionSocket, (int*) nodo.nuevo, sizeof(int), 0);
		recv(conexion.connectionSocket, (int*) nodo.espacioNodo, sizeof(int), 0);

		return nodo;
}

t_estado_nodo *nodo_create(t_connection conexion, t_conexionNodo nodo){
			t_estado_nodo *historial = malloc(sizeof(t_estado_nodo));
			int i;
			ldiv_t cantBloques;
			char* data;
			t_bitarray* bitarray;
			historial->datosNodo.connectionSocket=conexion.connectionSocket;
			historial->datosNodo.connectionAddr.sin_addr.s_addr=conexion.connectionAddr.sin_addr.s_addr;
			historial->datosNodo.connectionAddr.sin_family=conexion.connectionAddr.sin_family;
			historial->datosNodo.connectionAddr.sin_port=conexion.connectionAddr.sin_port;
			//memset(&(historial->datosNodo.connectionAddr.sin_zero),'\0', 8 );
			historial->estado=0;
			historial->nuevo=nodo.nuevo;
			historial->espacioTotal=nodo.espacioNodo;
			cantBloques = ldiv(nodo.espacioNodo,TAMANIO_BLOQUE);
			data = malloc(cantBloques.quot);
			for(i=0;i < cantBloques.quot; i++){
				data[i]=0;
			}
			bitarray= bitarray_create(data,cantBloques.quot);
			totalBloquesLibres=totalBloquesLibres + cantBloques.quot;
			historial->estadoBloques=bitarray;

			return historial;
	}

void conectarNodo(t_estado_nodo* nodo)
{
	pthread_mutex_lock( &historialNodos );
	nodo->estado=0;
	totalBloquesLibres = totalBloquesLibres + (cantBloquesLibres(nodo->estadoBloques));
	pthread_mutex_unlock( &historialNodos );
	logearConexionNodo(nodo);
	logearEspacioTotalSistema();

	// Me conecto a marta y le aviso el cambio de estado del Nodo
	int sockMarta;
	struct sockaddr_in marta_addr;
	sockMarta=socket(AF_INET,SOCK_STREAM,0);
	marta_addr=inicializarDireccion(Puerto_Marta,IP_Marta);
	while (connect(sockMarta,(struct sockaddr *)&marta_addr,sizeof(struct sockaddr))==-1)
			{;}
	logearYMostrarConexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");

	t_header cabecera;
	t_estado estado=0;
	cabecera.tipoMensaje=cambioDeEstado;
	cabecera.tamanioMensaje=sizeof(int) + sizeof(struct in_addr)+ sizeof(int);
	void* mensaje=malloc(sizeof(t_header) + cabecera.tamanioMensaje);
	packHeader(cabecera,mensaje);
	int offset=sizeof(t_header);
	memcpy(mensaje + offset,(struct in_addr*)&nodo->datosNodo.connectionAddr.sin_addr, sizeof(struct in_addr));
	offset= offset + sizeof(struct in_addr);
	memcpy(mensaje + offset,(int*)&nodo->datosNodo.connectionAddr.sin_port, sizeof(int));
	offset= offset + sizeof(int);
	memcpy(mensaje + offset,(int*)&estado, sizeof(int));


	if((sendall(sockMarta,mensaje,&cabecera.tamanioMensaje))==-1){
			logearYMostrarDesconexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");
		}


close(sockMarta);
}

void desconectarNodo(t_estado_nodo* nodo){

	pthread_mutex_lock( &historialNodos );
	nodo->estado=1;
	totalBloquesLibres = totalBloquesLibres - (cantBloquesLibres(nodo->estadoBloques));
	pthread_mutex_unlock( &historialNodos );

	logearDesconexionNodo(nodo);
	logearEspacioTotalSistema();

	// Me conecto a marta y le aviso el cambio de estado del Nodo
		int sockMarta;
		struct sockaddr_in marta_addr;
		sockMarta=socket(AF_INET,SOCK_STREAM,0);
		marta_addr=inicializarDireccion(Puerto_Marta,IP_Marta);
		while (connect(sockMarta,(struct sockaddr *)&marta_addr,sizeof(struct sockaddr))==-1)
				{;}
		logearYMostrarConexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");

		t_header cabecera;
		t_estado estado=1;
		cabecera.tipoMensaje=cambioDeEstado;
		cabecera.tamanioMensaje=sizeof(int) + sizeof(struct in_addr)+ sizeof(int);
		void* mensaje=malloc(sizeof(t_header) + cabecera.tamanioMensaje);
		packHeader(cabecera,mensaje);
		int offset=sizeof(t_header);
		memcpy(mensaje + offset,(struct in_addr*)&nodo->datosNodo.connectionAddr.sin_addr, sizeof(struct in_addr));
		offset= offset + sizeof(struct in_addr);
		memcpy(mensaje + offset,(int*)&nodo->datosNodo.connectionAddr.sin_port, sizeof(int));
		offset= offset + sizeof(int);
		memcpy(mensaje + offset,(int*)&estado, sizeof(int));


		if((sendall(sockMarta,mensaje,&cabecera.tamanioMensaje))==-1){
				logearYMostrarDesconexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");
			}
close(sockMarta);

}

char* umpackMartaConnection(t_connection connection,t_header header, t_dictionary *nodosHistorial)
{
	char* fileName;
	int nbytes;
	char* IP;
	t_estado_nodo* nodoDesc;
	fileName= (char*) malloc(header.tamanioMensaje);
	if((nbytes=recv(connection.connectionSocket, fileName,header.tamanioMensaje , 0)) <=0){
		if(nbytes == 0){
			IP=inet_ntoa(connection.connectionAddr.sin_addr);
			nodoDesc=(t_estado_nodo* )dictionary_get(nodosHistorial,IP);
			desconectarNodo(nodoDesc);
		}else{perror("send umpackFile");}
	}

	return fileName;
}

void packDiccionarioNodo(t_dictionary *nodoHistorial, void *data){
	int offset;
	offset=sizeof(int);
	memcpy(data + offset, nodoHistorial, sizeof(t_dictionary));

	return;
}

t_dictionary* umpackDiccionarioNodo(void *data){
	t_dictionary *dataNodo=malloc(sizeof(t_dictionary));
	int offset;
	offset= sizeof(int);
	memcpy(dataNodo, data + offset,sizeof(t_dictionary));
	return dataNodo;
}

void packsockFs(int sock, void *data){
	int offset=0;
	memcpy(data + offset,(int*)&sock,sizeof(int));
	return;
}

int umpacksockFs(void *data){
	int offset=0;
	int sock;
	memcpy((int*)&sock, data + offset, sizeof(int));
	return sock;
}

void packFileContent(int tamanioNombre,char* nombreArch, void* message)
{
	int offset;
	offset=sizeof(t_header);
	memcpy(message + offset, (int*) tamanioNombre, sizeof(int));
	offset = offset + sizeof(int);
	memcpy(message + offset, nombreArch, tamanioNombre);

	return;
}

int sendall(int s, char* buf, int* len)
{
	int total = 0;//cuantos bytes ha enviado
	int bytesleft = *len;//cuantos bytes quedan pendientes
	int n;
	while(total < *len){
		n = send(s, buf+total,bytesleft,0);
		perror("send");
		if(n == -1){break;}
		total += n;
		bytesleft -= n;
	}
	*len = total;//devuelve la cant de bytes enviados realmente
	return n==-1?-1:0;//devuelve -1 si falla, 0 en otro caso
}

void packSolicitudMarta(void* message,int indice,int tamNombreArch,char* nombreArch, int cantCopias)
{
	int offset=sizeof(t_header);

	/*memcpy(message + offset, (struct in_addr*) &Ip_Job, sizeof(struct in_addr));
	offset = offset + sizeof(struct in_addr);

	memcpy(message + offset, (int*)Puerto_Job, sizeof(int));*/
	memcpy(message + offset,(int*)&indice,sizeof(int));
	offset = offset + sizeof(int);

	memcpy(message + offset,(int*)&tamNombreArch,sizeof(int));
	offset = offset + sizeof(int);

	memcpy(message + offset, nombreArch,tamNombreArch);
	offset= offset + tamNombreArch;

	memcpy(message + offset, (int*)&cantCopias,sizeof(int));

	return;
}

void packSolicitudErrorMarta(void* message,struct in_addr Ip_Job,int Puerto_Job)
{
	int offset=sizeof(t_header);

	memcpy(message + offset, (struct in_addr*)&Ip_Job, sizeof(struct in_addr));
	offset = offset + sizeof(struct in_addr);

	memcpy(message + offset, (int*)&Puerto_Job, sizeof(int));

	return;
}

t_estado_nodo umpackDatosNodo(void* data)
{
	t_estado_nodo nodo;

	int offset=0;

	memcpy((int*)&nodo.datosNodo.connectionSocket, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((int*)&nodo.datosNodo.connectionAddr.sin_family, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(( int*)&nodo.datosNodo.connectionAddr.sin_port, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((long*)&nodo.datosNodo.connectionAddr.sin_addr.s_addr, data + offset, sizeof(long));
	offset = offset + sizeof(int);

	memcpy(nodo.datosNodo.connectionAddr.sin_zero, data + offset, 8*sizeof(char));
	offset = offset + 8*sizeof(char);

	memcpy((t_estado*)&nodo.estado, data + offset,sizeof(t_estado));
	offset= offset + sizeof(t_estado);

	memcpy((t_boleano*)&nodo.nuevo, data + offset,sizeof(t_boleano));
	offset= offset + sizeof(t_boleano);

	memcpy((int*)&nodo.estado, data + offset,sizeof(t_estado));
	offset= offset + sizeof(int);

	memcpy(nodo.estadoBloques, data + offset,sizeof(t_bitarray));



	return nodo;
}

void packDataRequest (t_header cabecera	,t_connection connection, void* data)
{
	int offset=0;

	memcpy(data + offset, (int*)&connection.connectionSocket, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, (int*)&connection.connectionAddr.sin_family, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, ( int*)&connection.connectionAddr.sin_port, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, (int*)&connection.connectionAddr.sin_addr.s_addr, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, connection.connectionAddr.sin_zero, 8*sizeof(char));
	offset= offset + (8*sizeof(char));

	memcpy(data + offset, (t_mensaje*) &cabecera.tipoMensaje, sizeof(t_mensaje));
	offset= offset + sizeof(t_mensaje);

	memcpy(data + offset, (int*) &cabecera.tamanioMensaje, sizeof(int));

	return;
}

t_request umpackDataRequest(void* data)
{
	t_request request;

	int offset=0;

	memcpy((int*)&request.conexion.connectionSocket, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((int*)&request.conexion.connectionAddr.sin_family, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((int*)&request.conexion.connectionAddr.sin_port, data + offset, sizeof(int));
	offset = offset + sizeof(int);

	memcpy((long*)&request.conexion.connectionAddr.sin_addr.s_addr, data + offset, sizeof(long));
	offset = offset + sizeof(int);

	memcpy(request.conexion.connectionAddr.sin_zero, data + offset, 8*sizeof(char));
	offset = offset + 8*sizeof(char);

	memcpy((t_mensaje*) &request.cabecera.tipoMensaje, data + offset,sizeof(t_mensaje));
	offset= offset + sizeof(t_mensaje);

	memcpy((int*) &request.cabecera.tamanioMensaje, data + offset,sizeof(int));
	offset= offset + sizeof(int);


	return request;
}

void packIpPuertoNodo(void* mensaje, int offset,struct in_addr* Ip)
{
	int i;
	char* Ip_Nodo;
	t_estado_nodo* nodo;
	for(i=0;i < cantNodosActivos;i++){

		pthread_mutex_lock( &historialIP );
		Ip=(struct in_addr*)list_get(listaIp,i);
		pthread_mutex_unlock( &historialIP );

		memcpy(mensaje + offset,Ip,sizeof(struct in_addr));
		offset= offset + sizeof(struct in_addr);

		Ip_Nodo = inet_ntoa(*Ip);
		pthread_mutex_lock( &historialNodos );
		nodo=(t_estado_nodo*)dictionary_get(nodosHistorial,Ip_Nodo);
		pthread_mutex_unlock( &historialNodos );
		memcpy(mensaje + offset,(int*)&nodo->datosNodo.connectionAddr.sin_port,sizeof(int));
		offset= offset + sizeof(int);


	}

	return;
}

void createFile(int fileLength, char* path, char* content)
{

	pthread_mutex_lock( &FileMutex );

	int fileDescriptor = open(path, O_WRONLY | O_CREAT); // abro el archivo donde se va a guardar el contenido al principio del mismo, en caso de no existir se creará

	char* mapped; // se define la variable sobre la que se mapeara el archivo

		/*se obtiene el area a mapeada */
	mapped = (char*) mmap(0, fileLength, PROT_WRITE, MAP_SHARED,fileDescriptor, 0);

	memcpy(mapped, content, fileLength);/*Se copia la informacion del archivo */

	pthread_mutex_unlock( &FileMutex );

	munmap(mapped, fileLength); //libero los datos mapeados

	close(fileDescriptor); //cierro el archivo

	return;
}

void verificarConexionNodo(t_estado_nodo* nodoDicc){
	if(nodoDicc->estado==1){
		conectarNodo(nodoDicc);
	}
return;
}
