#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>

#include <commons/string.h>
#include <commons/collections/dictionary.h>
#define SERVIDOR_DE_MONGO "mongodb://localhost:27017/"
#include "headers/mongodb.h"
#include <netinet/in.h>
#include <arpa/inet.h>

#include "headers/fsecundarias_mongodb.h"
#include "../headers/socketsFs.h"

/**********************************************************
FORMATEAR LA DB                                 TESTEADO =)
****************************/
void formatearDB (void)
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    bson_error_t * error = NULL;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");
        mongoc_collection_drop (coleccionDeDatosDB, error);

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();
}


/**********************************************************
INSERTAR ARCHIVO EN LA DB                       TESTEADO =)
****************************/
int insertarArchivo_DB(char * nombreDelArchivo, long tamanioDelArchivo, int dirPadreDelArchivo, t_dictionary * diccionarioDeBloques)
//Devuelve 1 si se inserto correctamente, caso contrario un 0 si el archivo ya esta en la DB
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    bson_error_t error;
    bson_t *documento_X;
    int Estado = 1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo un documento para insertarlo en la DB
        documento_X = crearDocumentoParaLaDB(nombreDelArchivo, tamanioDelArchivo, dirPadreDelArchivo, diccionarioDeBloques);

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

        //Lo inserto en la DB
        if (count == 0){
            mongoc_collection_insert (coleccionDeDatosDB, MONGOC_INSERT_NONE, documento_X, NULL, &error);
        }else{
            Estado = 0;
        }

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();

    return Estado;
}





