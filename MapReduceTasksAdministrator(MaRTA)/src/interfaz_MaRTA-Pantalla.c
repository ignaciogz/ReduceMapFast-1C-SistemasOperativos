#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include "interfaz_MaRTA-Pantalla.h"
#include "interfaz_MaRTA-Job.h"
#include "interfaz_MaRTA-filesystem.h"
#include <pthread.h>

void mostrarEstadoNodos (t_list* listaNodos)
{
	printf("           Informacion nodos          \n");
	printf("\n");
	list_iterate(listaNodos,(void*)&imprimirIpYEstado);
	printf("\n");
};

void mostrarTareasPendientesJob (t_list* listaJobs)
{
	printf("           Informacion jobs          \n");
	printf("\n");
	if(list_size(listaJobs) > 0)
		list_iterate(listaJobs,(void*)&imprimirIpPendientesDeMappingYPendientesDeReduce);
	printf("\n");
};


void mostrarPorPantalla (t_parametroPantalla* parametros)
{
		pthread_join(parametros->hilo, NULL);
	    printf("\033[34m================================================\n");
	    printf("                   Estado actual                \n");
	    printf("================================================\033[37m\n");
	    printf("\n");
	    mostrarEstadoNodos (parametros->listaNodos);
        mostrarTareasPendientesJob (parametros->listaJobs);
        printf("\n");
        printf("--------------------------------------------------\033[37m\n");

}
