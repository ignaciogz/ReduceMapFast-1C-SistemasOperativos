#include <bson.h>
#include <mongoc.h>
#include <stdio.h>

#include <commons/string.h>
#include <commons/collections/dictionary.h>

#include "headers/fsecundarias_mongodb.h"
#include "../headers/socketsFs.h"

/*********************************************************************************************
F_SECUNDARIA: Creando un BSON del archivo, para insertalo en la DB                 TESTEADO =)
****************************/
bson_t * crearDocumentoParaLaDB(char * nombreDelArchivo, long tamanioDelArchivo, int dirPadreDelArchivo, t_dictionary * diccionarioDeBloques)
//Va eliminando el diccionario que ingresa por parametro :D
{
	bson_t child; bson_t child2; bson_t child3; bson_t child4;
	bson_oid_t oid;

	int cantidadTotalDeBloques;
	int identificadorDelBloque = 0;

	t_dictionary* copiasDelBloqueX;
	t_copiaX* copiaX;

	char * iDelBloque;
	//FIN DECLARACION DE VARIABLES


	bson_t * documento = bson_new();
	// ::: Estructura BASICA del documento :::
		bson_oid_init (&oid, NULL);
        BSON_APPEND_OID (documento, "_id", &oid);
		BSON_APPEND_UTF8 (documento, "nombreDelArchivo", nombreDelArchivo);
	 	BSON_APPEND_INT32 (documento, "tamanio", tamanioDelArchivo);
	 	BSON_APPEND_INT32 (documento, "dirPadre", dirPadreDelArchivo);
		BSON_APPEND_INT32 (documento, "estado", 1);

	cantidadTotalDeBloques = dictionary_size(diccionarioDeBloques);

		BSON_APPEND_INT32 (documento, "cantidadDeBloques", cantidadTotalDeBloques);
	// ::: Estructura de BLOQUES del documento :::
	//Guardando los N bloques con copias 3
	bson_append_document_begin (documento, "Bloques", 7, &child);

		while(identificadorDelBloque < cantidadTotalDeBloques)
		{
			iDelBloque = string_itoa(identificadorDelBloque);

			bson_append_document_begin (&child, iDelBloque, 1, &child2);

				copiasDelBloqueX = (t_dictionary*) dictionary_get( diccionarioDeBloques, iDelBloque );

				//Guardando la cantidad de copias del bloque X
					BSON_APPEND_INT32 (&child2, "cantidadDeCopias", 3);

				//Guardando las 3 copias para el bloque X
				bson_append_array_begin (&child2, "copias", 6, &child4);

					copiaX = (t_copiaX*) dictionary_get( copiasDelBloqueX, "1" );

					int Guachin= dictionary_size(copiasDelBloqueX);
					bson_append_document_begin (&child4, "0", 1, &child3);
						bson_append_int32(&child3,"IP", 2, (*copiaX).IP);
						bson_append_int32(&child3, "puerto", 6, (*copiaX).puerto);
						bson_append_int32(&child3, "numBloque", 9, (*copiaX).numBloque);
					bson_append_document_end (&child4, &child3);

					copiaX = (t_copiaX*) dictionary_get( copiasDelBloqueX, "2" );

					bson_append_document_begin (&child4, "1", 1, &child3);
						bson_append_int32(&child3,"IP", 2, (*copiaX).IP);
						bson_append_int32(&child3, "puerto", 6, (*copiaX).puerto);
						bson_append_int32(&child3, "numBloque", 9, (*copiaX).numBloque);
					bson_append_document_end (&child4, &child3);

					copiaX = (t_copiaX*) dictionary_get( copiasDelBloqueX, "3" );

					bson_append_document_begin (&child4, "2", 1, &child3);
						bson_append_int32(&child3,"IP", 2, (*copiaX).IP);
						bson_append_int32(&child3, "puerto", 6, (*copiaX).puerto);
						bson_append_int32(&child3, "numBloque", 9, (*copiaX).numBloque);
					bson_append_document_end (&child4, &child3);

				bson_append_array_end (&child2, &child4);

				dictionary_clean(copiasDelBloqueX);
				dictionary_destroy(copiasDelBloqueX);

			//Anido las 3 copias al bloque X
			bson_append_document_end (&child, &child2);

			free(iDelBloque);
			identificadorDelBloque++;
		};
	//::Estructura BASICA + estructura BLOQUES::
	bson_append_document_end (documento, &child);

	//Elimino el diccionarioDeBloques
	dictionary_clean(diccionarioDeBloques);
	dictionary_destroy(diccionarioDeBloques);

	return documento;
}





