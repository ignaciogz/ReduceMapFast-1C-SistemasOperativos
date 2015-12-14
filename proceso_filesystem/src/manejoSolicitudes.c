/*
 * manejoSolicitudes.c
 *
 *  Created on: 5/6/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>

#include "headers/socketsFs.h"
#include "headers/manejoSolicitudes.h"
#include "headers/Log.h"

#include "mongodb/headers/mongodb.h"

//El diccionario es GLOBAL, se declara y define en proceso_filesystem.c
extern t_dictionary * nodosHistorial;
extern int cantNodosActivos;
extern t_list* listaIp;
extern int Puerto_Marta;
extern char *IP_Marta;
extern pthread_mutex_t historialNodos;
extern pthread_mutex_t historialIP;



void* hiloSecundario(void* dataConnection)
{
	//close(0);
	//close(1);
	t_connection newConnection;
	int sockFs;
	socklen_t sin_size;
	sin_size = sizeof(struct sockaddr_in);
	sockFs=umpacksockFs(dataConnection);
	t_data* data;

	while(1){
		if((newConnection.connectionSocket =accept(sockFs, (struct sockaddr *)&newConnection.connectionAddr, &sin_size))==-1){
			perror("accept");
			continue;
		}

		void* buffer = malloc(sizeof(t_header));
		int nbytes;

		if((nbytes=recv(newConnection.connectionSocket, buffer, sizeof(t_header), 0)) <= 0){
					perror("recv hiloSec");
					}
		//Desempaqueto la cabecera
		t_header header;
		pthread_t newThread3;
		//void* paquete=malloc(sizeof(t_connection) + sizeof(t_header));


		umpackHeader(&header, buffer);
		//packDataRequest(header,newConnection,paquete);
		data = malloc(sizeof(t_data));

		data->connection = newConnection;
		data->header = header;

		pthread_create(&newThread3,NULL,(void*)&execRequest, data);

				}
return NULL;
}



void* execRequest(t_data* data)
{
	char *IP;
	t_request request;
	request.cabecera = data->header;
	request.conexion = data->connection;
	t_mensaje mensajito = request.cabecera.tipoMensaje;
	switch(mensajito)
		{
	case 7:  				logearYMostrarConexion(request.conexion.connectionAddr.sin_port,inet_ntoa(request.conexion.connectionAddr.sin_addr),"Proceso Nodo");
							t_conexionNodo conexionNodo;
							t_estado_nodo* paqueteNodo;
							bool existeNodo;
							t_boleano estado;
							long espacioNodo;
							int Puerto_N;
							struct sockaddr_in marta;
							recv(request.conexion.connectionSocket,(t_boleano*)&estado,sizeof(t_boleano),0);
							conexionNodo.nuevo=estado;
							recv(request.conexion.connectionSocket,(long*)&espacioNodo,sizeof(long),0);
							conexionNodo.espacioNodo=espacioNodo;
							recv(request.conexion.connectionSocket,(int*)&Puerto_N,sizeof(int),0);
							request.conexion.connectionAddr.sin_port=Puerto_N;

							int sockMartita;
							sockMartita=socket(AF_INET,SOCK_STREAM,0);

							pthread_mutex_lock( &historialNodos );
							IP=inet_ntoa(request.conexion.connectionAddr.sin_addr);
							existeNodo=dictionary_has_key(nodosHistorial,IP);



							if(existeNodo==false){

								dictionary_put(nodosHistorial,IP,nodo_create(request.conexion,conexionNodo));
								cantNodosActivos= cantNodosActivos + 1;
								pthread_mutex_unlock( &historialNodos );

								struct in_addr* aux = malloc(sizeof(struct in_addr));
								memcpy(aux, (struct in_addr*) &request.conexion.connectionAddr.sin_addr, sizeof(struct in_addr));

								pthread_mutex_lock( &historialIP );
								list_add(listaIp,aux);
								pthread_mutex_unlock( &historialIP );

								marta = inicializarDireccion(Puerto_Marta,IP_Marta);
								if(connect(sockMartita,(struct sockaddr *)&marta,sizeof(struct sockaddr))==-1)
										{exit(0);}
								t_header cabecera;
								int offset=0;
								cabecera.tipoMensaje=nodoNuevo;
								cabecera.tamanioMensaje= sizeof(struct in_addr) + sizeof(int);
								void* mensaje= malloc(sizeof(t_header) + sizeof(struct in_addr) + sizeof(int));
								packHeader(cabecera,mensaje);
								offset=offset + sizeof(t_header);
								memcpy(mensaje + offset,(struct in_addr*)&request.conexion.connectionAddr.sin_addr,sizeof(struct in_addr));
								offset=offset + sizeof(int);
								memcpy(mensaje + offset,(int*)&request.conexion.connectionAddr.sin_port,sizeof(int));

								if((sendall(sockMartita,mensaje,&cabecera.tamanioMensaje))==-1){
									logearYMostrarDesconexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");
								}

								close(sockMartita);

							}else{
								pthread_mutex_lock( &historialNodos );
								paqueteNodo=(t_estado_nodo*)dictionary_get(nodosHistorial, IP);
								pthread_mutex_unlock( &historialNodos );
								conectarNodo(paqueteNodo);}

							free(paqueteNodo);
							close(request.conexion.connectionSocket);
							break;

	case 1: 					 logearYMostrarConexion(request.conexion.connectionAddr.sin_port,inet_ntoa(request.conexion.connectionAddr.sin_addr),"Proceso MaRTA");
								 int tamNombreArch,tamanioHeader,indice;
						 	 	 char* nomArchivo;
						 	 	 t_header solicitud;
						 	 	 t_dictionary* copias=malloc(sizeof(t_dictionary));
						 	 	 t_copiaX* copia=malloc(sizeof(t_copiaX));

						 	 	 recv(request.conexion.connectionSocket,(int*)&indice,sizeof(int),0);
						 	 	 recv(request.conexion.connectionSocket,(int*)&tamNombreArch,sizeof(int),0);
						 	 	 nomArchivo =(char*) malloc(tamNombreArch);
						 	 	 recv(request.conexion.connectionSocket,nomArchivo,tamNombreArch,0);

						 	 	 //Inicializo el sock de marta para mandar resultado de los bloques
						 	 	 int sockMarta;
						 	 	 sockMarta=socket(AF_INET,SOCK_STREAM,0);

						 	 	 if(archivoEstaEnLaDB(nomArchivo)==0){
						 	 		 t_header cabecera;
						 	 		 cabecera.tipoMensaje=ErrorSolicitud;
						 	 		 cabecera.tamanioMensaje=sizeof(int);
						 	 		 void* mensajeError=malloc(sizeof(t_header) + sizeof(int));
						 	 		 packHeader(cabecera,mensajeError);
						 	 		 int offset= sizeof(t_header);
						 	 		 memcpy(mensajeError + offset,(int*)&indice,sizeof(int));
						 	 		 tamanioHeader=sizeof(t_header)+sizeof(int);
						 	 		 struct sockaddr_in marta_addr;
						 	 		 marta_addr=inicializarDireccion(Puerto_Marta,IP_Marta);

						 	 		 //Conecto a marta
						 	 		 connect(sockMarta,(struct sockaddr *)&marta_addr,sizeof(struct sockaddr_in));
						 	 		 if((sendall(sockMarta,mensajeError,&tamanioHeader))==-1){
						 	 			logearYMostrarDesconexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");
						 	 		 }
						 	 		 free(mensajeError);

						 	 	 }else{
						 	 		t_dictionary* bloques;
						 	 		int cantBloques,i,j,tamanioPaquete,tamanioCopia;
						 	 		int cantCopias=0;
						 	 		int ofset =0;
						 	 		char* key;
						 	 		bloques = getBloques_DB(nomArchivo);
						 	 		cantBloques = dictionary_size(bloques);

						 	 		for(i=0 ;i < cantBloques; i++){
						 	 			key = string_itoa(i);
						 	 			copias =(t_dictionary*) dictionary_get(bloques,key);
						 	 			cantCopias = cantCopias + dictionary_size(copias);
						 	 			free(key);
						 	 		}
						 	 		tamanioCopia= sizeof(struct in_addr) + sizeof(int) + sizeof(int);
						 	 		tamanioPaquete =  sizeof(int) + sizeof(int) + tamNombreArch + (cantCopias * tamanioCopia);

						 	 		solicitud.tipoMensaje=RespuestaSolicitudBloques;
						 	 		solicitud.tamanioMensaje=tamanioPaquete;
						 	 		void* mensaje = malloc(sizeof(t_header) + tamanioPaquete);
						 	 		packHeader(solicitud,mensaje);
						 	 		packSolicitudMarta(mensaje,indice,tamNombreArch,nomArchivo,cantCopias);
						 	 		ofset= sizeof(int) + sizeof(int) + sizeof(int) + tamNombreArch;

						 	 		for(i=0;i < cantBloques; i++)
						 	 		{
						 	 				key = string_itoa(i);
						 	 				copias = (t_dictionary*)dictionary_get(bloques, key);
						 	 				cantCopias = dictionary_size(copias);
						 	 				free(key);
						 	 				for(j=1;j <= cantCopias; j++)
						 	 				{
						 	 						key = string_itoa(j);
						 	 						copia = (t_copiaX*)dictionary_get(copias,key);

						 	 						memcpy(mensaje + ofset,(struct in_addr*)&copia->IP,sizeof(struct in_addr));
						 	 						ofset=ofset + sizeof(struct in_addr);

						 	 						memcpy(mensaje + ofset,(int *)&copia->puerto,sizeof(int));
						 	 						ofset=ofset + sizeof(int);

						 	 						memcpy(mensaje + ofset,(int*)&copia->numBloque,sizeof(int));
						 	 						ofset=ofset + sizeof(int);

													//logearBloqueDeArchivo(nomArchivo,copia);

						 	 						free(copia);
						 	 						free(key);
						 	 				}
						 	 				free(copias);
						 	 		}
						 	 		 //Conecto a marta
						 	 		struct sockaddr_in marta_addr;
						 	 		marta_addr=inicializarDireccion(Puerto_Marta,IP_Marta);
						 	 		connect(sockMarta,(struct sockaddr *)&marta_addr,sizeof(struct sockaddr_in));
						 	 		if((sendall(sockMarta,mensaje,&tamanioPaquete))==-1)
						 	 		{
						 	 			logearYMostrarDesconexion(Puerto_Marta,IP_Marta,"Proceso MaRTA");
						 	 		}
						 	 		//free(mensaje);
						 	 	 }
						 	 	 free(nomArchivo);
						 	 	 close(request.conexion.connectionSocket);
						 	 	 close(sockMarta);
						 	 	 break;

	case 3: 						logearYMostrarConexion(request.conexion.connectionAddr.sin_port,inet_ntoa(request.conexion.connectionAddr.sin_addr),"Proceso MaRTA");
								   int tamanioNombreArch,Puerto_Nodo,sockNodo,tamanioMensaje;
								   char* nombreArchivo;
								   struct in_addr IP_Nodo;
								   struct sockaddr_in nodo_addr;

								   recv(request.conexion.connectionSocket,(int*)&tamanioNombreArch,sizeof(int),0);
								   nombreArchivo =(char*) malloc(tamanioNombreArch);
								   recv(request.conexion.connectionSocket,nombreArchivo,tamanioNombreArch,0);
								   recv(request.conexion.connectionSocket, (struct in_addr*)&IP_Nodo,sizeof(struct in_addr),0);
								   recv(request.conexion.connectionSocket,(int*)&Puerto_Nodo,sizeof(int),0);

								   logearLecturaDeArchivo(nombreArchivo);
								   IP=inet_ntoa(IP_Nodo);
								   char* ipChar = malloc(sizeof(char)* strlen(IP));
								   memcpy(ipChar,IP, sizeof(char)* strlen(IP));
								   nodo_addr=inicializarDireccion(Puerto_Nodo,ipChar);
								   sockNodo=socket(AF_INET,SOCK_STREAM,0);
								   if(connect(sockNodo,(struct sockaddr *)&nodo_addr,sizeof(struct sockaddr))==-1)
								   		{exit(0);}

								   t_estado_nodo* nodoDicc;
								   pthread_mutex_lock( &historialNodos );
								   nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,ipChar);
								   pthread_mutex_unlock( &historialNodos );
								   verificarConexionNodo(nodoDicc);
								   t_header cabecera;
								   cabecera.tipoMensaje=getFileContent;
								   cabecera.tamanioMensaje=sizeof(int)+tamanioNombreArch;

								   void* mensaje=malloc(sizeof(t_header)+sizeof(int)+tamanioNombreArch);
								   packHeader(cabecera,mensaje);

			   					   packFileContent(tamanioNombreArch,nombreArchivo,mensaje);
								   tamanioMensaje=sizeof(t_header)+sizeof(int)+tamanioNombreArch;
								   if((sendall(sockNodo,mensaje,&tamanioMensaje))==-1){
									    desconectarNodo(nodoDicc);
								   }
								   free(nombreArchivo);

								   int tamanioContenido;
								   void* buffer = malloc(sizeof(t_header));

								   recv(sockNodo,buffer,sizeof(t_header),0);
								   recv(sockNodo,(int*)&tamanioNombreArch,sizeof(int),0);
								   nombreArchivo =(char*)malloc(tamanioNombreArch);
								   recv(sockNodo,nombreArchivo,tamanioNombreArch,0);
								   recv(sockNodo,(int*)&tamanioContenido,sizeof(int),0);
								   char* contenido= malloc(tamanioContenido);
								   recv(sockNodo,contenido,tamanioContenido,0);
								   char* path= malloc((13*sizeof(char) + tamanioNombreArch));
								   path="./resultados/";
								   strcat(path,nombreArchivo);
								   createFile(tamanioContenido,path,contenido);
								   logearEscrituraDeArchivo(nombreArchivo);

								   free(nombreArchivo);
								   free(ipChar);
								   free(contenido);
								   close(request.conexion.connectionSocket);
								   close(sockNodo);
								   break;

	default: break;

	}
free(data);
return NULL;

}



void execConexion(t_header header, t_connection conexion)
{
	char *IP;
	if(header.tipoMensaje==7){
		logearYMostrarConexion(conexion.connectionAddr.sin_port,inet_ntoa(conexion.connectionAddr.sin_addr),"Proceso Nodo");
		t_conexionNodo conexionNodo;
		t_estado_nodo* paqueteNodo;
		t_boleano estado;
		long espacioNodo;
		bool existeNodo;
		int Puerto_N;
		recv(conexion.connectionSocket,(t_boleano*)&estado,sizeof(t_boleano),0);
		conexionNodo.nuevo=estado;
		recv(conexion.connectionSocket,(long*)&espacioNodo,sizeof(long),0);
		conexionNodo.espacioNodo=espacioNodo;
		recv(conexion.connectionSocket,(int*)&Puerto_N,sizeof(int),0);
		conexion.connectionAddr.sin_port=Puerto_N;
		pthread_mutex_lock( &historialNodos );
		IP=inet_ntoa(conexion.connectionAddr.sin_addr);
		existeNodo=dictionary_has_key(nodosHistorial,IP);

		if(existeNodo==false){
					dictionary_put(nodosHistorial,IP,nodo_create(conexion,conexionNodo));
					pthread_mutex_unlock( &historialNodos );

					struct in_addr* aux = malloc(sizeof(struct in_addr));
					memcpy(aux, (struct in_addr*) &conexion.connectionAddr.sin_addr, sizeof(struct in_addr));

					pthread_mutex_lock( &historialIP );
					list_add(listaIp,aux);
					pthread_mutex_unlock( &historialIP );

			}else{
					pthread_mutex_lock( &historialNodos );
					paqueteNodo=(t_estado_nodo*)dictionary_get(nodosHistorial,IP);
					pthread_mutex_unlock( &historialNodos );
					conectarNodo(paqueteNodo);
				 }


	close(conexion.connectionSocket);

	}
}