/**********************************************************
BUSCAR DB                                       TESTEADO =)
****************************/
int buscarArchivoParaImprimirlo_DB (char *nombreDelArchivo)
//Si existeDevuelve 1, en caso contrario un 0
{

    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_cursor_t *cursor;
    const bson_t *documento_X;
    bson_t *query;
    char *str;
    bson_error_t error;
    int existe = 0;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo la busqueda del documento que tenga la info del ARCHIVO SOLICITADO
        query = bson_new ();
        BSON_APPEND_UTF8 (query, "nombreDelArchivo", nombreDelArchivo);

        //GUARDO los resultados de la busqueda
        cursor = mongoc_collection_find (coleccionDeDatosDB, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

        //Se imprimen todos los json que coincidan
        while (mongoc_cursor_next (cursor, &documento_X)){
            str = bson_as_json (documento_X, NULL);
            printf ("%s\n", str);
            bson_free (str);
            existe = 1;
        };
        if(!existe){
            printf("El archivo NO existe");
        };

        if (mongoc_cursor_error (cursor, &error)) {
          printf ("Cursor Failure: %s\n", error.message);
        };

    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();
    return existe;
}





/**********************************************************
OBTENER BLOQUES, QUE CONFORMAN UN ARCHIVO       TESTEADO =)
****************************/
t_dictionary * getBloques_DB (char *nombreDelArchivo)
//Si el diccionario que devuelve esta VACIO, indica que el archivo NO ESTA en la DB
//OJO afuera limpiar y liberar el diccionario generado por esta funcion !!
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_cursor_t *cursor;
    const bson_t *documento_X;
    bson_t *query;

    int cantidadTotalDeBloques;
    int identificadorDelBloque = 0;
    int numeroDeCopia = 1;

    t_dictionary * dicDeBloques;
    t_dictionary * dicCopiasDelBloque;
    int numeroDeCopiasDelBloque = 0;

    char *iBloque; char *nCopia;


    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo la busqueda del documento que tenga la info del ARCHIVO SOLICITADO
        query = bson_new ();
        BSON_APPEND_UTF8 (query, "nombreDelArchivo", nombreDelArchivo);

        //GUARDO los resultados de la busqueda
        cursor = mongoc_collection_find (coleccionDeDatosDB, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

        //Obtengo el documento
        mongoc_cursor_next (cursor, &documento_X);
        dicDeBloques = dictionary_create();

        if (documento_X != NULL)
        {
            //**   ARMANDO diccionario de BLOQUES PARA RETORNALO
            cantidadTotalDeBloques = getCantidadTotalDeBloques_ArchivoDB(documento_X);

            while(identificadorDelBloque < cantidadTotalDeBloques)
            {

                dicCopiasDelBloque = dictionary_create();
                numeroDeCopiasDelBloque = getCantidadDeCopiasDe1Bloque_ArchivoDB(documento_X, identificadorDelBloque);

                while(numeroDeCopia <= numeroDeCopiasDelBloque)
                {
                    t_copiaX* copiaX = malloc(sizeof(int)*2 + sizeof(struct in_addr));

                       long int valorDeIP = getValorDeUnaCopia_ArchivoDB(documento_X, identificadorDelBloque, numeroDeCopia, "IP");
                       int valorDePuerto = getValorDeUnaCopia_ArchivoDB(documento_X, identificadorDelBloque, numeroDeCopia, "puerto");
                       int valorDeNumBloque = getValorDeUnaCopia_ArchivoDB(documento_X, identificadorDelBloque, numeroDeCopia, "numBloque");

                    (*copiaX).IP = htonl(valorDeIP);
                    (*copiaX).puerto = htons(valorDePuerto);
                    (*copiaX).numBloque = valorDeNumBloque;

                    nCopia = string_itoa(numeroDeCopia);
                    dictionary_put(dicCopiasDelBloque, nCopia, copiaX);

                    free(nCopia);
                    numeroDeCopia++;
                }

                numeroDeCopia = 1;

                iBloque = string_itoa(identificadorDelBloque);
                dictionary_put(dicDeBloques, iBloque, dicCopiasDelBloque);

                free(iBloque);
                identificadorDelBloque++;
            }
        }

    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();

    return dicDeBloques;
}





/**********************************************************
OBTENER ESTADO DE UN ARCHIVO                    TESTEADO =)
****************************/
int getEstado_ArchivoDB (char *nombreDelArchivo)
//Devuelve 1 o 0 (Dependiendo del estado) o -1 si el archivo NO esta en la DB
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_cursor_t *cursor;
    const bson_t *documento_X = NULL;
    bson_t *query;
    bson_iter_t punteroDeBusqueda;
    bson_iter_t valorEncontrado;
    int Estado = -1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo la busqueda del documento que tenga la info del ARCHIVO SOLICITADO
        query = bson_new ();
        BSON_APPEND_UTF8 (query, "nombreDelArchivo", nombreDelArchivo);

        //GUARDO los resultados de la busqueda
        cursor = mongoc_collection_find (coleccionDeDatosDB, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

        //Obtengo el documento
        mongoc_cursor_next (cursor, &documento_X);


            if (documento_X != NULL &&
                bson_iter_init (&punteroDeBusqueda, documento_X) &&
                bson_iter_find_descendant (&punteroDeBusqueda, "estado", &valorEncontrado) &&
                BSON_ITER_HOLDS_INT32 (&valorEncontrado))
            {
                Estado = bson_iter_int32 (&valorEncontrado);
            }


    bson_destroy (query);
    mongoc_cursor_destroy (cursor);

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();
    return Estado;
}





/**********************************************************
ELIMINAR ARCHIVO DE LA DB                       TESTEADO =)
****************************/
int borrarArchivo_DB (char * unNombreDeArhivo)
//Devuelve 1 si puedo eliminar el archivo de la DB o 0 si NO existe el archivo
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    bson_error_t error;
    bson_t *documento_X;
    int Estado = 0;

        mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Busco el documento
        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", unNombreDeArhivo);

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);
        //ELIMINO EL DOCUMENTO DE LA DB
        if (count > 0){
            mongoc_collection_remove (coleccionDeDatosDB, MONGOC_DELETE_SINGLE_REMOVE, documento_X, NULL, &error);
            Estado = 1;
        }

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();

    return Estado;
}





/**********************************************************
ACTUALIZAR ESTADO DE UN ARCHIVO                 TESTEADO =)
****************************/
int actualizarEstado_ArchivoDB (char * nombreDelArchivo, int nuevoValorDeEstado)
//Devuelve 1 si pudo actualizar el estado, o un 0 si el archivo no existe
{
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_client_t *nuevoClienteDB;
    bson_error_t error;
    bson_t *documento_X;
    bson_t *elemento_actualizado;
    int Estado = 1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Busco el documento
        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", nombreDelArchivo);
        //Preparo los datos de actualizacion
        elemento_actualizado = BCON_NEW ("$set", "{",
                                   "estado", BCON_INT32 (nuevoValorDeEstado),
                               "}");

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

        if (count > 0){
            // Busqueda y modificacion del archivo.
            mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_actualizado, NULL, false, false, false, NULL, &error);
        }else{
            Estado = 0;
        }

    bson_destroy (elemento_actualizado);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup ();

    return Estado;
}





