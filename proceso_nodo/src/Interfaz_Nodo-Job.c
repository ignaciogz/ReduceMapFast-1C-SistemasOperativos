/*
 * Interfaz_Nodo-Job.c
 *
 *  Created on: 28/6/2015
 *      Author: utnso
 */


/*Common libraries */
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/error.h>
#include <commons/log.h>
#include <commons/process.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/txt.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>

/*Standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>/*biblioteca que contiene las funciones para manejo de sockets*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> /*biblioteca que contiene la funcion memset()*/
#include <pthread.h>
#include <sys/mman.h> /*biblioteca que contiene la funcion mmap() y munmap()*/
#include <unistd.h> /*biblioteca que contiene la funcion sysconf(_SC_PAGE_SIZE) y close()*/
#include <fcntl.h> /*biblioteca que contiene la funcion open()*/
#include "Interfaz_Nodo-Job.h"

#include <commons/collections/list.h>

#include "Interfaz_Nodo_FileSystem.h"
#include "ManejoDeSockets.h"
#include "LoggerNodo.h"


pthread_mutex_t pipesMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tempFileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reduceListMutex = PTHREAD_MUTEX_INITIALIZER;

void createFile(int fileLength, char* path, char* content)
{
	logearEscrituraDeArchivoTemporal(path);

	int fileDescriptor = open(path, O_WRONLY | O_CREAT); // abro el archivo donde se va a guardar el contenido al principio del mismo, en caso de no existir se creará

	char* mapped; // se define la variable sobre la que se mapeara el archivo

		/*se obtiene el area a mapeada */
		mapped = (char*) mmap(0, fileLength, PROT_WRITE, MAP_SHARED,
				fileDescriptor, 0);

		memcpy(mapped, content, fileLength);/*Se copia la informacion del archivo */

		munmap(mapped, fileLength); //libero los datos mapeados

		close(fileDescriptor); //cierro el archivo

		return;
}

void executeMap(t_connection connection, char* scriptPath, char* espacioDatos, char* espacioTemporal)
{

	int pipe1[2]; //pipe donde escribira el padre
	int pipe2[2]; //pipe donde escribira el hijo
	pid_t childPid;


	t_bloque_datos block;
	recv(connection.connectionSocket, (int*)block.numero_bloque, sizeof(int), 0); //recivo el numero de bloque a operar
	block.datos = malloc(tamanio_bloque);

	getBloque(&block, espacioDatos); //obtengo el bloque a operar

	char* path = malloc (strlen(espacioTemporal) + strlen("Temporal.txt"));
	string_append(&path, espacioTemporal); // concateno el directorio del espacio temporal
	string_append(&path, "Temporal.txt"); // concateno el directorio del espacio temporal con el nombre del archivo

	pthread_mutex_lock( &tempFileMutex );
	int tempFile = open(path, O_WRONLY | O_CREAT);

	do{}while(pipe(pipe1) != 0);
	do{}while(pipe(pipe2) != 0);

	childPid = fork();

	pthread_mutex_lock( &pipesMutex );

	t_blockToMap blockToMap;
	blockToMap.blockData = block.datos;
	blockToMap.pipe = pipe1[1];

	t_scriptResult scriptResultData;
	scriptResultData.pipe = pipe2[0];
	scriptResultData.fd = tempFile;

    if(childPid == 0)
    {
            /* Child stuff*/
    		close(pipe1[1]); //cierro el extremo de escritura del pipe1
    		close(0); //cierro stdin
    		dup(pipe1[0]); //redirecciono stdin al pipe1

    		close(pipe2[0]); //cierro el extremo de lectura del pipe2
    		close(1); //cierro el stdout
    		dup(pipe2[1]); // redirecciono stdout al pipe2

    		execlp(scriptPath,scriptPath,NULL); //Intercambio la imagen del hijo con el script de map



    }
    else
    {
            /* Parent stuff*/
    	close(pipe1[0]); //cierro el extremo de lectura del pipe1
    	close(pipe2[1]); //cierro el extremo de escritura del pipe2

    	//Defino el  hilo que se encargara de enviar el bloque al script de map
    	pthread_t newThread1;

    	//Creo el hilo nuevo
    	pthread_create( &newThread1, NULL, (void*) &sendBlockToMap, &blockToMap);

    	//Defino el  hilo que se encargara de recibir el resultado del map
    	pthread_t newThread2;

    	//Creo el hilo nuevo
    	pthread_create( &newThread2, NULL, (void*) &reciveScriptResult, &scriptResultData);

    	pthread_join( newThread1, NULL);
    	pthread_join( newThread2, NULL);

    }

    close(tempFile);
	pthread_mutex_unlock( &tempFileMutex );
    pthread_mutex_unlock( &pipesMutex );
    free(block.datos);
    free(path);

    return;

}

