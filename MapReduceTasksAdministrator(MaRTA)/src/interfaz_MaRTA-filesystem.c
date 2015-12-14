/*
 * interfaz_MaRTA-filesystem.c
 *
 *  Created on: 9/6/2015
 *      Author: utnso
 */

#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include "interfaz_MaRTA-filesystem.h"
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

extern pthread_mutex_t mutex;
void aumentarRutinasMapping(t_nodo nodo)
{
	nodo.cant_rutinas_mapper++;
};

void disminuirRutinasMapping(t_nodo nodo){
	nodo.cant_rutinas_mapper--;
};

void aumentarRutinasReduce(t_nodo nodo){
	nodo.cant_rutinas_reduce ++;
};

void disminuirRutinasReduce(t_nodo nodo){
	nodo.cant_rutinas_reduce--;
};


void imprimirIpYEstado(t_nodo* nodo)
{
    char* ip = inet_ntoa(nodo->ip);
	printf("\033[34mIp:\033[37m%s\n",ip);
	imprimirEstado(nodo);
	imprimirTareasEnEjecucion(nodo);
	printf("\n");
};



void imprimirTareaEnEjecucion(char* tarea){
	printf(" %s ", tarea);
};


 void imprimirTareasEnEjecucion(t_nodo* nodo)
 {
	printf("\033[34mtareas en ejecucion:\033[37m");
	list_iterate((nodo->tareas_en_ejecucion) ,(void*)imprimirTareaEnEjecucion);
	printf("\n");
}



void buscarNodoMenosOcupado(t_list* listaNodos,t_nodo* elMasDesocupado)
{
  pthread_mutex_lock( &mutex );
  int minimo;
  bool primerLectura = 1;
  void elMasLibreEs(t_nodo* nodo)
  {
      int gradoDeOcupacion = ocupacionDelNodo(nodo);
	  if(primerLectura !=0)
      {
	    elMasDesocupado= nodo;
	    minimo = ocupacionDelNodo(nodo);
	    primerLectura = 0;
      }
      else
      {
    	if(gradoDeOcupacion < minimo)
    	{
    		minimo = gradoDeOcupacion;
    		elMasDesocupado = nodo;
    	}
      }
  }
  list_iterate(listaNodos,(void*)elMasLibreEs);
  pthread_mutex_unlock( &mutex );
}

int ocupacionDelNodo(t_nodo* nodo)
{
	int ocupacion = nodo->cant_rutinas_mapper + nodo->cant_rutinas_reduce;
    return ocupacion;
}

t_nodo* buscarNodo(t_list* lista,struct in_addr ip, int puerto)
{
	t_nodo *nodoBuscado;
	bool buscar(t_nodo* nodo)
	{
		bool value = nodo->ip.s_addr == ip.s_addr && nodo->puerto == puerto;
		return value;
	}
	nodoBuscado = (t_nodo*)list_find(lista,buscar);
	return nodoBuscado;
}

void eliminarTarea(t_list* tareas,char*tarea)
{
	pthread_mutex_lock( &mutex );
	bool mismaTarea(char*elemento)
			{
			    bool value;
				int evaluacion = strcmp(elemento,tarea);
				if(evaluacion == 0)
				{
				     value = true;
				}
				else
				{
					 value = false;
				}
				return value;
			}
    list_remove_by_condition(tareas,(void*)mismaTarea);
    pthread_mutex_unlock( &mutex );
}

void imprimirEstado(t_nodo* nodo)
{
	switch(nodo->estado)
		{
	     case Conectado:    printf("\033[34mEstado:\033[37m Conectado\n");
	                        break;
	     case Desconectado: printf("\033[34mEstado:\033[34m Desconectado\n");
		                    break;
		}

}
