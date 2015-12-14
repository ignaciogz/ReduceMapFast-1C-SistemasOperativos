/*
 * inerfaz_MaRTA-Pantalla.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef MAPREDUCETASKSADMINISTRATOR_MARTA__INTERFAZ_MARTA_PANTALLA_H_
#define MAPREDUCETASKSADMINISTRATOR_MARTA__INTERFAZ_MARTA_PANTALLA_H_

#include "mensajes.h"

//dada una lista de nodos, muestra por pantalla el estado de sus nodos
void mostrarEstadoNodos (t_list* listaNodos);

// dada una lista de nodos, muestra por pantalla las tareas en ejecucuion de cada nodo
void mostrarTareasEnEjecucionNodos (t_list* listaNodos);

// dada una lista de jobs, muestra por pantalla la cantidad de tareas pendientes de mapping y reduce de cada job
void mostrarTareasPendientesJob (t_list* listaJobs);

void mostrarPorPantalla (t_parametroPantalla* parametros);
#endif /* MAPREDUCETASKSADMINISTRATOR_MARTA__INTERFAZ_MARTA_PANTALLA_H_ */