void sendBlockToMap(t_blockToMap* blockToMap)
{
	int sent = 0; //variable auxiliar para controlar el envio del bloque
	int recived = 0;

	do{
		recived = write(blockToMap->pipe, blockToMap->blockData + sent, tamanio_bloque - sent); // envio el contenido del bloque por el pipe

		if(recived > 0)
			sent = sent + recived;

	}while(sent < tamanio_bloque);

	return;
}

void reciveScriptResult (t_scriptResult* scriptResultData)
{

	int recived = -1; //variable auxiliar para controlar la recepcion del map, inicializo en valor negativo para no tener problemas con el control del read
	int copied = 0; //variable auxiliar para controlar la copia de los datos del map al archivo temporal
	int bytesInBuffer = 0; //variable auxiliar que dice cuantos bytes hay en el buffer listos a ser copiados
	char* buffer = malloc(100); //variable auxiliar para leer los datos del map

	do{
		if (recived != 0)
			recived = read(scriptResultData->pipe, buffer, 100); //leo lo que devuelve el script de a un maximo de 100 bytes simultaneos
		if (recived >= 0)
		{
			bytesInBuffer = bytesInBuffer + recived;
			copied =  write(scriptResultData->fd, buffer, bytesInBuffer); //copio el resultado del script al archivo
			bytesInBuffer = bytesInBuffer - copied;
		}
	}while((recived == 0) && (bytesInBuffer == 0));


	free(buffer);

	return;
}

void sortTempFile(char* resultFile, char* tempDirectory)
{
	char* path = malloc (strlen(tempDirectory) + strlen("Temporal.txt"));
	string_append(&path, tempDirectory); // concateno el directorio del espacio temporal
	string_append(&path, "Temporal.txt"); // concateno el directorio del espacio temporal con el nombre del archivo

	pthread_mutex_lock( &tempFileMutex );
	int tempFile = open(path, O_WRONLY | O_CREAT);
	int result = open(resultFile, O_WRONLY | O_CREAT);

	int pipe1[2], childPid;

	do{}while(pipe(pipe1) != 0);

	childPid = fork();

	pthread_mutex_lock( &pipesMutex );

	t_scriptResult scriptResultData;
	scriptResultData.pipe = pipe1[0];
	scriptResultData.fd = result;

    if(childPid == 0)
    {
            /* Child stuff*/
    		close(pipe1[0]); //cierro el extremo de lectura del pipe1
    		close(1); //cierro el stdout
    		dup(pipe1[1]); //redirecciono stdout al pipe1

    		execlp("sort", "sort", tempFile, NULL); //ejecuto el sort de bash para ordenar el archivo temporal, se envian los resusltados x el pipe debido a la redireccion




    }
    else
    {
    	/* Parent stuff*/

    	//Defino el  hilo que se encargara de recibir el resultado del map
    	pthread_t newThread;

    	//Creo el hilo nuevo
    	pthread_create( &newThread, NULL, (void*) &reciveScriptResult, &scriptResultData);

    	pthread_join( newThread, NULL);

    }

    close(tempFile);
    close(result);

	pthread_mutex_lock( &tempFileMutex );
    pthread_mutex_unlock( &pipesMutex );

    free(path);

	return;
}

void notifyEndOfMap(t_connection connection)
{
	logearYMostrarFinalizacionDeMap(inet_ntoa(connection.connectionAddr.sin_addr));

	char* message = malloc(sizeof(t_header));
	t_header header;

	header.tamanioMensaje = 0;
	header.tipoMensaje = FinalizacionDeMap;

	packHeader(header, message);


	sendAll(connection.connectionSocket, message, sizeof(t_header));

	return;
}

void receiveReduceRequest(t_connection connection, char* tempDirectory, struct in_addr myip, int miPuerto, t_bool conCombiner)
{
	t_reduce reduceRequest;

	char* reduceFileName;
	char* reduceFileDirectory;
	int fileLength = 0;
	char* fileContent;

	char* resultFileName;
	char* resultFileDirectory;

	//Desempaqueto el nombre del archivo reducer
	reduceFileName = umpackFileName(connection);
	reduceFileDirectory = malloc (strlen(tempDirectory) + strlen(reduceFileName));
	//concateno el nombre del archivo reducer con el directorio temporal
	string_append(&reduceFileDirectory, tempDirectory);
	string_append(&reduceFileDirectory, reduceFileName);
	//desempaqueto el contenido del archivo reduce y creo el archivo en el espacio temporal
	recv(connection.connectionSocket, (int*)fileLength, sizeof(int), 0);
	fileContent = malloc(fileLength);
	umpackFileContent(connection, fileContent, fileLength);
	createFile(fileLength, reduceFileDirectory, fileContent);
	//agrego el archivo reducer a la estructura de reduce
	reduceRequest.archivoReducer = reduceFileDirectory;

	//Desempaqueto el nombre del archivo resultado
	resultFileName = umpackFileName(connection);
	resultFileDirectory = malloc (strlen(tempDirectory) + strlen(resultFileName));
	//concateno el nombre del archivo resultado con el directorio temporal
	string_append(&reduceFileDirectory, tempDirectory);
	string_append(&reduceFileDirectory, reduceFileName);
	//agrego el archivo resultado a la estructura de reduce
	reduceRequest.archivoResultado = resultFileDirectory;

	//agrego el ip y el puerto del job a la estructura de reduce
	reduceRequest.ipJob = connection.connectionAddr.sin_addr;
	reduceRequest.puertoJob = connection.connectionAddr.sin_port;

	//inicializo los archivos a reducir
	reduceRequest.cantidadArchivosFaltantes = 0;
	reduceRequest.archivosAReducir = list_create();

	if(conCombiner == FALSE)
	{
		//recivo los archivos a reducir y defino la cantidad de archivos faltantes (ubicados en otros nodos)
		reduceRequest.cantidadArchivosFaltantes = receiveFilesToReduce(connection, reduceRequest.archivosAReducir, myip, miPuerto, reduceRequest.ipJob);
	}
	else
	{
		receiveFilesToReduceWithCombiner(connection, reduceRequest.archivosAReducir);
		sendLocalReduceRequest(reduceRequest, fileContent, myip, miPuerto, tempDirectory);
		reduceRequest.cantidadArchivosFaltantes = list_size(reduceRequest.archivosAReducir);

	}

	free(fileContent);

	//Inicializo el archivo candidato a proxima reduccion a NULL
	reduceRequest.proximoArchivoAReducir = NULL;

	pthread_mutex_lock( &reduceListMutex );

	//Agrego el reduce a la lista
	list_add(reducePendientes, &reduceRequest);

	pthread_mutex_unlock( &reduceListMutex );

	return;
}

