/*
 ============================================================================
 Name        : filesystem.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "headers/socketsFs.h"
#include "headers/manejoSolicitudes.h"
#include "headers/Log.h"
#include "consola/headers/consola.h"
#include "headers/manejoDeDatos.h"
#define BACKLOG 10

//variables configuracion
t_config * config;
int Puerto_Listen, Puerto_Marta;//almaceno los puertos del arch config
int cantidadDeNodosNecesarios;//almaceno cant nodos de arch config
char *IP_Marta;
//Otras variab.
int cantNodosActivos=0;//Contador que guarda solo los nodos activos
int totalBloquesLibres=0;//Contador que almacena bloques libres total

t_dictionary * nodosHistorial;
t_list* listaIp;
t_log* logFs;

//SEMAFOROS GLOBALES
pthread_mutex_t historialNodos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t historialIP = PTHREAD_MUTEX_INITIALIZER;

int main(void)
{
	t_config * config;
	config = config_create("src/config.cfg");
	IP_Marta =(char*)malloc(15*sizeof(char));
	char*IP_FS=malloc(15*sizeof(char));
	printf("\033[33m************************************************************\n");
	printf("			DATOS DE CONFIGURACION          \n");
	printf("**************************************************************\033[37m\n");

	//-----------------------Datos del FS------------------------

	if(config_has_property(config,"PUERTO_LISTEN")){
	Puerto_Listen = config_get_int_value(config,"PUERTO_LISTEN");
	printf("El puerto listen es:%i\n",Puerto_Listen);

	}else{
		printf("No se encuentra el puerto Fs en el archivo\n");
		}

	if(config_has_property(config,"LISTA_NODOS")){
	cantidadDeNodosNecesarios = config_get_int_value(config, "LISTA_NODOS");
	printf("La cantidad de nodos activos:%i\n",cantidadDeNodosNecesarios);
	}else{
		printf("No se encuentra la cantidad de nodos en el archivo\n");
		}
	if(config_has_property(config,"IP_FS")){
	IP_FS = config_get_string_value(config, "IP_FS");
	printf("El IP del FS es:%s\n",IP_FS);
	}else{
		printf("No se encuentra el IP FS en el archivo\n");
		}

	//--------------------Datos de Marta--------------------------
	if(config_has_property(config,"PUERTO_MARTA")){
	Puerto_Marta = config_get_int_value(config,"PUERTO_MARTA");
	printf("El puerto marta es:%i\n",Puerto_Marta);
	}else{
		printf("No se encuentra el puerto marta en el archivo\n");
		}

	if(config_has_property(config,"IP_MARTA")){
	IP_Marta = config_get_string_value(config, "IP_MARTA");
	printf("El IP de marta es:%s\n",IP_Marta);

	}else{
		printf("No se encuentra el IP marta en el archivo\n");
		 }


	//Creo el logger del Fs

	logFs = log_create("LoggerFs.txt", "Proceso FileSystem", false, LOG_LEVEL_INFO);


	//variables socket servidor
	int sockFs; //sockFs escucho
	struct sockaddr_in fs_addr, marta_addr;//marta_addr es donde voy a guardar datos del arch config


	//Creamos socket del Fs

	if((sockFs = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("error");
	}

	fs_addr = inicializarDireccion(Puerto_Listen,IP_FS);

	if(bind(sockFs,(struct sockaddr *)&fs_addr,sizeof(struct sockaddr))==-1)
	{perror("bind");
	exit(0);
	}

	if(listen(sockFs,BACKLOG)==-1)
	{perror("listen");
	exit(0);
	}

	t_connection newConnection;
	socklen_t  sin_size;
	sin_size = sizeof(struct sockaddr_in);
	nodosHistorial = dictionary_create();
	dictionary_clean(nodosHistorial);
	listaIp = list_create();
	cantNodosActivos = 0;
	int nbytes;
	do{
		if((newConnection.connectionSocket =accept(sockFs, (struct sockaddr *)&newConnection.connectionAddr, (socklen_t*)&sin_size))==-1){
					perror("accept");
					continue;
				}

		void* buffer = malloc(sizeof(t_header));
		if((nbytes=recv(newConnection.connectionSocket, buffer, sizeof(t_header), 0)) <= 0){
			perror("recv hilo");

		}else{

		t_header header;
		//Desempaqueto la cabecera
		umpackHeader(&header, buffer);

		execConexion(header,newConnection);

		cantNodosActivos=dictionary_size(nodosHistorial);
		printf("Conexion Nodo Aceptado\n");
		free(buffer);
		}
	}while(cantidadDeNodosNecesarios >  cantNodosActivos);

	printf("\033[33m============================================================\n");
	printf("				OPERATIVO\n");
	printf("============================================================\033[37m\n");

	//paquete donde almaceno el sock del Fs,diccionario nodos
	void* dataNewConnection;
	dataNewConnection = malloc(sizeof(t_connection)+ sizeof(t_dictionary)+sizeof(int));
	packsockFs(sockFs, dataNewConnection);
	marta_addr=inicializarDireccion(Puerto_Marta,IP_Marta);
	void* mensaje=malloc(sizeof(t_header)+ sizeof(int) + (cantNodosActivos*sizeof(int)) + (cantNodosActivos*sizeof(struct in_addr)));
	t_header cabecera;


	//--------------------Mensaje a Marta con los IP de nodos conectados-----------------
	cabecera.tipoMensaje=ConexionFileSystem;
	cabecera.tamanioMensaje= sizeof(t_header) + sizeof(int)  + (cantNodosActivos*sizeof(int)) + (cantNodosActivos*sizeof(struct in_addr));
	packHeader(cabecera,mensaje);
	int offset=sizeof(t_header);
	int sockMarta;//socket conexion marta
	struct in_addr* Ip=malloc(sizeof(struct in_addr));



	memcpy(mensaje + offset,(int*)&cantNodosActivos,sizeof(int));
	offset = offset + sizeof(int);
	packIpPuertoNodo(mensaje,offset,Ip);

	//------------Conexion a Marta---------------------------------------------
	sockMarta=socket(AF_INET,SOCK_STREAM,0);
	while (connect(sockMarta,(struct sockaddr *)&marta_addr,sizeof(struct sockaddr))==-1)
	{;}

	printf("Conexion MaRTA Aceptada\n");
	logearYMostrarConexion(Puerto_Marta, IP_Marta, "Proceso MaRTA");
	if((sendall(sockMarta,mensaje,&cabecera.tamanioMensaje))==-1){
		logearYMostrarDesconexion(Puerto_Marta, IP_Marta, "Proceso MaRTA");
	}


	pthread_t newThread,newThread2;
	//----Ejecuto Hilo para seguir escuchando conexiones---------
	pthread_create(&newThread2,NULL,(void*)&hiloSecundario,(void*)dataNewConnection);

	//----Ejecuto Hilo Consola----------
	pthread_create(&newThread,NULL,(void*)&consola,NULL);

	pthread_join(newThread2,NULL);
	return 0;
}



