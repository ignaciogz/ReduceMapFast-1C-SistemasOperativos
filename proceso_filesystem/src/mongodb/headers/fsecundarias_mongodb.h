#ifndef FSECUNDARIAS_MONGODB_H_
#define FSECUNDARIAS_MONGODB_H_
	//Ingresa un DICCIONARIO DE BLOQUES del archivo Z
	//Donde la clave: "elNumeroDeBloque" y el valor es un: "t_dictionary copiasDelBloqueX"
	//Dentro de copias, la clave es: "numeroDeCopia" y el valor: "t_copiaX unaCopia" (que es un struct)
	//Dentro de unaCopia, hay IP, Puerto y NumBloque.

		/*EJEMPLO del diccionario::
			{
				"0" =
					{
						"1"=[IP Puerto numBloque]
						"2"=[IP Puerto numBloque]
						"3"=[IP Puerto numBloque]
					}
				"1" = t_dictionary * copias
					...
					...
				"N" = t_dictionary * copias
			}
		*/
	//Donde [ ] representa un struct y {} representa un Diccionario

//ASI SE VE DENTRO DE LA DB un Archivo con sus bloques
/*
*	CLAVE: Bloques
*
*	{
*		"0" = {
				"cantidadDeCopias" = 3,
*				"1" = ["IP" = 18765423564, "puerto" = 6578, "numBloque" = 14],
*				"2" = ["IP" = 24523612545, "puerto" = 620, "numBloque" = 9],
				"3" = ["IP" = 32432467079, "puerto" = 1054, "numBloque" = 13],
*			},
*		"1" = {una variacion del bloque "0" quizas con mas copias},
*		...
*		...
*		"N" = una variacion del bloque "0"
*	}
*/

// PROTOTIPOS:

	/*********************************************************************************************
	F_SECUNDARIA: Creando un BSON del archivo, para insertalo en la DB                 TESTEADO =)
	****************************/
	bson_t * crearDocumentoParaLaDB(char * nombreDelArchivo, long tamanioDelArchivo, int dirPadreDelArchivo, t_dictionary * diccionarioDeBloques);
	//Va eliminando el diccionario que ingresa por parametro :D


	/*********************************************************************************************
	F_SECUNDARIA: Obteniendo CANTIDAD TOTAL DE BLOQUES que conforman un Archivo 	   TESTEADO =)
	****************************/
	//Devuelve un num positivo o Devuelve 0 si hay error
	int getCantidadTotalDeBloques_ArchivoDB(const bson_t * documento_X);


	/*********************************************************************************************
	F_SECUNDARIA: Obteniendo CANTIDAD TOTAL DE COPIAS de un BLOQUE  				   TESTEADO =)
	****************************/
	//Devuelve un num positivo o Devuelve 0 si hay error
	int getCantidadDeCopiasDe1Bloque_ArchivoDB(const bson_t * documento_X, int numBloque);


	/*************************************************************************************************************************
	F_SECUNDARIA: Obteniendo el VALOR dada una clave: IP, puerto o numBloque para una copiaX de un bloque X        TESTEADO =)
	****************************/
	long getValorDeUnaCopia_ArchivoDB (const bson_t * documento_X, int numBloque, int numDeCopia, char * claveRequerida);

#endif