void receiveFilesToReduceWithCombiner(t_connection connection, t_list* filesList)
{
	int cantidadDeArchivos = 0;
	int index = 0;
	int listLength;
	t_bool encontrado;
	char* ipNodo;

	recv(connection.connectionSocket, (int*) cantidadDeArchivos, sizeof(int), 0);

		t_archivoAReducir nuevoArchivo;
		t_archivoAReducir* auxFile;
		int i;

		for (i = 0; i < cantidadDeArchivos; i++)
		{
			nuevoArchivo = receiveNewFileToReduce(connection);

			ipNodo = malloc( strlen( inet_ntoa(nuevoArchivo.ipNodo)) + 1);
			strcpy( ipNodo, inet_ntoa(nuevoArchivo.ipNodo));

			index = 0;
			listLength = list_size(filesList);
			encontrado = FALSE;

			while((index < listLength) && (encontrado == FALSE))
			{
				auxFile = list_get(filesList, index);

				if(strcmp(ipNodo, inet_ntoa(auxFile->ipNodo)) == 0)
				{
					encontrado = TRUE;
					list_add(auxFile->archivosDelNodo, &nuevoArchivo.nombreArchivo);
					list_replace(filesList, index, auxFile);
				}

				index++;
			}

			if(encontrado == FALSE)
			{
				nuevoArchivo.archivosDelNodo = list_create();
				list_add(filesList, &nuevoArchivo);
			}

		}
}

int receiveFilesToReduce(t_connection connection, t_list* filesList, struct in_addr myip, int miPuerto, struct in_addr ipJob)
{
	int cantidadDeArchivos = 0;
	int cantidadArchivosDeOtrosNodos = 0;
	recv(connection.connectionSocket, (int*) cantidadDeArchivos, sizeof(int), 0);

	char* ipNodo = malloc( strlen( inet_ntoa(myip)) + 1);
	strcpy( ipNodo, inet_ntoa(myip));

	t_archivoAReducir nuevoArchivo;
	int i;

	for (i = 0; i < cantidadDeArchivos; i++)
	{
		nuevoArchivo = receiveNewFileToReduce(connection);
		if(strcmp(ipNodo, inet_ntoa(nuevoArchivo.ipNodo)) != 0)
		{
			cantidadArchivosDeOtrosNodos++;

			sendFileRequest(nuevoArchivo, ipJob, miPuerto);
		}

		list_add(filesList, &nuevoArchivo);

	}

	return cantidadArchivosDeOtrosNodos;
}

t_archivoAReducir receiveNewFileToReduce(t_connection connection)
{
	t_archivoAReducir nuevoArchivoAReducir;

	recv(connection.connectionSocket, &nuevoArchivoAReducir.ipNodo, sizeof(struct in_addr), 0);
	recv(connection.connectionSocket, (int*) nuevoArchivoAReducir.puertoNodo, sizeof(int), 0);
	nuevoArchivoAReducir.nombreArchivo = umpackFileName(connection);
	nuevoArchivoAReducir.ultimaLineaLeida = NULL;
	nuevoArchivoAReducir.byteProximaLinea = 0;

	return nuevoArchivoAReducir;
}