/*************************************************************************************************************************
F_SECUNDARIA: Obteniendo CANTIDAD TOTAL DE BLOQUES que conforman un Archivo 								   TESTEADO =)
****************************/
int getCantidadTotalDeBloques_ArchivoDB(const bson_t * documento_X)
//Devuelve un num positivo o Devuelve 0 si hay error
{
    bson_iter_t punteroDeBusqueda;
    bson_iter_t valorEncontrado;
    int CantidadBloques = 0;

    if (documento_X != NULL &&
        bson_iter_init (&punteroDeBusqueda, documento_X) &&
        bson_iter_find_descendant (&punteroDeBusqueda, "cantidadDeBloques", &valorEncontrado) &&
        BSON_ITER_HOLDS_INT32 (&valorEncontrado))
    {
        CantidadBloques = bson_iter_int32 (&valorEncontrado);
    }

    return CantidadBloques;
}





/*************************************************************************************************************************
F_SECUNDARIA: Obteniendo CANTIDAD TOTAL DE COPIAS de un BLOQUE  											   TESTEADO =)
****************************/
//Devuelve un num positivo o Devuelve 0 si hay error
int getCantidadDeCopiasDe1Bloque_ArchivoDB(const bson_t * documento_X, int numBloque)
{
    bson_iter_t punteroDeBusqueda;
    bson_iter_t valorEncontrado;
    int CantidadDeCopias = 0;

    char *claveGenerada = string_new();

		string_append(&claveGenerada, "Bloques");
    		string_append(&claveGenerada, ".");
		string_append(&claveGenerada, string_itoa(numBloque));
			string_append(&claveGenerada, ".");
		string_append(&claveGenerada, "cantidadDeCopias");

    if (documento_X != NULL &&
        bson_iter_init (&punteroDeBusqueda, documento_X) &&
        bson_iter_find_descendant (&punteroDeBusqueda, claveGenerada, &valorEncontrado) &&
        BSON_ITER_HOLDS_INT32 (&valorEncontrado))
    {
        CantidadDeCopias = bson_iter_int32 (&valorEncontrado);
    }

    free(claveGenerada);

    return CantidadDeCopias;
}





/*************************************************************************************************************************
F_SECUNDARIA: Obteniendo el VALOR dlong int IP, puerto o numBloque para una copiaX de un bloque X        TESTEADO =)
****************************/
long int getValorDeUnaCopia_ArchivoDB (const bson_t * documento_X, int numBloque, int numDeCopia, char * claveRequerida)
{
    bson_iter_t punteroDeBusqueda;
    bson_iter_t valorEncontrado;
    long int valorRequerido;


	// GENERO LA CLAVE DE BUSQUEDA
	//Ejemplo "Bloqlong int.IP" 	Bloques.numBloque.ArrayDeCopias.numDeCopia.claveRequerida (clavlong int IP o puerto o numDeBloque)

    char *claveGenerada = string_new();

    	string_append(&claveGenerada, "Bloques");
    		string_append(&claveGenerada, ".");
		string_append(&claveGenerada, string_itoa(numBloque));
			string_append(&claveGenerada, ".copias.");
		string_append(&claveGenerada, string_itoa(numDeCopia-1));
			string_append(&claveGenerada, ".");
		string_append(&claveGenerada, claveRequerida);


		if (documento_X != NULL &&
		        bson_iter_init (&punteroDeBusqueda, documento_X) &&
		        bson_iter_find_descendant (&punteroDeBusqueda, claveGenerada, &valorEncontrado) &&
		        BSON_ITER_HOLDS_INT32 (&valorEncontrado))
		{
        valorRequerido = bson_iter_int32 (&valorEncontrado);
    }

    free(claveGenerada);
    return valorRequerido;
}
