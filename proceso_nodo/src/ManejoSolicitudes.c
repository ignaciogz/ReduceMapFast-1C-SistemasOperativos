/*
 * ManejoSolicitudes.c
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>/*biblioteca que contiene las funciones para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ManejoDeSockets.h"
#include "ManejoSolicitudes.h"
#include <commons/string.h>

#include "Interfaz_Nodo_FileSystem.h"
#include "Interfaz_Nodo-Job.h"
#include "LoggerNodo.h"


void* receiveRequest(t_data* dataConnection)
{
	void* buffer = malloc(sizeof(t_header));

	//Recibo la cabecera del mensaje
	recv(dataConnection->connection.connectionSocket, buffer, sizeof(t_header), 0);

	//Desempaqueto la cabecera
	t_header header;

	umpackHeader(&header, buffer);

	execRequest(header, dataConnection->connection, dataConnection->espacioDatos, dataConnection->directorioTemporal,
				dataConnection->miIp, dataConnection->miPuerto, dataConnection->fsIp, dataConnection->fsPuerto);

	free(dataConnection);

	return NULL;
}


void execRequest(t_header header, t_connection connection, char* espacioDatos, char* directorioTemporal, struct in_addr myip,
					int miPuerto, struct in_addr fsIp, int fsPuerto)
{
	t_bloque_datos requestedBlock;
	t_bloque_datos newBlock;
	char* content = NULL;
	char* requestedFileName = NULL;
	char* scriptFile = NULL;
	char* resultFile = NULL;
	char* scriptFileDirectory = NULL;
	char* resultFileDirectory = NULL;
	char* fileToReduce = NULL;
	char* fileToReduceDirectory = NULL;
	int fileLength = 0;
	int puertoNodo;
	struct in_addr ipJob;
	t_bool conCombiner;
	t_reduceLocal reduceLocal;

	switch(header.tipoMensaje)
	{
	case PedidoBloque:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "File System");
						requestedBlock.numero_bloque = umpackBlockNum(connection);
						requestedBlock.datos = malloc(tamanio_bloque);
						getBloque(&requestedBlock, espacioDatos);
						sendBlock(requestedBlock, fsIp, fsPuerto);
						close(connection.connectionSocket);

						break;

	case SeteoBloque:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "File System");
						newBlock = umpackBlock(connection);
						newBlock.datos = malloc(tamanio_bloque);
						memset(newBlock.datos,'a',tamanio_bloque);
						setBloque(newBlock, espacioDatos);
						close(connection.connectionSocket);
						free(newBlock.datos);
						break;

	case PedidoArchivo:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "File System");
						requestedFileName = umpackFileName(connection);
						content = getFileContent(requestedFileName, directorioTemporal);
						sendFileContent(requestedFileName, content, fsIp, fsPuerto);
						close(connection.connectionSocket);
						free(content);
						free(requestedFileName);

						break;

	case SolicitudDeMap:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "Job");
						logearYMostrarSolicitudMap(inet_ntoa(connection.connectionAddr.sin_addr));
						//Creo el archivo de script
						scriptFile = umpackFileName(connection);
						scriptFileDirectory = malloc(strlen(directorioTemporal) + strlen(scriptFile) + 1);
						string_append(&scriptFileDirectory, directorioTemporal); // concateno el directorio del espacio temporal
						string_append(&scriptFileDirectory, scriptFile); // concateno el directorio del espacio temporal con el nombre del archivo
						recv(connection.connectionSocket, (int*)&fileLength, sizeof(int), 0);
						content = malloc(fileLength);
						umpackFileContent(connection, content, fileLength);
						createFile(fileLength, scriptFileDirectory, content);

						//Creo el archivo de resultado
						resultFile = umpackFileName(connection);
						resultFileDirectory = malloc(strlen(directorioTemporal) + strlen(resultFile) + 1);
						string_append(&scriptFileDirectory, directorioTemporal); // concateno el directorio del espacio temporal
						string_append(&scriptFileDirectory, resultFile); // concateno el directorio del espacio temporal con el nombre del archivo
						createFile(fileLength, scriptFileDirectory, NULL);

						//Ejecuto el script de map
						executeMap(connection, scriptFileDirectory, espacioDatos, directorioTemporal);

						//Ordeno el archivo temporal
						sortTempFile(resultFileDirectory,directorioTemporal);

						//Notifico al job la finalizacion de la rutina map
						notifyEndOfMap(connection);
						close(connection.connectionSocket);

						break;

	case SolicitudDeReduceSinCombiner:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "Job");
						logearYMostrarSolicitudReduce(inet_ntoa(connection.connectionAddr.sin_addr));
						conCombiner = FALSE;
						receiveReduceRequest(connection, directorioTemporal, myip, miPuerto, conCombiner);

						break;

	case SolicitudDeReduceConCombiner:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "Job");
						logearYMostrarSolicitudReduce(inet_ntoa(connection.connectionAddr.sin_addr));
						conCombiner = TRUE;
						receiveReduceRequest(connection, directorioTemporal, myip, miPuerto, conCombiner);
						break;

	case PedidoArchivoAReducir:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "Nodo");
						requestedFileName = umpackFileName(connection);
						content = getFileContent(requestedFileName, directorioTemporal);
						recv(connection.connectionSocket, (struct in_addr*)&ipJob, sizeof(struct in_addr), 0);
						recv(connection.connectionSocket, (int*)&puertoNodo, sizeof(int), 0);
						sendFileToReduce(requestedFileName, content, ipJob, connection, puertoNodo);
						close(connection.connectionSocket);
						free(content);
						free(requestedFileName);

						break;

	case EnvioArchivoAReducir:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "Nodo");
						//Creo el archivo a reducir
						fileToReduce = umpackFileName(connection);
						fileToReduceDirectory = malloc(strlen(directorioTemporal) + strlen(fileToReduce) + 1);
						string_append(&scriptFileDirectory, directorioTemporal); // concateno el directorio del espacio temporal
						string_append(&scriptFileDirectory, fileToReduce); // concateno el directorio del espacio temporal con el nombre del archivo
						recv(connection.connectionSocket, (int*)fileLength, sizeof(int), 0);
						content = malloc(fileLength);
						umpackFileContent(connection, content, fileLength);
						createFile(fileLength, fileToReduceDirectory, content);

						//Actualizo el contador del job correspondiente
						recv(connection.connectionSocket, (struct in_addr*) &ipJob, sizeof(struct in_addr), 0);
						updateFileCounter(ipJob, directorioTemporal);
						close(connection.connectionSocket);

						break;

	case PedidoReduceLocal:
						logearYMostrarConexion(connection.connectionAddr.sin_port, inet_ntoa(connection.connectionAddr.sin_addr), "Nodo");
						logearYMostrarSolicitudReduceLocal(inet_ntoa(connection.connectionAddr.sin_addr));
						reduceLocal = reciveLocalReduce(connection, directorioTemporal);
						t_localReduceData* reduceData = malloc(sizeof(t_localReduceData));
						reduceData->reduce = reduceLocal;
						reduceData->tempDirectory = directorioTemporal;
						execLocalReduce(reduceData);
						logearYMostrarFinalizacionDeReduceLocal(inet_ntoa(connection.connectionAddr.sin_addr));
						recv(connection.connectionSocket, (int*)&puertoNodo, sizeof(int), 0);
						sendLocalReduceResult(connection, reduceLocal, directorioTemporal, puertoNodo);
						close(connection.connectionSocket);
						break;

	default: break;
	}

	return;
}