long getFileLength (char* fileName, char* tempDirectory)
{
	char* fileDirectory = malloc (strlen (fileName) + strlen(tempDirectory) + 1);

	string_append(&fileDirectory, tempDirectory);
	string_append(&fileDirectory, fileName);

	FILE* fileDescriptor = fopen(fileDirectory, "r"); //se abre el archivo solicitado para lectura
	long fileLength;

	fseek(fileDescriptor, 0, SEEK_END); //se posiciona el cursor del archivo sobre el final
	fileLength = ftell(fileDescriptor); //se devuelve la posicion en bytes en la que esta posicionado el cursor

	fclose(fileDescriptor);

	return fileLength;
}

void sendLocalReduceRequest(t_reduce reduce, char* reduceFileContent, struct in_addr myip, int miPuerto, char* tempDirectory)
{

	t_header header;
	int i = 0;
	int NodeListLength = list_size(reduce.archivosAReducir);

	char* localResultFile;
	void* message;

	char* ipNodo = malloc( strlen( inet_ntoa(myip)) + 1);
	strcpy( ipNodo, inet_ntoa(myip));

	t_archivoAReducir* auxNode;

	int sendSocket;
	struct sockaddr_in nodoAddres;

	t_localReduceData reduceData;

	//Creo el header del mesage
	header.tipoMensaje = PedidoReduceLocal;

	t_reduceLocal localReduce;

	for(i = 0; i < NodeListLength; i++)
	{
		auxNode = list_get(reduce.archivosAReducir, i);

		if(strcmp(ipNodo, inet_ntoa(auxNode->ipNodo)) == 0)
		{
			localResultFile = malloc(strlen("localResult") + strlen(inet_ntoa(auxNode->ipNodo)) + strlen(inet_ntoa(reduce.ipJob)) + 1);
			string_append(&localResultFile, "localResult");
			string_append(&localResultFile, inet_ntoa(auxNode->ipNodo));
			string_append(&localResultFile, inet_ntoa(reduce.ipJob));

			localReduce.archivoReducer = reduce.archivoReducer;
			localReduce.archivoResultado = localResultFile;
			localReduce.archivosAReducir = auxNode->archivosDelNodo;
			localReduce.ipJob = reduce.ipJob;
			localReduce.proximaReduccion = NULL;

			reduceData.reduce = localReduce;
			reduceData.tempDirectory = tempDirectory;

			//Defino el  hilo que se encargara de enviar las lineas al script de reduce
			pthread_t newThread1;

			//Creo el hilo nuevo
			pthread_create( &newThread1, NULL, (void*) &execLocalReduce, &reduceData);

			reduce.cantidadArchivosFaltantes--;

		}
		else
		{
		sendSocket = socket(AF_INET, SOCK_STREAM, 0);

		nodoAddres.sin_family = AF_INET;
		nodoAddres.sin_addr = auxNode->ipNodo;
		nodoAddres.sin_port = auxNode->puertoNodo;
		memset(&(nodoAddres.sin_zero), '\0', 8);

		localResultFile = malloc(strlen("locarResult") + strlen(inet_ntoa(auxNode->ipNodo)) + strlen(inet_ntoa(reduce.ipJob)) + 1);
		string_append(&localResultFile, "localResult");
		string_append(&localResultFile, inet_ntoa(auxNode->ipNodo));
		string_append(&localResultFile, inet_ntoa(reduce.ipJob));

		header.tamanioMensaje = sizeof(int)*5 + strlen(reduce.archivoReducer) + strlen(reduceFileContent) + strlen(localResultFile) + sizeof(struct in_addr) + nodeFilesLength(auxNode);

		message = malloc(header.tamanioMensaje + sizeof(t_header));

		packHeader(header, message);

		packLocalReduceRequest(message, reduceFileContent, localResultFile, reduce, auxNode, miPuerto);

		connect(sendSocket, (struct sockaddr*) &nodoAddres,sizeof(struct sockaddr_in));

		sendAll(sendSocket, message, (header.tamanioMensaje + sizeof(t_header)));

		auxNode->nombreArchivo = localResultFile;

		list_replace(reduce.archivosAReducir, i, auxNode);

		free(message);
		close(sendSocket);
		}
	}


	return;
}

int nodeFilesLength(t_archivoAReducir* node)
{
	int length = 0;
	int i;

	for(i = 0; i < list_size(node->archivosDelNodo); i++)
		length = length + strlen(list_get(node->archivosDelNodo, i));

	length = length + sizeof(int) * list_size(node->archivosDelNodo);

	return length;
}

