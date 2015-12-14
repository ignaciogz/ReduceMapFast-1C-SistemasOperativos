/*
 * consola.c
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include "headers/consola.h"
#include <string.h>

#include "headers/subPantallasDeLaConsola.h"

void * consola (void){
	int valorIngresado;

	//Listar opciones para el usuario y esperar una seleccion
	printf("\n\033[33m/*------- C O N S O L A -------*/\033[37m\n");
	printf("Presione '0' para cerrar la consola\n\n");
	listarComandos();

	printf("Seleccione el TIPO DE COMANDO a ejecutar: ");
	scanf("%d", &valorIngresado);
	getchar();

	t_comando tipoDeComando = valorIngresado;

	//Selecciono un tipo de comando
	switch(tipoDeComando)
	{
		case DIRECTORIO:
			comandosDeDirectorio();
			break;

		case ARCHIVOLOCAL:
			comandosDeArchivoLocal();
			break;

		case MDFS:
			comandosDelMDFS();
			break;

		case BLOQUE:
			comandosDeBloque();
			break;

		case NODO:
			comandosDeNodo();
			break;

		case Cerrar_Consola:
			exit(EXIT_SUCCESS);
			break;

		default:
			printf("Por favor, ingrese un tipo de comando valido\n");
			printf("Presione una tecla para continuar: ");

			char unaTecla;
			scanf("%s", &unaTecla);
			consola();
			break;
	}
	return NULL;
}

void listarComandos(void){
	printf("\t1 - DIRECTORIO\n");
	printf("\t2 - ARCHIVO LOCAL\n");
	printf("\t3 - MDFS\n");
	printf("\t4 - BLOQUE\n");
	printf("\t5 - NODO\n\n");
}
