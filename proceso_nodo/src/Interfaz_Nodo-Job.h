/*
 * Interfaz_Nodo-Job.h
 *
 *  Created on: 28/6/2015
 *      Author: utnso
 */


#ifndef INTERFAZ_NODO_JOB_H_
#define INTERFAZ_NODO_JOB_H_

#include <commons/collections/list.h>

#include "Interfaz_Nodo_FileSystem.h"
#include "ManejoDeSockets.h"

typedef enum {TRUE = 0, FALSE} t_bool;

typedef struct{
	char* blockData;
	int pipe;
} t_blockToMap;

typedef struct{
	int fd;
	int pipe;
}t_scriptResult;

typedef struct {
	struct in_addr ipNodo;
	int puertoNodo;
	char* nombreArchivo;
	char* ultimaLineaLeida;
	long byteProximaLinea;
	long tamanioArchivo;
	t_list* archivosDelNodo; //lista utilizada para los reduce con combiner, guardara los nombres de los archivos que debe reducir localmente el nodo
}t_archivoAReducir;

typedef struct {
	t_list* archivosAReducir;
	int cantidadArchivosFaltantes;
	char* archivoReducer;
	char* archivoResultado;
	struct in_addr ipJob;
	int puertoJob;
	t_archivoAReducir* proximoArchivoAReducir;
}t_reduce;

typedef struct{
	t_reduce* reduce;
	char* tempDirectory;
	int pipe;
}t_reduceData;

typedef struct{
	char* nombreArchivo;
	char* ultimaLineaLeida;
	long largoArchivo;
	long byteProximaLinea;
}t_archivoAReducirLocalmente;

typedef struct{
	char* archivoReducer;
	char* archivoResultado;
	t_list* archivosAReducir;
	struct in_addr ipJob;
	t_archivoAReducirLocalmente* proximaReduccion;
}t_reduceLocal;

typedef struct{
	t_reduceLocal reduce;
	char* tempDirectory;
	int pipe;
}t_localReduceData;

// lista global de reduce
extern t_list* reducePendientes;

//Se crea un archivo copiandole los datos
void createFile(int fileLength, char* path, char* content);

//Ejecuta el script de map y almacena el resultado en un archivo temporal
void executeMap(t_connection connection, char* scriptPath, char* espacioDatos, char* espacioTemporal);

//Ordena el archivo temporal y almacena el resultado ordenado en el archivo de resultado
void sortTempFile(char* resultFile, char* tempDirectory);

//Notifica al job que se termino el map
void notifyEndOfMap(t_connection connection);

//Recibe un pedido de reduce de parte de un nodo, arma la estructura conrrespondiente y la guarda en la lista de reduce pendientes
void receiveReduceRequest(t_connection connection, char* tempDirectory, struct in_addr myip, int miPuerto, t_bool conCombiner);

//Recibe los archivos que se van a reducir agrupandolos por nodo para luego enviar la solicitud de reduce local
void receiveFilesToReduceWithCombiner(t_connection connection, t_list* filesList);

//Recibe los archivos que se van a reducir y los guarda en la lista de archivos del reduce, devuelve la cantidad de archivos ajenos al nodo en que se esta ejecutando el reduce
int receiveFilesToReduce(t_connection connection, t_list* filesList, struct in_addr myip, int miPuerto, struct in_addr ipJob);

//Recibe cada uno de los archivos con los datos de su nodo
t_archivoAReducir receiveNewFileToReduce(t_connection connection);

//Envia la solicitud del archivo a reducir a un nodo
void sendFileRequest(t_archivoAReducir file, struct in_addr ipJob, int miPuerto);

//Actualiza el contador de archivos pendientes de ser recibido de la lista de reduce pendientes
void updateFileCounter(struct in_addr ip, char* tempDirectory);

//Dado los datos de un bloque y el extremo del pipe a escribir manda el contenido del bloque por el pipe al script de map
void sendBlockToMap(t_blockToMap* blockToMap);

//Recive por el pipe el resultado del map y lo almacena en el archivo temporal;
void reciveScriptResult (t_scriptResult* scriptResultData);

//Obtiene la siguiente linea a reducir
void getFirstLineToReduce(t_reduce* reduce, char* tempDirectory);

//Obtiene la siguiente linea de un archivo
void getNextLineOfFile (t_archivoAReducir* file, char* tempDirectory, long byteToStartReading);

//Envia una linea por el pipe;
void sendSingleLineToReduce(char* line, int pipe);

//Recorre la lista de archivos y elige al proximo a reducir
void chooseNextFileToReduce(t_reduce* reduce);

//Dado un archivo y una lista lo elimina de la misma
void deleteFileFromList(t_archivoAReducir* file, t_list* filesList);

//Envia las lineas de los archivos del job al script de reduce
void sendLinesToReduce(t_reduceData* reduceData);

//Ejecuta el script de reduce
void execReduce (t_reduceData* reduceData);

//Devuelve el largo de un archivo em bytes
long getFileLength (char* fileName, char* tempDirectory);

//Notifica al job que se termino el reduce
void notifyEndOfReduce(struct in_addr ipJob, int puertoJob);

//Devuelve el largo de los nombre de los archivos a reducir de un nodo y la suma del tamaño de los ints uno por cada archivo
int nodeFilesLength(t_archivoAReducir* node);

//Empaqueta el mensaje de pedido de reduce local para un nodo
//void packLocalReduceRequest(void* message, char* reduceFileContent, char* localResultFileName, t_reduce reduce, t_archivoAReducir* node);

//Envia a cada nodo la solicitud de reduce local
void sendLocalReduceRequest(t_reduce reduce, char* reduceFileContent, struct in_addr myip, int miPuerto, char* tempDirectory);

void packLocalReduceRequest(void* message, char* reduceFileContent, char* localResultFileName, t_reduce reduce, t_archivoAReducir* node , int miPuerto);

//Recive la solicitud de reduce local
t_reduceLocal reciveLocalReduce(t_connection connection, char* tempDirectory);

//Recive los nombres de los archivos a reducir localmente;
void reciveFilesNamesToReduce(t_connection connection, t_list* list, int listSize, char* tempDirectory);

//Obtiene la siguiente linea de un archivo
void getNextLineToReduceLocaly(t_archivoAReducirLocalmente* file);

//Envia las lineas de los archivos a reducir
void sendLinesToLocalReduce(t_localReduceData* reduceData);

//Elige el siguiente archivo a reducir
void chooseNextFileToReduceLocaly(t_reduceLocal reduce);

//Elimina un archivo de la lista del reduce local
void deleteFileFromLocalReduceList(t_archivoAReducirLocalmente* fileToDelete, t_list* list);

//Ejecuta el reduce local del nodo
void execLocalReduce(t_localReduceData* reduceData);

//Envia el resultado del reduce local
void sendLocalReduceResult(t_connection connection, t_reduceLocal reduce, char* tempDirectory, int puertoNodo);

#endif /* INTERFAZ_NODO_JOB_H_ */