/**********************************************************
VERIFICANDO SI EL ARCHIVO ESTA EL DB            TESTEADO =)
****************************/
int archivoEstaEnLaDB (char * nombreDelArchivo)
//Retorna un numero positivo si esta en la DB o un 0 si NO esta el archivo
{
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_client_t *nuevoClienteDB;
    bson_error_t error;
    int count = 0;
    bson_t* documento_X;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", nombreDelArchivo);
        count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);
    mongoc_cleanup ();

    return count;
}





/**********************************************************
OBTENER TAMANIO DE UN ARCHIVO                   TESTEADO =)
****************************/
long getTamanio_ArchivoDB (char *nombreDelArchivo)
//Devuelve un numero positivo, que indica el tamanio o 0 si el archivo NO esta en la DB
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_cursor_t *cursor;
    const bson_t *documento_X = NULL;
    bson_t *query;
    bson_iter_t punteroDeBusqueda;
    bson_iter_t valorEncontrado;
    long Tamanio = 0;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo la busqueda del documento que tenga la info del ARCHIVO SOLICITADO
        query = bson_new ();
        BSON_APPEND_UTF8 (query, "nombreDelArchivo", nombreDelArchivo);

        //GUARDO los resultados de la busqueda
        cursor = mongoc_collection_find (coleccionDeDatosDB, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

        //Obtengo el documento
        mongoc_cursor_next (cursor, &documento_X);


            if (documento_X != NULL &&
                bson_iter_init (&punteroDeBusqueda, documento_X) &&
                bson_iter_find_descendant (&punteroDeBusqueda, "tamanio", &valorEncontrado) &&
                BSON_ITER_HOLDS_INT32 (&valorEncontrado))
            {
                Tamanio = bson_iter_int32 (&valorEncontrado);
            }


    bson_destroy (query);
    mongoc_cursor_destroy (cursor);

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();
    return Tamanio;
}





/**********************************************************
OBTENER EL DIRECTORIO PADRE DE UN ARCHIVO       TESTEADO =)
****************************/
int getDirPadre_ArchivoDB (char *nombreDelArchivo)
//Devuelve el numero de Directorio Padre, o -1 si el archivo NO esta en la DB
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_cursor_t *cursor;
    const bson_t *documento_X = NULL;
    bson_t *query;
    bson_iter_t punteroDeBusqueda;
    bson_iter_t valorEncontrado;
    int dirPadre = -1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo la busqueda del documento que tenga la info del ARCHIVO SOLICITADO
        query = bson_new ();
        BSON_APPEND_UTF8 (query, "nombreDelArchivo", nombreDelArchivo);

        //GUARDO los resultados de la busqueda
        cursor = mongoc_collection_find (coleccionDeDatosDB, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

        //Obtengo el documento
        mongoc_cursor_next (cursor, &documento_X);


            if (documento_X != NULL &&
                bson_iter_init (&punteroDeBusqueda, documento_X) &&
                bson_iter_find_descendant (&punteroDeBusqueda, "dirPadre", &valorEncontrado) &&
                BSON_ITER_HOLDS_INT32 (&valorEncontrado))
            {
                dirPadre = bson_iter_int32 (&valorEncontrado);
            }


    bson_destroy (query);
    mongoc_cursor_destroy (cursor);

    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();
    return dirPadre;
}





/**********************************************************
ACTUALIZAR NOMBRE DE UN ARCHIVO                 TESTEADO =)
****************************/
int actualizarNombre_ArchivoDB (char * nombreDelArchivo, char * nuevoNombre)
//Devuelve 1 si pudo actualizar el nombre, o un 0 si el archivo no existe
{
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_client_t *nuevoClienteDB;
    bson_error_t error;
    bson_t *documento_X;
    bson_t *elemento_actualizado;
    int Estado = 1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Busco el documento
        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", nombreDelArchivo);
        //Preparo los datos de actualizacion
        elemento_actualizado = BCON_NEW ("$set", "{",
                                   "nombreDelArchivo", BCON_UTF8 (nuevoNombre),
                               "}");

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

        if (count > 0){
            // Busqueda y modificacion del archivo.
            mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_actualizado, NULL, false, false, false, NULL, &error);
        }else{
            Estado = 0;
        }

    bson_destroy (elemento_actualizado);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup ();

    return Estado;
}