void sendFileRequest(t_archivoAReducir file, struct in_addr ipJob, int miPuerto)
{
	t_header header;
	int offset = 0;

	int sendSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in nodoAddres;

		//Creo el header del mesage
		header.tipoMensaje = PedidoArchivoAReducir;
		header.tamanioMensaje = sizeof(int) + strlen(file.nombreArchivo) + 1 + sizeof(struct in_addr) + sizeof(int);
		//Aloco memoria para el mensaje y empaqueto cabecera
		void* message = malloc(sizeof(t_header) + header.tamanioMensaje);
		packHeader(header, message);
		//Empaqueto largo del nombre del archivo
		memcpy(message, (int*) strlen(file.nombreArchivo), sizeof(int));
		offset = sizeof(int);
		//Empaqueto el nombre del archivo
		memcpy(message + offset, file.nombreArchivo, strlen(file.nombreArchivo) + 1);
		offset = offset + strlen(file.nombreArchivo) + 1;
		memcpy(message + offset, (struct in_addr*)&ipJob, sizeof(struct in_addr));
		offset = offset + sizeof(struct in_addr);
		memcpy(message + offset, (int*)& miPuerto, sizeof(int));
		//Armo la estructura para la conexion con el nodo
		nodoAddres.sin_family = AF_INET;
		nodoAddres.sin_port = file.puertoNodo;
		nodoAddres.sin_addr = file.ipNodo;
		memset(&(nodoAddres.sin_zero), '\0', 8);

		connect(sendSocket, (struct sockaddr*)&nodoAddres, sizeof(struct sockaddr_in));
		//Envio el mensaje de pedido de archivo al nodo
		sendAll(sendSocket, message, sizeof(t_header) + header.tamanioMensaje);


		free(message);
		close(sendSocket);

	return;
}

void updateFileCounter(struct in_addr ip, char* tempDirectory)
{
	int index = 0;
	int listLength = list_size(reducePendientes);
	t_bool find = FALSE;

	t_reduce* reduce;

	t_reduceData reduceData;

	char* ipJob = malloc( strlen( inet_ntoa(ip)) + 1);
	strcpy( ipJob, inet_ntoa(ip));

	pthread_mutex_lock( &reduceListMutex );

	while((index < listLength) && (find == FALSE))
	{
		reduce = (t_reduce*) list_get(reducePendientes, index);
		if(strcmp(ipJob, inet_ntoa(reduce->ipJob)) != 0)
		{
			reduce->cantidadArchivosFaltantes = reduce->cantidadArchivosFaltantes - 1;

			list_replace(reducePendientes, index, reduce);

			if(reduce->cantidadArchivosFaltantes == 0) //lanzar reduce
			{
				reduceData.reduce = reduce;
				reduceData.tempDirectory = tempDirectory;

		    	//Defino el  hilo que se encargara de enviar las lineas al script de reduce
		    	pthread_t newThread1;

		    	//Creo el hilo nuevo
		    	pthread_create( &newThread1, NULL, (void*) &execReduce, &reduceData);

			}
			find = TRUE;
		}

		index++;
	}

	pthread_mutex_unlock( &reduceListMutex );

	return;
}

void execReduce (t_reduceData* reduceData)
{
	int pipe1[2]; //pipe donde escribira el padre
	int pipe2[2]; //pipe donde escribira el hijo
	pid_t childPid;

	int resultFile = open(reduceData->reduce->archivoResultado, O_WRONLY | O_CREAT);

	do{}while(pipe(pipe1) != 0);
	do{}while(pipe(pipe2) != 0);

	childPid = fork();

	pthread_mutex_lock( &pipesMutex );



	t_scriptResult scriptResultData;
	scriptResultData.pipe = pipe2[0];
	scriptResultData.fd = resultFile;

	reduceData->pipe = pipe1[1];

    if(childPid == 0)
    {
            /* Child stuff*/
    		close(pipe1[1]); //cierro el extremo de escritura del pipe1
    		close(0); //cierro stdin
    		dup(pipe1[0]); //redirecciono stdin al pipe1

    		close(pipe2[0]); //cierro el extremo de lectura del pipe2
    		close(1); //cierro el stdout
    		dup(pipe2[1]); // redirecciono stdout al pipe2

    		execlp(reduceData->reduce->archivoReducer,reduceData->reduce->archivoReducer,NULL); //Intercambio la imagen del hijo con el script de map



    }
    else
    {
            /* Parent stuff*/
    	close(pipe1[0]); //cierro el extremo de lectura del pipe1
    	close(pipe2[1]); //cierro el extremo de escritura del pipe2

    	//Defino el  hilo que se encargara de enviar las lineas al script de reduce
    	pthread_t newThread1;

    	//Creo el hilo nuevo
    	pthread_create( &newThread1, NULL, (void*) &sendLinesToReduce, reduceData);

    	//Defino el  hilo que se encargara de recibir el resultado del reduce
    	pthread_t newThread2;

    	//Creo el hilo nuevo
    	pthread_create( &newThread2, NULL, (void*) &reciveScriptResult, &scriptResultData);

    	pthread_join( newThread1, NULL);
    	pthread_join( newThread2, NULL);

    }

	pthread_mutex_unlock( &pipesMutex );

	close(resultFile);

	notifyEndOfReduce(reduceData->reduce->ipJob, reduceData->reduce->puertoJob);



	return;
}

