/*
 * ManejoDeSockets.c
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>/*biblioteca que contiene las funciones para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> /*biblioteca que contiene la funcion memset()*/
#include <errno.h>

#include "ManejoDeSockets.h"


struct sockaddr_in inicializarDireccion(int puerto, char* ip)
{
	struct sockaddr_in direccion;

	direccion.sin_family = AF_INET;
	direccion.sin_port = htons(puerto);
	inet_aton(ip, &(direccion.sin_addr));
	memset(&(direccion.sin_zero), '\0', 8);

	return direccion;
}

void sendAll(int sendSocket, void* message, int length)
{
	int offset = 0; // desplazamiento
	int sent = 0;	//cantidad de bytes enviados

	do{

		do{
			sent = send(sendSocket, message + offset, length, 0); //se envia el mensaje y se registra la cantidad de bytes enviados
			//perror("Error:");
		}while(sent < 0); //mientras no se mande nada seguir intentando.

	offset = offset + sent; //se le suma al desplazamiento la cantidad de bytes enviados
	length = length - sent; //se le resta a la longitud la cantidad de bytes enviados

	}while(length != 0 ); //cuando la longitud sea 0 significa que se envio el mensaje entero

	return;
}

void packHeader(t_header header, void* message)
{
	int offset;

	memcpy(message, (t_mensaje*) &header.tipoMensaje, sizeof(t_mensaje));//copio el tipo de mansaje al mensaje
	offset = sizeof(t_mensaje); //aumento el desplazamiento

	memcpy(message + offset, (int*) &header.tamanioMensaje, sizeof(int));//copio la longitud del mensaje (sin contar la cabecera) al mensaje

	return;
}

void umpackHeader(t_header* header, void* package)
{
	memcpy((t_mensaje*)&header->tipoMensaje, package, sizeof(t_mensaje));
	memcpy((int*)& header->tamanioMensaje, package, sizeof(int));
	return;
}

void packDataConnection (t_connection connection, void* data)
{
	int offset = 0;

	memcpy(data + offset, (int*)&connection.connectionSocket, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, (int*)&connection.connectionAddr.sin_family, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, ( int*)&connection.connectionAddr.sin_port, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, (long*)&connection.connectionAddr.sin_addr.s_addr, sizeof(long));
	offset = offset + sizeof(int);

	memcpy(data + offset,(char*)&connection.connectionAddr.sin_zero, 8*sizeof(char));

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

	memcpy((long*)&connection.connectionAddr.sin_addr.s_addr, data + offset, sizeof(long));
	offset = offset + sizeof(int);

	memcpy((char*)&connection.connectionAddr.sin_zero, data + offset, 8*sizeof(char));

	return connection;
}

