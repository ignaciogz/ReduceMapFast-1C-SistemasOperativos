/*
 * funcionesManejoDeDatos.c
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>

#include "headers/socketsFs.h"
#include "headers/funcionesDeManejoDeDatos.h"

//Por si falla en algun conect. Para liberar los lugares ocupados!
extern t_dictionary * nodosHistorial;
extern t_dictionary* diccionarioDeBloques;


void agregarDatosCopiaNormal(t_estado_nodo* nodoDicc, int i, int bloqueAEscribir,int repeticiones){
	t_dictionary * dicDeCopias=malloc(sizeof(t_dictionary));
	char* nBloque = string_itoa(i);
	char* numCopia = string_itoa(repeticiones);
	int numeroPuerto;
	long numeroIP;

	dicDeCopias = (t_dictionary*)dictionary_get(diccionarioDeBloques, nBloque);
	//Transoformo valores a long para poder alamcenarlos luege a la DB
	numeroIP = ntohl((*nodoDicc).datosNodo.connectionAddr.sin_addr.s_addr);
	numeroPuerto =ntohs((*nodoDicc).datosNodo.connectionAddr.sin_port);

	agregaCopia(dicDeCopias, numCopia, numeroIP, numeroPuerto, bloqueAEscribir);
	return;
}

void agregaCopia(t_dictionary* dicDeCopias, char * numeroCopia, long IP, int PUERTO, int NUMBLOQUE)
{
	t_copiaX* copia;

	copia = registroCreate(IP, PUERTO, NUMBLOQUE);
	dictionary_put(dicDeCopias, numeroCopia, copia);
	return;
}

t_copiaX* registroCreate(long IP, int PUERTO, int NUMBLOQUE){
	t_copiaX* new=malloc(sizeof(t_copiaX));
	new->IP=IP;
	new->puerto=PUERTO;
	new->numBloque=NUMBLOQUE;
	return new;
}


void agregarDatosCopiaGrande(t_estado_nodo* nodoDicc, int i, int bloqueAEscribir,int repeticiones){
	t_dictionary* dicDeCopias;
	dicDeCopias = dictionary_create();
	int PUERTO;
	long IP;

	char* keyParaBloques = string_itoa(i);
	char* key = string_itoa(repeticiones);
	IP = ntohl((*nodoDicc).datosNodo.connectionAddr.sin_addr.s_addr);
	PUERTO =ntohs((*nodoDicc).datosNodo.connectionAddr.sin_port);

	agregaCopia(dicDeCopias, key, IP, PUERTO, bloqueAEscribir);
	dictionary_put(diccionarioDeBloques, keyParaBloques, dicDeCopias);
	free(key);
	free(keyParaBloques);
}



void LiberarEspacios()
//Liberando espacios ocupados (Dentro de los nodos) durante una transaccion//
{
	t_dictionary* dicDeCopias;
	int numeroDeBloque,numeroDeCopia;
	if(!dictionary_is_empty(diccionarioDeBloques))
	{
		int cantidadTotalDeBloques = dictionary_size(diccionarioDeBloques);

		//Ingresando a un bloque X
		for (numeroDeBloque = 0; numeroDeBloque < cantidadTotalDeBloques; numeroDeBloque++)
		{
			char *nBloque = string_itoa(numeroDeBloque);

			dicDeCopias = (t_dictionary *) dictionary_get( diccionarioDeBloques, nBloque );
			free(nBloque);

			int cantidadTotalDeCopias = dictionary_size( dicDeCopias );

			//Ingresando a una copia
				for (numeroDeCopia = 1; numeroDeCopia <= cantidadTotalDeCopias; numeroDeCopia++)
				{
					char * nCopia = string_itoa(numeroDeCopia);

					t_copiaX * unaCopia = (t_copiaX*)dictionary_get( dicDeCopias, nCopia );
					free(nCopia);

					//OBTENGO LOS DATOS CONTENIDOS EN LA COPIA:
						int numBloque_NODO = (*unaCopia).numBloque;
						struct in_addr Ip;
						Ip.s_addr= (*unaCopia).IP;

						char* aux = inet_ntoa(Ip);
						char* ipChar = malloc(sizeof(char)* strlen(aux));
						t_estado_nodo* nodoDicc;
						memcpy(ipChar, aux, sizeof(char)* strlen(aux));
						nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,ipChar);

						bitarray_clean_bit(nodoDicc->estadoBloques,numBloque_NODO);


					//Libero el struct de la copia;
					free(unaCopia);
					if(numeroDeCopia == cantidadTotalDeCopias)
					{
						dictionary_clean(dicDeCopias);
						dictionary_destroy(dicDeCopias);
					}
				}
		}
		dictionary_clean(diccionarioDeBloques);
		dictionary_destroy(diccionarioDeBloques);
	}return;
}

void agregarCopia1ConResto(t_estado_nodo* nodoDicc, int bloqueAEscribir)
{
			//El parametro i representa el numeroDeBloque Actual
			int ultimoBloque =dictionary_size(diccionarioDeBloques);
			char * key = string_itoa(ultimoBloque);
			int numeroPuerto;
			long numeroIP;

			t_dictionary* dicDeCopias;
			dicDeCopias = dictionary_create();

			numeroIP = ntohl((*nodoDicc).datosNodo.connectionAddr.sin_addr.s_addr);
			numeroPuerto =ntohs((*nodoDicc).datosNodo.connectionAddr.sin_port);


			agregaCopia(dicDeCopias, "1", numeroIP, numeroPuerto, bloqueAEscribir);
			dictionary_put(diccionarioDeBloques, key, dicDeCopias);


		}

void agregaCopia23ConResto(t_estado_nodo* nodoDicc,int i,int bloqueAEscribir)
{
		//Para la 2/3 Copia
		t_dictionary * dicDeCopias=malloc(sizeof(t_dictionary));
		int numeroPuerto;
		long numeroIP;

		char * nroCopia= string_itoa(i);
		int nBloque = dictionary_size(diccionarioDeBloques);
		nBloque = nBloque - 1;

		char* key = string_itoa(nBloque);
		dicDeCopias = dictionary_get(diccionarioDeBloques,key);

		numeroIP = ntohl((*nodoDicc).datosNodo.connectionAddr.sin_addr.s_addr);
		numeroPuerto =ntohs((*nodoDicc).datosNodo.connectionAddr.sin_port);

		agregaCopia(dicDeCopias, nroCopia, numeroIP, numeroPuerto, bloqueAEscribir);


}