/**********************************************************
ACTUALIZAR DIR PADRE DE UN ARCHIVO              TESTEADO =)
****************************/
int actualizarDirPadre_ArchivoDB (char * nombreDelArchivo, int nuevoValorDeDirPadre)
//Devuelve 1 si pudo actualizar el directorio padre, o un 0 si el archivo no existe
{
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_client_t *nuevoClienteDB;
    bson_error_t error;
    bson_t *documento_X;
    bson_t *elemento_actualizado;
    int Estado = 1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Busco el documento
        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", nombreDelArchivo);
        //Preparo los datos de actualizacion
        elemento_actualizado = BCON_NEW ("$set", "{",
                                   "estado", BCON_INT32 (nuevoValorDeDirPadre),
                               "}");

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

        if (count > 0){
            // Busqueda y modificacion del archivo.
            mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_actualizado, NULL, false, false, false, NULL, &error);
        }else{
            Estado = 0;
        }

    bson_destroy (elemento_actualizado);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup ();

    return Estado;
}





/**********************************************************
Agregar Una Copia Al BLOQUE X de un Archivo     TESTEADO =)
****************************/
int agregarUnaCopiaAlBloqueX(char * nombreDelArchivo, int numBloque, long int copia_IP, int copia_Puerto, int copia_numBloque)
//Devuelve 1 si pudo agregar la copia al Bloque X, o un 0 si el archivo solicitado no exite
{
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_client_t *nuevoClienteDB;
    bson_error_t error;
    bson_t *documento_X;
    bson_t *elemento_a_agregar;
    bson_t *elemento_a_actualizar;
    int Estado = 1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Busco el documento
        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", nombreDelArchivo);


        //Creando la copia que voy a agregar... (1)
        bson_t * copiaNueva = bson_new();
            // ::: Estructura BASICA de la copia nueva :::
                BSON_APPEND_INT32 (copiaNueva, "IP", copia_IP);
                BSON_APPEND_INT32 (copiaNueva, "puerto", copia_Puerto);
                BSON_APPEND_INT32 (copiaNueva, "numBloque", copia_numBloque);

        //Creando la clave de acceso al ARRAY del BloqueX... (2)
            char *claveGenerada = string_new();

            string_append(&claveGenerada, "Bloques");
                string_append(&claveGenerada, ".");
            string_append(&claveGenerada, string_itoa(numBloque));
                string_append(&claveGenerada, ".");
            string_append(&claveGenerada, "copias");


        //Preparo el elemento a agregar, usando (1) y (2)
            elemento_a_agregar = BCON_NEW ("$push",
                                            "{",
                                                claveGenerada, BCON_DOCUMENT(copiaNueva),
                                            "}"
                                        );


        //Creando la clave de acceso a la CANTIDAD DE COPIAS del BloqueX...
            char *claveGenerada2 = string_new();

            string_append(&claveGenerada2, "Bloques");
                string_append(&claveGenerada2, ".");
            string_append(&claveGenerada2, string_itoa(numBloque));
                string_append(&claveGenerada2, ".");
            string_append(&claveGenerada2, "cantidadDeCopias");

        //Aumentando en 1 la CANTIDAD DE COPIAS del BloqueX...
            elemento_a_actualizar = BCON_NEW ("$inc", "{",
                                                   claveGenerada2, BCON_INT32(1),
                                            "}"
                                   );

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

        if (count > 0){
            // Busqueda y modificacion del archivo, agrego la copia y actualizo la cantida de copias.
            mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_a_agregar, NULL, false, false, false, NULL, &error);
            mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_a_actualizar, NULL, false, false, false, NULL, &error);
        }else{
            Estado = 0;
        }

    bson_destroy(copiaNueva);
    bson_destroy (elemento_a_agregar);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    free(claveGenerada);
    free(claveGenerada2);
    mongoc_cleanup ();

    return Estado;
}





