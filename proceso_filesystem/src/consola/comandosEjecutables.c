/*
 * comandosEjecutables.c
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/
#include "headers/comandosEjecutables.h"
#include "headers/comandosEjecutablesSecundarios.h"
#include "../mongodb/headers/mongodb.h"
#include "../headers/manejoDeDatos.h"
#include "../headers/socketsFs.h"
#include "../headers/Log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <pthread.h>

/*Para directorios*/
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> //Para elimanar directorio

extern t_dictionary * nodosHistorial;
extern t_list* listaIp;

extern pthread_mutex_t historialNodos;
extern pthread_mutex_t historialIP;
/*
	///////////////////////////////////////////////////////////
	COMANDOS DE DIRECTORIO:
	///////////////////////////////////////////////////////////
*/
	void ejecutarCrearDirectorio(char* rutaDelDirectorio){
		if(mkdir(rutaDelDirectorio, S_IRWXU | S_IRWXG | S_IRWXO)!=0){
			printf("Error al crear la carpeta\n");
		}
		else printf("Sea ha creado la carpeta: %s\n", rutaDelDirectorio);

		return;
	}


	void ejecutarRenombrarDirectorio(char* nombreViejoDelDirectorio, char* nombreNuevoDelDirectorio){
		if (rename(nombreNuevoDelDirectorio, nombreNuevoDelDirectorio)){
			printf( "\nNo pudo ser renombrado\n" );
		}else {
			printf( "renombrado a: %s\n", nombreViejoDelDirectorio );
			//FALTA Actualizar el dir padre en la DB de dicho archivo
			//EN BASE AL SEGUNDO PARAMETRO: rutaNueva
		}
		return;
	}


	void ejecutarEliminarDirectorio(char* rutaDelDirectorio){
	//OJO EL DIRECTORIO DEBE ESTAR VACIO PARA ELIMINARLO
		if(rmdir(rutaDelDirectorio) == 0){
			printf("Sea ha eliminado la carpeta: %s\n", rutaDelDirectorio);
			return;
		} else printf("Error al crear la carpeta\n");
		return;
	}

	void ejecutarMoverDirectorio(char* rutaDelDirectorio, char* rutaNuevaDelDirectorio){
		if (rename(rutaDelDirectorio, rutaNuevaDelDirectorio)){
			printf( "\nNo pudo ser movido el archivo\n" );
		}else {
			printf( "Archivo movido a: %s\n", rutaNuevaDelDirectorio );
			// FALTA Actualizar el dir padre en la DB
			// De cada Archivo que ya este en la DB !!!!! Verificar estaEnLaDB
			// EN BASE AL SEGUNDO PARAMETRO: rutaNueva
		}
	}

/*
	///////////////////////////////////////////////////////////
	COMANDOS DE ARCHIVOLOCAL:
	///////////////////////////////////////////////////////////
*/
	void ejecutarMoverArchivo(char* rutaVieja, char* rutaNueva){
		if (rename(rutaVieja, rutaNueva)){
			printf( "\nNo pudo ser movido el archivo\n" );
		}else {
			printf( "Archivo movido a: %s\n", rutaNueva );
			//FALTA Actualizar el dir padre en la DB de dicho archivo
			//EN BASE AL SEGUNDO PARAMETRO: rutaNueva
		}
	}

	void ejecutarRenombrarArchivo(char* nombreViejo, char* nombreNuevo){
		if (rename(nombreViejo, nombreNuevo)){
			printf( "\nNo pudo ser renombrado\n" );
		}else {
			printf( "renombrado a: %s\n", nombreNuevo );
			//FALTA Actualizar el dir padre en la DB de dicho archivo
			//EN BASE AL SEGUNDO PARAMETRO: rutaNueva
		}
	}

	void ejecutarEliminarArchivo(char* rutaDeUnArchivo){
		if(!remove(rutaDeUnArchivo)){
			printf( "\nArchivo eliminado CORRECTAMENTE\n" );
		}
	    else{
	    	printf( "\nEl archivo NO pudo ser elimanado\n" );
		}
	}

	void ejecutarCopiarAlMDFS(char* nombreDelArch){
		if(cargarBloquesNodo(nombreDelArch))
		{
			printf( "\nArchivo fue cargado CORRECTAMENTE\n" );
		}else
		{
			printf( "\nLa carga del archivo FALLO !\n" );
		}
		return;
	}

