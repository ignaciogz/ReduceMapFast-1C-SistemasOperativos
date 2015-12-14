/*
 * manejoDeDatos.h
 *
 *  Created on: 4/7/2015
 *      Author: utnso
 */
#ifndef SRC_HEADERS_MANEJODEDATOS_H_
#define SRC_HEADERS_MANEJODEDATOS_H_

#include <commons/bitarray.h>
#include <stdlib.h>
#include "socketsFs.h"

typedef struct{
	char* Ip1;
	char* Ip2;
	int historial;
}t_IpViejas;

//Dado un nombreArchivo, divide el archivo en 20mb y lo manda al nodo con mayor prioridad
int cargarBloquesNodo(char* nombreDeUnArchivo);
//Retorna 1 si hay mucho espacio y 0 en su defecto
int muchoEspacio(ldiv_t bloquesArchivo,int cantBloquesLibres);
//Retorna 1 si hay muchos nodos y 0 en su defecto
int muchosNodos (int cantNodosActivos);
//Retorna longitud archivo
long getFileLength (char* fileName);
//Devuelve el nodo con mas bloques libres
t_nodo nodoConPrioridad(t_IpViejas IpRepe);
//devuelve la cant  de bloquess totales de todos los nodos conectados
int cantBloquesTotal();
//Dado un bitarray devuelve la suma de los 0
int cantBloquesLibres(t_bitarray* estadBloques);
//Devuelve el primer bit q sea 0
int obtenerBloqueLibre(t_bitarray* estadBloques);
//Devuelve la cantidad de veces q hay q dividir el Archivo
int cantDividirArchivo(int bloquesArch);
//Esta disctribucion copia el arch entero a un nodo
int distribucionGrande(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, int fd,t_IpViejas IpRepe,int repeticiones);
//Esta distribucion copia de a un bloque y lo manda
int distribucionNormal(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, int fd,t_IpViejas IpRepe, int origen,int repeticiones);
//Esta dist. divide el archivo de acuerdo a un valor y lo manda y retorna el nodo el cual uso
t_nodo distribucionMediana(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, int fd,t_IpViejas IpRepe,int repeticiones);
//Esta dist. es casi similar a la distrGrande pero trabaja con resto de dividir el arch con 20mb
int distribucionGrandeConResto(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, long tamanioArch, int fd,t_IpViejas IpRepe, int repeticiones);
//manda mensaje al nodo agregando '|0'
int mandarMensajeConResto(t_nodo nodo, int fd, long tamanioArch, long resto, int origen, int repeticiones);
//manda mensaje al nodo
int mandarMensaje(t_nodo nodo, int fd, int i, int origen, int repeticiones);
//ACtualiza el registro con la nueva IP usada por nodoConPrioridad
void actualizarIpVieja(const struct in_addr IP, t_IpViejas IpRepe);
//Comprueba si el IP esta en el registro
bool noSeRepite(char* Ip_Lista, t_IpViejas IpRepe);
#endif /* SRC_HEADERS_MANEJODEDATOS_H_ */