void sendLinesToReduce(t_reduceData* reduceData)
{
	getFirstLineToReduce(reduceData->reduce, reduceData->tempDirectory);

	do{
		sendSingleLineToReduce(reduceData->reduce->proximoArchivoAReducir->ultimaLineaLeida, reduceData->pipe);

		if(reduceData->reduce->proximoArchivoAReducir->byteProximaLinea < reduceData->reduce->proximoArchivoAReducir->tamanioArchivo)
			getNextLineOfFile(reduceData->reduce->proximoArchivoAReducir, reduceData->tempDirectory, reduceData->reduce->proximoArchivoAReducir->byteProximaLinea);
		else
		{
			deleteFileFromList(reduceData->reduce->proximoArchivoAReducir, reduceData->reduce->archivosAReducir);
			reduceData->reduce->proximoArchivoAReducir = NULL;
		}
		chooseNextFileToReduce(reduceData->reduce);

	}while(list_size(reduceData->reduce->archivosAReducir) > 0);

	return;
}

void sendSingleLineToReduce(char* line, int pipe)
{
	int sent = 0; //variable auxiliar para controlar el envio del bloque
	int recived = 0;
	int lineLength = strlen(line);

	do{
		recived = write(pipe, line + sent, lineLength - sent); // envio el contenido del bloque por el pipe

		if(recived > 0)
			sent = sent + recived;

	}while(sent < lineLength);

	return;
}

void getFirstLineToReduce(t_reduce* reduce, char* tempDirectory)
{
	int index = 1;
	int listLength = list_size(reduce->archivosAReducir);
	t_archivoAReducir* file;

	file = (t_archivoAReducir*) list_get(reduce->archivosAReducir, 0);

	getNextLineOfFile(file, tempDirectory, 0);

	file->tamanioArchivo = getFileLength (file->nombreArchivo, tempDirectory);

	list_replace(reduce->archivosAReducir, 0, file);

	reduce->proximoArchivoAReducir = file; //Inicializo el candidato a ser reducido

	while (index < listLength)
	{
		file = (t_archivoAReducir*) list_get(reduce->archivosAReducir, index);

		getNextLineOfFile(file, tempDirectory, 0);

		file->tamanioArchivo = getFileLength (file->nombreArchivo, tempDirectory);

		list_replace(reduce->archivosAReducir, index, file);

		if( strcmp(reduce->proximoArchivoAReducir->ultimaLineaLeida, file->ultimaLineaLeida) > 0)
			reduce->proximoArchivoAReducir = file;

	}

	return;

}

void getNextLineOfFile (t_archivoAReducir* file, char* tempDirectory, long byteToStartReading)
{
	file->ultimaLineaLeida = NULL;
	int lineSize = 0;
	char* fileDirectory = malloc( strlen(tempDirectory) + strlen(file->nombreArchivo)+ 1);
	string_append(&fileDirectory, tempDirectory);
	string_append(&fileDirectory, file->nombreArchivo);

	FILE* fd = fopen(fileDirectory, "r");

	fsetpos( fd, (fpos_t *) &byteToStartReading );

	getline(&file->ultimaLineaLeida, (size_t*) &lineSize, fd);

	file->byteProximaLinea = file->byteProximaLinea + strlen(file->ultimaLineaLeida);

	fclose(fd);

	return;
}

void chooseNextFileToReduce(t_reduce* reduce)
{
	int index = 0;
	int listLength = list_size(reduce->archivosAReducir);
	t_archivoAReducir* file;

	if(reduce->proximoArchivoAReducir == NULL)
	{
		index = 1;
		reduce->proximoArchivoAReducir = (t_archivoAReducir*) list_get(reduce->archivosAReducir, index);
	}


	while (index < listLength)
	{
		file = (t_archivoAReducir*) list_get(reduce->archivosAReducir, index);

		if( strcmp(reduce->proximoArchivoAReducir->ultimaLineaLeida, file->ultimaLineaLeida) > 0)
		reduce->proximoArchivoAReducir = file;

		}

	return;
}

void deleteFileFromList(t_archivoAReducir* file, t_list* filesList)
{
	int index = 0;
	t_bool encontrado = FALSE;
	int listLength = list_size(filesList);
	t_archivoAReducir* auxFile;

	while((index < listLength) && (encontrado == FALSE))
	{
		auxFile = (t_archivoAReducir*) list_get(filesList, index);

		if(strcmp(auxFile->nombreArchivo, file->nombreArchivo) == 0)
		{
			encontrado = TRUE;
			list_remove(filesList, index);
			free(auxFile);
		}
	}

	return;
}

void notifyEndOfReduce(struct in_addr ipJob, int puertoJob)
{
	logearYMostrarFinalizacionDeReduce(inet_ntoa(ipJob));

	char* message = malloc(sizeof(t_header));
	t_header header;

	struct sockaddr_in job;

	job.sin_port = puertoJob;
	job.sin_family = AF_INET;
	job.sin_addr = ipJob;
	memset(&(job.sin_zero), '\0', 8);

	int jobSocket;

	do{
		jobSocket = socket(AF_INET,SOCK_STREAM, 0);
	}while (jobSocket != -1);

	header.tamanioMensaje = 0;
	header.tipoMensaje = FinalizacionDeReduce;

	packHeader(header, message);

	int connectState;
	do{
		connectState = connect(jobSocket, (struct sockaddr*) &job, sizeof(struct sockaddr_in));
	}while(connectState != 0);

	sendAll(jobSocket, message, sizeof(t_header));

	return;
}