/*
	///////////////////////////////////////////////////////////
	COMANDOS DE MDFS:
	///////////////////////////////////////////////////////////
*/
	void ejecutarFormatearMDFS(void){
		formatearDB();
	}

	void ejecutarCopiarAlFS(char* nombreDelArchivo, char* rutaDeAlmacenamiento){
		copiarAlFS(nombreDelArchivo, rutaDeAlmacenamiento); //Listo ya lo programe WIIIII
		return;
	}

	void ejecutarObtenerMD5(char* nombreDelArchivo){
		obtenerMD5(nombreDelArchivo); //Listo ya lo programe WIIIII
		return;
	}

/*
	///////////////////////////////////////////////////////////
	COMANDOS DE BLOQUE:
	///////////////////////////////////////////////////////////
*/
	void ejecutarVerBloquesDe(char* nombreDelArchivo){
		int resultado = buscarArchivoParaImprimirlo_DB (nombreDelArchivo);

		if(!resultado)
		{
			printf( "\nEl archivo no esta cargado al sistema\n" );
			return;
		}
	}

	void ejecutarCopiarBloquesDe(char* nombreDelArchivo, int bloqueAClonar)
	{
		int bloqueAEscribir;
		t_estado_nodo* nodoDicc;
		int numeroDeCopia;
		char * nCopia;
		t_nodo nodito;
		struct sockaddr_in nodo_seleccionado;
		int existeAlgunNodoActivo = 0;
		int bloqueAObtener;
		char * contenido = NULL;

		if(!archivoEstaEnLaDB(nombreDelArchivo))
		{
			printf( "\nEl archivo no esta cargado al sistema\n" );
			return;
		}

		t_dictionary * diccionarioDeBloques = getBloques_DB(nombreDelArchivo);

		char * nBloque = string_itoa(bloqueAClonar);
		t_dictionary * dicDeCopias = dictionary_get(diccionarioDeBloques, nBloque);

		int cantidadTotalDeCopias = dictionary_size(dicDeCopias);
		char ** ipHistorial;
		ipHistorial = malloc(sizeof(char*) * cantidadTotalDeCopias + 1);

		for (numeroDeCopia = 1; numeroDeCopia <= cantidadTotalDeCopias; numeroDeCopia++)
		{
			nCopia = string_itoa(numeroDeCopia);

			t_copiaX * unaCopia = (t_copiaX*)dictionary_get( dicDeCopias, nCopia );
			free(nCopia);

			//OBTENGO LOS DATOS CONTENIDOS EN LA COPIA:
				struct in_addr Ip;
				Ip.s_addr= (*unaCopia).IP;

				char* aux = inet_ntoa(Ip);
				char* ipChar = (char *)malloc(sizeof(char)* strlen(aux));
				memcpy(ipChar, aux, (sizeof(char)* strlen(aux)));

				free(aux);
				ipHistorial[numeroDeCopia] = ipChar;

				free(unaCopia);
		}

		for (numeroDeCopia = 1; numeroDeCopia <= cantidadTotalDeCopias; numeroDeCopia++)
		{
			nCopia = string_itoa(numeroDeCopia);

			t_copiaX * unaCopia = (t_copiaX*)dictionary_get( dicDeCopias, nCopia );
			free(nCopia);

			//VEO CUAL NODO QUE POSEE EL BLOQUE, ESTA CONECTADO
				long int ip_NODO =  (*unaCopia).IP;
				int puerto_NODO = (*unaCopia).puerto;

				int sockNodoX;//socket conexion
				sockNodoX=socket(AF_INET,SOCK_STREAM,0);
				struct sockaddr_in nodo_addr;//nodo_addr es donde voy a guardar datos del Nodo

				nodo_addr = inicializarDireccionConsola(puerto_NODO,ip_NODO);

				if (connect(sockNodoX,(struct sockaddr *)&nodo_addr,sizeof(struct sockaddr))==-1)
				{
					free(unaCopia);

					close(sockNodoX);
					continue;
				}

			close(sockNodoX);
			//Guardo la INFO del nodo
			nodo_seleccionado = nodo_addr;
			bloqueAObtener = (*unaCopia).numBloque;
			free(unaCopia);

			existeAlgunNodoActivo = 1;
			break;
		}

		dictionary_clean(dicDeCopias);
		dictionary_destroy(dicDeCopias);
		dictionary_clean(diccionarioDeBloques);
		dictionary_destroy(diccionarioDeBloques);

	//OBTENGO EL BLOQUE:
		if(existeAlgunNodoActivo)
		{
			int sockNodoX;//socket conexion
			sockNodoX=socket(AF_INET,SOCK_STREAM,0);

			connect(sockNodoX,(struct sockaddr *)&nodo_seleccionado,sizeof(struct sockaddr));

			//--------------------Mensaje a el NODO que contiene la copia-----------------
			void* mensaje = malloc(sizeof(t_header)+sizeof(int));
			t_header cabecera;
			cabecera.tipoMensaje=GetBloque;
			cabecera.tamanioMensaje=sizeof(int);
			packHeader(cabecera,mensaje);
			int offset=sizeof(t_header);

			memcpy(mensaje + offset, (int*)&bloqueAObtener, sizeof(int));

			//--------------------Envio el mensaje al NODO X-----------------
			sendall(sockNodoX,mensaje,&cabecera.tamanioMensaje);

			//Elimino el mensaje enviado
			free(mensaje);

			//------------Espero y Obtengo la respuesta del NODO (el contenido)--------
			contenido = obtenerContenidoDelBloque(sockNodoX);
			close(sockNodoX);
		}

	//Si pude obtener el contenido del bloque, lo envio
		if(contenido)
		{
			// Obtengo y Ocupo el bloque del nodo donde voy a guardar la nueva copia
					nodito = nodoConPrioridadV2(ipHistorial, cantidadTotalDeCopias);
					char * IP_Nodito =inet_ntoa(nodito.Ip);

					pthread_mutex_lock( &historialNodos );
						nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,IP_Nodito);
					pthread_mutex_unlock( &historialNodos );
					bloqueAEscribir = obtenerBloqueLibre(nodoDicc->estadoBloques);
					bitarray_set_bit(nodoDicc->estadoBloques, bloqueAEscribir);

			//Envio al nodo disponible la info de la copia
				int sockNodoX;//socket conexion
				sockNodoX=socket(AF_INET,SOCK_STREAM,0);

				void* mensaje=malloc(sizeof(t_header) +(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int));

				t_header cabecera;
				cabecera.tipoMensaje=SetBloque;
				cabecera.tamanioMensaje=(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int);
				packHeader(cabecera,mensaje);

				int offset=0;
				offset=offset + sizeof(t_header);
				memcpy(mensaje + offset,(int*)bloqueAEscribir,sizeof(int));
				offset = offset + sizeof(int);
				memcpy(mensaje + offset, contenido, TAMANIO_BLOQUE);

				connect(sockNodoX,(struct sockaddr *)&nodo_seleccionado,sizeof(struct sockaddr));

				int tamanioPaquete;
				tamanioPaquete=sizeof(t_header)+(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int);
				sendall(sockNodoX,mensaje,&tamanioPaquete);

				close(sockNodoX);
				free(mensaje);

				long copia_IP = htonl(nodoDicc->datosNodo.connectionAddr.sin_addr.s_addr);
				int copia_Puerto = htons(nodoDicc->datosNodo.connectionAddr.sin_port);

				//agrego a la DB:
				agregarUnaCopiaAlBloqueX(nombreDelArchivo, bloqueAClonar, copia_IP, copia_Puerto, bloqueAEscribir);
				printf( "\nLa copia fue agregada CORRECTAMENTE\n" );
		}else{
			printf( "\nError al copiar el bloque\n" );
		}
		return;
	}

	void ejecutarBorrarBloquesDe(char* nombreDelArchivo, int numBloque, long ip_nodo, int puerto_nodo){
		//MIRAR PARAMETROS DE ESTA FUNCION
		int resultado = eliminarUnaCopiaAlBloqueX(nombreDelArchivo, numBloque, ip_nodo, puerto_nodo);

		if(resultado)
		{
			//Obtener de la DB el numDeBloque en el nodo, en base a ip_nodo puerto_nodo y nombreDelArchivo
			struct in_addr Ip;
			Ip.s_addr= htonl(ip_nodo);

			char* aux = inet_ntoa(Ip);
			char* ipChar = malloc(sizeof(char)* strlen(aux));
			memcpy(ipChar, aux, sizeof(char)* strlen(aux));

			free(aux);

		//Lo marco como vacio en nodosHistorial
			t_estado_nodo* nodoDicc;
			pthread_mutex_lock( &historialNodos );
				nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,ipChar);
			pthread_mutex_unlock( &historialNodos );

			//Obtener de la DB el numDeBloque en el nodo, en base a ip_nodo y nombreDelArchivo
			int numBloqueQueMeDaLaDB = getNumBloqueDeUnNodoEspecifico_ArchivoDB(nombreDelArchivo, numBloque, ip_nodo);

			bitarray_clean_bit(nodoDicc->estadoBloques, numBloqueQueMeDaLaDB);

			printf( "\nLa copia fue eliminada CORRECTAMENTE\n" );
		}
		else if (resultado == 0){
			printf( "\nEl archivo solicitado NO EXISTE\n" );
		}
		else if (resultado == -1){
			printf( "\nLa IP, el Puerto o el numBloque son INCORRECTOS\n" );
		}

		return;
	}

