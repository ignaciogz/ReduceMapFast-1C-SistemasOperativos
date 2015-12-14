
#ifndef MONGODB_H_
#define MONGODB_H_

	#include <netinet/in.h>
	#include <arpa/inet.h>

	/**********************************************************
	FORMATEAR LA DB                                 TESTEADO =)
	****************************/
	void formatearDB (void);


	/**********************************************************
	INSERTAR ARCHIVO EN LA DB                       TESTEADO =)
	****************************/
	int insertarArchivo_DB(char * nombreDelArchivo, long tamanioDelArchivo, int dirPadreDelArchivo, t_dictionary * diccionarioDeBloques);
	//Devuelve 1 si se inserto correctamente, caso contrario un 0 si el archivo ya esta en la DB


	/**********************************************************
	BUSCAR DB                                       TESTEADO =)
	****************************/
	int buscarArchivoParaImprimirlo_DB (char *nombreDelArchivo);
	//Si existeDevuelve 1, en caso contrario un 0


	/**********************************************************
	OBTENER BLOQUES, QUE CONFORMAN UN ARCHIVO       TESTEADO =)
	****************************/
	t_dictionary * getBloques_DB (char *nombreDelArchivo);
	//Si el diccionario que devuelve esta VACIO, indica que el archivo NO ESTA en la DB
	//OJO afuera limpiar y liberar el diccionario generado por esta funcion !!


	/**********************************************************
	OBTENER ESTADO DE UN ARCHIVO 					TESTEADO =)
	****************************/
	int getEstado_ArchivoDB (char *nombreDelArchivo);
	//Devuelve 1 o 0 (Dependiendo del estado) o -1 si el archivo NO esta en la DB


	/**********************************************************
	ELIMINAR ARCHIVO DE LA DB 						TESTEADO =)
	****************************/
	int borrarArchivo_DB (char * unNombreDeArhivo);
	//Devuelve 1 si puedo eliminar el archivo de la DB o 0 si NO existe el archivo


	/**********************************************************
	ACTUALIZAR ESTADO DE UN ARCHIVO 				TESTEADO =)
	****************************/
	int actualizarEstado_ArchivoDB (char * nombreDelArchivo, int nuevoValorDeEstado);
	//Devuelve 1 si pudo actualizar el estado, o un 0 si el archivo no existe


	/**********************************************************
	VERIFICANDO SI EL ARCHIVO ESTA EL DB 			TESTEADO =)
	****************************/
	int archivoEstaEnLaDB (char * nombreDelArchivo);
	//Retorna un numero positivo si esta en la DB o un 0 si NO esta el archivo


	/**********************************************************
	OBTENER TAMANIO DE UN ARCHIVO 					TESTEADO =)
	****************************/
	long getTamanio_ArchivoDB (char *nombreDelArchivo);
	//Devuelve un numero positivo, que indica el tamanio o 0 si el archivo NO esta en la DB


	/**********************************************************
	OBTENER EL DIRECTORIO PADRE DE UN ARCHIVO       TESTEADO =)
	****************************/
	int getDirPadre_ArchivoDB (char *nombreDelArchivo);
	//Devuelve el numero de Directorio Padre, o -1 si el archivo NO esta en la DB


	/**********************************************************
	ACTUALIZAR NOMBRE DE UN ARCHIVO                 TESTEADO =)
	****************************/
	int actualizarNombre_ArchivoDB (char * nombreDelArchivo, char * nuevoNombre);
	//Devuelve 1 si pudo actualizar el nombre, o un 0 si el archivo no existe


	/**********************************************************
	ACTUALIZAR DIR PADRE DE UN ARCHIVO 				TESTEADO =)
	****************************/
	int actualizarDirPadre_ArchivoDB (char * nombreDelArchivo, int nuevoValorDeDirPadre);
	//Devuelve 1 si pudo actualizar el directorio padre, o un 0 si el archivo no existe


	/**********************************************************
	Agregar Una Copia Al BLOQUE X de un Archivo     TESTEADO =)
	****************************/
	int agregarUnaCopiaAlBloqueX(char * nombreDelArchivo, int numBloque, long copia_IP, int copia_Puerto, int copia_numBloque);
	//Devuelve 1 si pudo agregar la copia al Bloque X, o un 0 si el archivo solicitado no exite


	/**********************************************************
	Eliminar Una Copia Al BLOQUE X de un Archivo    TESTEADO =)
	****************************/
	int eliminarUnaCopiaAlBloqueX(char * nombreDelArchivo, int numBloque, long ip_nodo, int puerto_nodo);
	//Devuelve 1 si pudo eliminar la copia del Bloque X.
	//Un 0 si el archivo solicitado no exite.
	//Un -1 si la IP / Puerto / numBloque son incorrectos.

	/******************************************************************************************
	Obtiene el numeroDeBloque donde esta la info, de un bloque+nodo especifico      TESTEADO =)
	****************************/
	int getNumBloqueDeUnNodoEspecifico_ArchivoDB(char* nombreDelArchivo, int numBloque, long ip_nodo);
#endif