/**********************************************************
Eliminar Una Copia Al BLOQUE X de un Archivo    TESTEADO =)
****************************/
int eliminarUnaCopiaAlBloqueX(char * nombreDelArchivo, int numBloque, long int ip_nodo, int puerto_nodo)
//Devuelve 1 si pudo eliminar la copia del Bloque X.
//Un 0 si el archivo solicitado no exite.
//Un -1 si la IP / Puerto / numBloque son incorrectos.
{
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_client_t *nuevoClienteDB;
    bson_error_t error;
    bson_t *documento_X;
    bson_t *elemento_a_actualizar;
    bson_t *elemento_a_eliminar;
    int Estado = 1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Busco el documento
        documento_X = bson_new ();
        BSON_APPEND_UTF8 (documento_X, "nombreDelArchivo", nombreDelArchivo);

        //Creando la clave de acceso al ARRAY del BloqueX... (2)
            char *claveGenerada = string_new();

            string_append(&claveGenerada, "Bloques");
                string_append(&claveGenerada, ".");
            string_append(&claveGenerada, string_itoa(numBloque));
                string_append(&claveGenerada, ".");
            string_append(&claveGenerada, "copias");

        //Preparo el elemento a eliminar
            elemento_a_eliminar = BCON_NEW ("$pull",
                                            "{",
                                                claveGenerada,
                                                "{",
                                                    "IP",  BCON_INT32(ip_nodo),
                                                    "puerto", BCON_INT32(puerto_nodo),
                                                "}",
                                            "}"
                                        );

        //Creando la clave de acceso a la CANTIDAD DE COPIAS del BloqueX...
            char *claveGenerada2 = string_new();

            string_append(&claveGenerada2, "Bloques");
                string_append(&claveGenerada2, ".");
            string_append(&claveGenerada2, string_itoa(numBloque));
                string_append(&claveGenerada2, ".");
            string_append(&claveGenerada2, "cantidadDeCopias");

        //Disminuyendo en 1 la CANTIDAD DE COPIAS del BloqueX...
            elemento_a_actualizar = BCON_NEW ("$inc", "{",
                                                   claveGenerada2, BCON_INT32(-1),
                                            "}"
                                   );

        int count = mongoc_collection_count (coleccionDeDatosDB, MONGOC_QUERY_NONE, documento_X, 0, 0, NULL, &error);

        if (count > 0){
            // Busqueda y modificacion del archivo, elimino la copia solicitada
            if( !mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_a_eliminar, NULL, false, false, false, NULL, &error) )
            {
                Estado = -1;
            }else{
                mongoc_collection_find_and_modify (coleccionDeDatosDB, documento_X, NULL, elemento_a_actualizar, NULL, false, false, false, NULL, &error);
            }
        }else{
            Estado = 0;
        }

    bson_destroy (elemento_a_eliminar);
    bson_destroy (elemento_a_actualizar);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    free(claveGenerada);
    free(claveGenerada2);
    mongoc_cleanup ();

    return Estado;
}

/******************************************************************************************
Obtiene el numeroDeBloque donde esta la info, de un bloque+nodo especifico      TESTEADO =)
****************************/
int getNumBloqueDeUnNodoEspecifico_ArchivoDB(char* nombreDelArchivo, int numBloque, long ip_nodo)
{
    mongoc_client_t *nuevoClienteDB;
    mongoc_collection_t *coleccionDeDatosDB;
    mongoc_cursor_t *cursor;
    const bson_t *documento_X;
    bson_t *query;

    int numeroDeCopia = 1;

    int numeroDeCopiasDelBloque = 0;
    int numeroDeBloqueBuscado = -1;

    mongoc_init ();

        nuevoClienteDB = mongoc_client_new (SERVIDOR_DE_MONGO);
        coleccionDeDatosDB = mongoc_client_get_collection (nuevoClienteDB, "DBFS", "MDFS");

        //Preparo la busqueda del documento que tenga la info del ARCHIVO SOLICITADO
        query = bson_new ();
        BSON_APPEND_UTF8 (query, "nombreDelArchivo", nombreDelArchivo);

        //GUARDO los resultados de la busqueda
        cursor = mongoc_collection_find (coleccionDeDatosDB, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

        //Obtengo el documento
        mongoc_cursor_next (cursor, &documento_X);

        if (documento_X != NULL)
        {
                numeroDeCopiasDelBloque = getCantidadDeCopiasDe1Bloque_ArchivoDB(documento_X, numBloque);

                while(numeroDeCopia <= numeroDeCopiasDelBloque)
                {

                       long int valorDeIP = getValorDeUnaCopia_ArchivoDB(documento_X, numBloque, numeroDeCopia, "IP");
                       int valorDeNumBloque = getValorDeUnaCopia_ArchivoDB(documento_X, numBloque, numeroDeCopia, "numBloque");

                    if(ip_nodo == valorDeIP)
                    {
                        numeroDeBloqueBuscado = valorDeNumBloque;
                        break;
                    }

                    numeroDeCopia++;
                }
        }

    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (coleccionDeDatosDB);
    mongoc_client_destroy (nuevoClienteDB);

    mongoc_cleanup();

    return numeroDeBloqueBuscado;
}