/*
	///////////////////////////////////////////////////////////
	COMANDOS DE NODO:
	///////////////////////////////////////////////////////////
*/
	void ejecutarAgregarNodo(char* IPdelNodo, int PuertoDelNodo, int EspacioDeDatos){
		bool existeNodo;

		pthread_mutex_lock( &historialNodos );
			existeNodo=dictionary_has_key(nodosHistorial,IPdelNodo);
		pthread_mutex_unlock( &historialNodos );

		if(!existeNodo){
				t_connection conexion;
				t_conexionNodo conexionNodo;
				conexion.connectionSocket= socket(AF_INET,SOCK_STREAM,0);
				conexion.connectionAddr= inicializarDireccion(PuertoDelNodo,IPdelNodo);
				conexionNodo.espacioNodo=EspacioDeDatos;
				conexionNodo.nuevo=0;

					pthread_mutex_lock( &historialNodos );
						dictionary_put(nodosHistorial, IPdelNodo, nodo_create(conexion,conexionNodo));
					pthread_mutex_unlock( &historialNodos );
					pthread_mutex_lock( &historialIP );
						list_add(listaIp,&conexion.connectionAddr.sin_addr);
					pthread_mutex_unlock( &historialIP );


					t_estado_nodo* nodoDicc;
					pthread_mutex_lock( &historialNodos );
						nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,IPdelNodo);
					pthread_mutex_unlock( &historialNodos );

					conectarNodo(nodoDicc);
		}else{
			printf("Ya esta el nodo nabo");
		}
	}

	void ejecutarEliminarNodo(char* IPdelNodo)
	{
		bool existeNodo;

		pthread_mutex_lock( &historialNodos );
			existeNodo=dictionary_has_key(nodosHistorial,IPdelNodo);
		pthread_mutex_unlock( &historialNodos );

		if(existeNodo)
		{
					struct in_addr * ipDeLaLista;
					int posicion;
					int i;

					pthread_mutex_lock( &historialNodos );
						t_estado_nodo* datoEliminable = (t_estado_nodo *) dictionary_remove(nodosHistorial, IPdelNodo);
					pthread_mutex_unlock( &historialNodos );

					pthread_mutex_lock( &historialIP );
						int cantidadDeElementoDeLaLista = list_size(listaIp);
					pthread_mutex_unlock( &historialIP );

					for(i=0; i < cantidadDeElementoDeLaLista; i++)
					{
						pthread_mutex_lock( &historialIP );
							ipDeLaLista = (struct in_addr *) list_get(listaIp, i);
						pthread_mutex_unlock( &historialIP );

						char* aux = inet_ntoa(*ipDeLaLista);
						char* ipChar = (char *)malloc(sizeof(char)* strlen(aux));
						memcpy(ipChar, aux, (sizeof(char)* strlen(aux)));

						if(strcmp(IPdelNodo, ipChar)==0){
							posicion = i;
							break;
						}

						free(aux);
						free(ipChar);
					}

					pthread_mutex_lock( &historialIP );
						struct in_addr * elementoAEliminar = (struct in_addr *) list_remove(listaIp, posicion);
					pthread_mutex_unlock( &historialIP );
					free(elementoAEliminar);

					desconectarNodo(datoEliminable);
					free(datoEliminable);

					printf( "\nEl nodo fue eliminado CORRECTAMENTE\n" );
		}else
		{
			printf( "\nEl nodo no EXISTE !\n" );
		}
	}
