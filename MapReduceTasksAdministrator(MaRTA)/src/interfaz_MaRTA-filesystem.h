/*
 * interfaz_MaRTA-filesystem.h
 *
 *  Created on: 9/6/2015
 *      Author: utnso
 */

#ifndef MAPREDUCETASKSADMINISTRATOR_MARTA__INTERFAZ_MARTA_FILESYSTEM_H_
#define MAPREDUCETASKSADMINISTRATOR_MARTA__INTERFAZ_MARTA_FILESYSTEM_H_


#include <netinet/in.h>
#include <arpa/inet.h>
#include "interfaz_MaRTA-Job.h"

typedef enum {
	Conectado = 0,
	Desconectado = 1
}t_estado;
typedef struct {
	    struct in_addr ip;
	    int puerto;
		t_estado estado; // procesado,nuevo o caido
	    int cant_rutinas_mapper;
	    int cant_rutinas_reduce;
		t_list* tareas_en_ejecucion; // lista de char* que puede tener: "mapping","reduce" o "inactivo"
		} t_nodo;






//dado un nodo, aumenta en uno su cantidad de tareas mapping
void aumentarRutinasMapping(t_nodo nodo);

//dado un nodo, disminuye en uno su cantidad de tareas mapping
void disminuirRutinasMapping(t_nodo nodo);

//dado un nodo, aumenta en uno su cantidad de tareas reduce
void aumentarRutinasReduce(t_nodo nodo);

//dado un nodo, disminuye en uno su cantidad de tareas reduce
void disminuirRutinasReduce(t_nodo nodo);


bool nodoTieneMismaIpYPuerto (t_nodo* nodo, in_addr_t ip, int puerto);

// muestra por pantalla ip y estado de un nodo
void imprimirIpYEstado(t_nodo* nodo);

// muestra por pantalla una tarea
void imprimirTareaEnEjecucion(char* tarea);

// dado un nodo imprime su ip y las tareas q esta ejecutando
void imprimirTareasEnEjecucion(t_nodo* nodo);


//dada una lista de nodos, una ip y un puerto: retorna el elemento de la lista correspondiente o NULL si no esta
t_nodo* buscarNodo(t_list* lista,struct in_addr ip, int puerto);
//dado una lista de nodo y un nodo, lo inserta siempre y cuando no haya otro con la misma ip y puerto
void* buscarNumero(t_list* lista,int nro);
void insertarNodoSolicitadoONumeroDeBloque(struct in_addr ip,int puerto,int numeroBloque, t_list* bloques);


int ocupacionDelNodo(t_nodo* nodo);
void buscarNodoMenosOcupado(t_list* listaNodos,t_nodo* elMasDesocupado);
void eliminarTarea(t_list* tareas,char*tarea);

void imprimirEstado(t_nodo* nodo);


#endif /* MAPREDUCETASKSADMINISTRATOR_MARTA__INTERFAZ_MARTA_FILESYSTEM_H_ */
