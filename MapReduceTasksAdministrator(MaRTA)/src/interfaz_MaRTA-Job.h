/*
 * interfaz_MaRTA-Job.h
 *
 *  Created on: 2/6/2015
 *      Author: utnso
 */

#ifndef INTERFAZ_MARTA_JOB_H_
#define INTERFAZ_MARTA_JOB_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>


typedef enum {no, si } t_combiner;

typedef struct {
	struct sockaddr_in direccion;
	int sock;
	t_list* archivos; // lista de elementos de tipo t_archivo
    char * archivo_resultado;
	t_combiner soporte_combiner;
	int pend_reduce;
	int pend_mapping;
	} t_job;

typedef struct {
	char* archivo;
	t_list* nodos; // lista de las copias separadas por nodos para el job sin combiner (elementos de tipo t_nodosDelArchivo)
	t_list* bloques; // lista de los elementos de tipo t_bloque
	t_bitarray* bitarray; // agregar list_create
	t_list* nodosConCopiasAMapear; //lista que contiene las copias a ser mapeadas (elementos de tipo t_nodosDelArchivo)
}t_archivo;

typedef struct{
	struct in_addr ipNodo;
	int puerto_nodo;
	int bloqueInterno;
}t_copia;

typedef struct{
	t_list* copias; //lista de elementos de tipo t_copia (agregar list_create en cargarBloque)
    t_copia copiaCandidata;
    char* archivoResultadoMap;
}t_bloque;

typedef struct{
	struct in_addr ipNodo;
	int puerto_nodo;
	t_list* copias; //lista de elementos de tipo t_copiasPorNodo;
}t_nodosDelArchivo;

typedef struct{
	int bloqueInterno;
	int bloqueDeArchivo;
	char* archivoResultadoMap;
}t_copiasPorNodo;


void imprimirIpPendientesDeMappingYPendientesDeReduce(t_job* job);

t_job* buscarJob(t_list* lista, struct in_addr ip , int puerto);

t_nodosDelArchivo* seleccionarNodoConMayorCantidadDeCopias(t_list* nodos);

void actualizarBitarray(t_nodosDelArchivo* nodo, t_bitarray* bitarray);

void actualizarNodos(t_nodosDelArchivo* ultimoNodoSeleccionado, t_list* nodos, t_bitarray* bitarray);

void distribuirTareasSinCombiner(t_archivo* archivo);

t_nodosDelArchivo* buscarNodoArchivo(t_list* lista, struct in_addr ip, int puerto);

void insertarNodoSolicitadoOCopia(struct in_addr ip,int puerto,t_copiasPorNodo* copiaNueva,t_list* nodos);

t_archivo* buscarArchivo(char* nombreArchivo,t_job* job);

t_copia* buscarCopia(t_bloque* bloque,struct in_addr ip, int puerto);

void elegirYGuardarCopiaCandidata(t_bloque* bloque,t_list*listaNodos);

void distribuirTareasConCombiner(t_archivo* archivo,t_list*listaNodos);

void disminuirPendReduce(t_job* job);

int longArchivosResultadoMapping(t_job * job,t_archivo* archivo);

void eliminarJob(t_list* listaJobs,struct in_addr ip, int puerto);

t_bloque* inicializarBloque(t_job* job,int nroBloque,t_archivo* archivo);

t_nodosDelArchivo* crearNodoDelArchivo(struct in_addr ip,int puerto);

t_copiasPorNodo* crearCopiaPorNodo(int bloqueDeArchivo,int bloqueInterno,char* archivo);

void asignarPendMapping(t_job* job, int cantidad);

void gestionarCombiner(t_combiner combiner,t_archivo* archivo,t_list*listaNodos);

int cantidadDeCopiasQueMapean(t_combiner combiner,t_archivo archivo);

t_copia seleccionarNodoReductor(t_job* job);
#endif /* INTERFAZ_MARTA_JOB_H_ */
