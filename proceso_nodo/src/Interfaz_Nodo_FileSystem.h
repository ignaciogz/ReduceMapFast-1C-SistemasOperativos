#ifndef INTERFAZ_NODO_FILESYSTEM_H_
#define INTERFAZ_NODO_FILESYSTEM_H_

/* Se define la constante tamaño_bloque que representa la cantidad de bytes equivalentes a 20MB*/
#define tamanio_bloque 20971520

/*Se define la estructura que representa a un bloque de datos con su numero y sus 20MB de datos*/
typedef struct {
	int numero_bloque;
	char* datos;
} t_bloque_datos;


/*Funciones de interfaz entre un nodo con el file system y otros nodos*/


/*getBloque: dado un bloque de datos pasado por referencia, que solo tiene el numero de bloque indicado, y en los datos tiene basura, se le copian los datos del espacio de datos*/
void getBloque(t_bloque_datos* bloqueDeseado, char* espacio_datos);


/*setBloque: dado un bloque de datos, lo guarda en el espacio de datos del nodo*/
void setBloque(t_bloque_datos bloqueACopiar, char* espacio_datos) ;


/*getFileContent: dado un nombre de un archivo del espacio tempora, obtiene el contenido de dicho archivo*/
char* getFileContent (char* nombre_Archivo, char* espacio_temporal);



#endif /* INTERFAZ_NODO-FILESYSTEM_H_ */