void removeJob(t_reduce* reduce)
{
	int index = 0;
	int listLength = list_size(reducePendientes);
	t_bool find = FALSE;

	t_reduce* auxReduce;

	char* ipJob = malloc( strlen( inet_ntoa(reduce->ipJob)) + 1);
	strcpy( ipJob, inet_ntoa(reduce->ipJob));

	pthread_mutex_lock( &reduceListMutex );

	while((index < listLength) && (find == FALSE))
	{
		auxReduce = (t_reduce*) list_get(reducePendientes, index);
		if(strcmp(ipJob, inet_ntoa(reduce->ipJob)) == 0)
		{
			list_remove(reducePendientes, index);
			free(auxReduce);
			find = TRUE;
		}

		index++;
	}

	pthread_mutex_unlock( &reduceListMutex );

	return;
}


void packLocalReduceRequest(void* message, char* reduceFileContent, char* localResultFileName, t_reduce reduce, t_archivoAReducir* node , int miPuerto)
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
	offset = offset + sizeof(struct in_addr);

	memcpy(message + offset, (int*) &miPuerto, sizeof(int));

	return;
}

t_reduceLocal reciveLocalReduce(t_connection connection, char* tempDirectory)
{
	t_reduceLocal reduce;
	char* reduceFileDirectory;
	int fileLength = 0;
	char* content;
	int files = 0;

	//Creo el archivo reducer
	reduce.archivoReducer = umpackFileName(connection);
	reduceFileDirectory= malloc(strlen(tempDirectory) + strlen(reduce.archivoReducer) + 1);
	string_append(&reduceFileDirectory, tempDirectory); // concateno el directorio del espacio temporal
	string_append(&reduceFileDirectory, reduce.archivoReducer); // concateno el directorio del espacio temporal con el nombre del archivo
	recv(connection.connectionSocket, (int*)fileLength, sizeof(int), 0);
	content = malloc(fileLength);
	umpackFileContent(connection, content, fileLength);
	createFile(fileLength, reduceFileDirectory, content);

	free (content);

	//Recivo el nombre del archivo resultado
	reduce.archivoResultado = umpackFileName(connection);

	//Recivo los nombres de los archivos a reducir
	recv(connection.connectionSocket, (int*) files, sizeof(int), 0);
	reciveFilesNamesToReduce(connection, reduce.archivosAReducir, files, tempDirectory);

	//Recibo la ip del job
	recv(connection.connectionSocket, (struct in_addr*) &reduce.ipJob, sizeof(struct in_addr), 0);

	return reduce;
}

void reciveFilesNamesToReduce(t_connection connection, t_list* list, int listSize, char* tempDirectory)
{
	t_archivoAReducirLocalmente file;
	int i;
	char* fileName;
	char* fileDirectory;

	for( i = 0; i < listSize; i++)
	{
		fileName= umpackFileName(connection);

		fileDirectory = malloc(strlen(tempDirectory) + strlen(fileName) + 1);
		string_append(&fileDirectory, tempDirectory); // concateno el directorio del espacio temporal
		string_append(&fileDirectory, fileName); // concateno el directorio del espacio temporal con el nombre del archivo

		file.nombreArchivo = fileDirectory;
		file.byteProximaLinea = 0;
		file.ultimaLineaLeida = NULL;
		file.largoArchivo = getFileLength(fileName, tempDirectory);


		list_add(list, &file);
	}

	return;
}

void execLocalReduce(t_localReduceData* reduceData)
{
	int pipe1[2]; //pipe donde escribira el padre
		int pipe2[2]; //pipe donde escribira el hijo
		pid_t childPid;

		char* resultFileDirectory = malloc(strlen(reduceData->tempDirectory) + strlen(reduceData->reduce.archivoResultado) + 1);
		string_append(&resultFileDirectory, reduceData->tempDirectory); // concateno el directorio del espacio temporal
		string_append(&resultFileDirectory, reduceData->reduce.archivoResultado); // concateno el directorio del espacio temporal con el nombre del archivo

		int resultFile = open(resultFileDirectory, O_WRONLY | O_CREAT);

		do{}while(pipe(pipe1) != 0);
		do{}while(pipe(pipe2) != 0);

		childPid = fork();

		pthread_mutex_lock( &pipesMutex );

		reduceData->pipe = pipe1[1];

		t_scriptResult scriptResultData;
		scriptResultData.pipe = pipe2[0];
		scriptResultData.fd = resultFile;



	    if(childPid == 0)
	    {
	            /* Child stuff*/
	    		close(pipe1[1]); //cierro el extremo de escritura del pipe1
	    		close(0); //cierro stdin
	    		dup(pipe1[0]); //redirecciono stdin al pipe1

	    		close(pipe2[0]); //cierro el extremo de lectura del pipe2
	    		close(1); //cierro el stdout
	    		dup(pipe2[1]); // redirecciono stdout al pipe2

	    		execlp(reduceData->reduce.archivoReducer,reduceData->reduce.archivoReducer,NULL); //Intercambio la imagen del hijo con el script de map



	    }
	    else
	    {
	            /* Parent stuff*/
	    	close(pipe1[0]); //cierro el extremo de lectura del pipe1
	    	close(pipe2[1]); //cierro el extremo de escritura del pipe2

	    	//Defino el  hilo que se encargara de enviar las lineas al script de reduce
	    	pthread_t newThread1;

	    	//Creo el hilo nuevo
	    	pthread_create( &newThread1, NULL, (void*) &sendLinesToLocalReduce, &reduceData);

	    	//Defino el  hilo que se encargara de recibir el resultado del reduce
	    	pthread_t newThread2;

	    	//Creo el hilo nuevo
	    	pthread_create( &newThread2, NULL, (void*) &reciveScriptResult, &scriptResultData);

	    	pthread_join( newThread1, NULL);
	    	pthread_join( newThread2, NULL);

	    }

		pthread_mutex_unlock( &pipesMutex );

		close(resultFile);

	return;
}