void packDataFiles(char* espacioDatos, char* espacioTemporal, struct in_addr ipNodo ,void* data)
{
	int offset = sizeof (t_connection);
	int longitud;

	longitud = strlen(espacioDatos) + 1;
	memcpy(data + offset, (int*) &longitud, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, espacioDatos, longitud);
	offset = offset + longitud;

	longitud = strlen(espacioTemporal) + 1;
	memcpy(data + offset, (int*) &longitud, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(data + offset, espacioTemporal, longitud);
	offset = offset + longitud;

	memcpy(data + offset, (struct in_addr*) &ipNodo, sizeof(struct in_addr));

	return;
}

void umpackDataFiles (char* espacioDatos, char* directorioTemporal, struct in_addr* myip, void* data)
{
	int longitud;
	int offset = sizeof(t_connection);

	memcpy((int*) &longitud, data + offset, sizeof(int));
	offset = offset+ sizeof(int);
	espacioDatos = malloc (longitud + 1);

	memcpy(espacioDatos, data + offset, longitud);
	offset = offset + longitud;

	memcpy((int*) longitud, data + offset, sizeof(int));
	offset = offset + sizeof(int);
	directorioTemporal = malloc (longitud + 1);

	memcpy(directorioTemporal, data + offset, longitud);
	offset = offset + longitud;

	memcpy(myip, data + offset, sizeof(struct in_addr));

	return;
}

int umpackBlockNum(t_connection connection)
{
	int blockNum;
	int offset = 0;
	int recived = 0;

	//Hago el recv comprobando errores y asegurandome que reciba todo.
	do{
		do{
			recived = recv(connection.connectionSocket,((int*)&blockNum) + offset , sizeof(int), 0);
		}while(recived > 0);
		offset = offset + recived;
	}while (offset < sizeof(int));


	return blockNum;
}

void packBlock (t_bloque_datos block, void* message)
{
	int offset = sizeof(t_header);

	memcpy(message + offset, (int*)&block.numero_bloque, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(message + offset, block.datos, tamanio_bloque);

	return;
}

t_bloque_datos umpackBlock(t_connection connection)
{
	t_bloque_datos block;
	block.datos = malloc(tamanio_bloque);

	recv(connection.connectionSocket, (int*) &block.numero_bloque, sizeof(int), 0);
	recv(connection.connectionSocket, (char*) block.datos, tamanio_bloque, 0);

	return block;
}

void sendBlock(t_bloque_datos block, struct in_addr ip, int puerto)
{
	int messageLength = tamanio_bloque + sizeof(int) + sizeof(t_header);
	void* message = malloc(messageLength);
	t_header header;

	header.tamanioMensaje = messageLength - sizeof(t_header);
	header.tipoMensaje = EnvioBloque;

	packHeader(header, message);
	packBlock(block, message);

	int sock;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
			perror("socket");
			exit(1);
	}

	struct sockaddr_in addr;

	addr.sin_addr = ip;
	addr.sin_family = AF_INET;
	addr.sin_port = puerto;
	memset(&(addr.sin_zero), '\0', 8);

	//Conecto al socket al FileSystem para notificar conexion del nodo
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		{
			perror("connect");
			exit(1);
		}

	sendAll(sock, message, messageLength);


	return;
}

char* umpackFileName(t_connection connection)
{
	char* fileName;
	int length;

	recv(connection.connectionSocket, (int*)&length, sizeof(int), 0);
	fileName = malloc(length + 1);
	recv(connection.connectionSocket, fileName, length, 0);

	return fileName;
}

void packFile(char* fileName, char* content, int length, void* message)
{
	int offset = sizeof(t_header);
	int fileLength = strlen(fileName) + 1;

	memcpy(message + offset, (int*) &fileLength, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(message + offset, fileName, strlen(fileName) + 1);
	offset = offset + fileLength;

	memcpy(message + offset, (int*) &length, sizeof(int));
	offset = offset + sizeof(int);

	memcpy(message + offset, content, length);

	return;
}

void sendFileContent(char* fileName, char* content, struct in_addr ip, int puerto)
{
	t_header header;
	void* message;
	int contentLength = strlen(content) + 1;
	int messageLength = sizeof(t_header) + contentLength + sizeof(int) * 2 + strlen(fileName) + 1;

	header.tipoMensaje = EnvioArchivo;
	header.tamanioMensaje = messageLength - sizeof(t_header);

	message = malloc(messageLength);

	packHeader(header, message);

	packFile(fileName, content, contentLength, message);

	int sock;

		if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
				perror("socket");
				exit(1);
		}

		struct sockaddr_in addr;

		addr.sin_addr = ip;
		addr.sin_family = AF_INET;
		addr.sin_port = puerto;
		memset(&(addr.sin_zero), '\0', 8);

		//Conecto al socket al FileSystem para notificar conexion del nodo
		if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
			{
				perror("connect");
				exit(1);
			}

	sendAll(sock, message, messageLength);

	return;
}

void umpackFileContent(t_connection connection, char* content, int length)
{
	int offset = 0;
	int recived = 0;

	//Hago el recv comprobando errores y asegurandome que reciba todo.
	do{
		do{
			recived = recv(connection.connectionSocket,content + offset , length, 0);
		}while(recived > 0);
		offset = offset + recived;
	}while (offset < length);

	return;
}

void sendFileToReduce(char* fileName, char* content, struct in_addr ipJob, t_connection connection, int puertoNodo)
{
	t_header header;
	void* message;
	int contentLength = strlen(content) + 1;
	int messageLength = sizeof(t_header) + contentLength + sizeof(int) * 2 + strlen(fileName) + 1 + sizeof(struct in_addr);

	header.tipoMensaje = EnvioArchivoAReducir;
	header.tamanioMensaje = messageLength - sizeof(t_header);

	message = malloc(messageLength);

	packHeader(header, message);

	packFile(fileName, content, contentLength, message);

	int offset = messageLength - sizeof(struct in_addr);
	memcpy(message + offset, (struct in_addr*) &ipJob, sizeof(struct in_addr));

	int sock;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
			perror("socket");
			exit(1);
	}

	struct sockaddr_in addr;

	addr = connection.connectionAddr;
	addr.sin_port = puertoNodo;

	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		{
			perror("connect");
			exit(1);
		}

	sendAll(sock, message, messageLength);

	return;
}

/*void packLocalReduceRequest(void* message, char* reduceFileContent, char* localResultFileName, t_reduce reduce, t_archivoAReducir* node)
{
	int offset = sizeof(t_header);
	int i;
	char* fileName;

	memcpy(message + offset, (int*) strlen(reduce.archivoReducer), sizeof(int));
	offset = offset + sizeof(int);
	memcpy(message + offset, reduce.archivoReducer, strlen(reduce.archivoReducer));
	offset = offset + strlen(reduce.archivoReducer);

	memcpy(message + offset, (int*) strlen(reduceFileContent), sizeof(int));
	offset = offset + sizeof(int);
	memcpy(message + offset, reduceFileContent, strlen(reduceFileContent));
	offset = offset + strlen(reduceFileContent);

	memcpy(message + offset, (int*) strlen(localResultFileName), sizeof(int));
	offset = offset + sizeof(int);
	memcpy(message + offset, localResultFileName, strlen(localResultFileName));
	offset = offset + strlen(localResultFileName);

	memcpy(message + offset, (int*) list_size(node->archivosDelNodo), sizeof(int));
	offset = offset + sizeof(int);

	for(i = 0; i < list_size(node->archivosDelNodo); i++)
	{
		fileName = list_get(node->archivosDelNodo, i);

		memcpy(message + offset, (int*) strlen(fileName), sizeof(int));
		offset = offset + sizeof(int);
		memcpy(message + offset, fileName, strlen(fileName));
		offset = offset + strlen(fileName);
	}

	memcpy(message + offset, (struct in_addr*) &reduce.ipJob, sizeof(struct in_addr));

	return;
}*/