void sendLinesToLocalReduce(t_localReduceData* reduceData)
{
	t_archivoAReducirLocalmente* auxFile;
	int index = 1;

	auxFile = list_get(reduceData->reduce.archivosAReducir, 0);

	getNextLineToReduceLocaly(auxFile);

	list_replace(reduceData->reduce.archivosAReducir, 0, auxFile);

	reduceData->reduce.proximaReduccion = auxFile;

	while(index < list_size(reduceData->reduce.archivosAReducir))
	{
		auxFile = list_get(reduceData->reduce.archivosAReducir, index);

		getNextLineToReduceLocaly(auxFile);

		list_replace(reduceData->reduce.archivosAReducir, index, auxFile);

		if(strcmp(reduceData->reduce.proximaReduccion->ultimaLineaLeida, auxFile->ultimaLineaLeida) > 0)
			reduceData->reduce.proximaReduccion = auxFile;

		index++;
	}

	do{
		sendSingleLineToReduce(reduceData->reduce.proximaReduccion->ultimaLineaLeida, reduceData->pipe);

		if(reduceData->reduce.proximaReduccion->byteProximaLinea < reduceData->reduce.proximaReduccion->largoArchivo)
			getNextLineToReduceLocaly(reduceData->reduce.proximaReduccion);
		else
		{
			deleteFileFromLocalReduceList(reduceData->reduce.proximaReduccion, reduceData->reduce.archivosAReducir);
			reduceData->reduce.proximaReduccion = NULL;
		}

		chooseNextFileToReduceLocaly(reduceData->reduce);

	}while(list_size(reduceData->reduce.archivosAReducir) > 0);



	return;
}

void getNextLineToReduceLocaly(t_archivoAReducirLocalmente* file)
{
	file->ultimaLineaLeida = NULL;
	int lineSize = 0;

	FILE* fd = fopen(file->nombreArchivo, "r");

	fsetpos( fd, (fpos_t *) &file->byteProximaLinea);

	getline(&file->ultimaLineaLeida, (size_t*) &lineSize, fd);

	file->byteProximaLinea = file->byteProximaLinea + strlen(file->ultimaLineaLeida);

	fclose(fd);

	return;
}

void deleteFileFromLocalReduceList(t_archivoAReducirLocalmente* fileToDelete, t_list* list)
{
	int index = 0;
	t_bool encontrado = FALSE;
	int listLength = list_size(list);
	t_archivoAReducirLocalmente* auxFile;

		while((index < listLength) && (encontrado == FALSE))
		{
			auxFile = (t_archivoAReducirLocalmente*) list_get(list, index);

			if(strcmp(auxFile->nombreArchivo, fileToDelete->nombreArchivo) == 0)
			{
				encontrado = TRUE;
				list_remove(list, index);
				free(auxFile);
			}
		}

		return;
}

void chooseNextFileToReduceLocaly(t_reduceLocal reduce)
{
	int index = 0;
	int listLength = list_size(reduce.archivosAReducir);
	t_archivoAReducirLocalmente* file;

	if(reduce.proximaReduccion == NULL)
	{
		index = 1;
		reduce.proximaReduccion = (t_archivoAReducirLocalmente*) list_get(reduce.archivosAReducir, index);
	}


		while (index < listLength)
		{
			file = (t_archivoAReducirLocalmente*) list_get(reduce.archivosAReducir, index);

			if( strcmp(reduce.proximaReduccion->ultimaLineaLeida, file->ultimaLineaLeida) > 0)
			reduce.proximaReduccion = file;

			}

	return;
}

void sendLocalReduceResult(t_connection connection, t_reduceLocal reduce, char* tempDirectory, int puertoNodo)
{
	char* requestedFileName = reduce.archivoResultado;
	char* content = NULL;
	content = getFileContent(requestedFileName, tempDirectory);
	sendFileToReduce(requestedFileName, content, reduce.ipJob, connection, puertoNodo);

	return;
}


