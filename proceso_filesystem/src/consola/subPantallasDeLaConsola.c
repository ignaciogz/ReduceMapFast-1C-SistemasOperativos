/*
 * subPantallasDeLaConsola.c
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/
#include "headers/subPantallasDeLaConsola.h"

#include "headers/funcionesDeSubPantallas.h"
#include "headers/comandosEjecutables.h"
#include "headers/consola.h"
#include <stdio.h>
#include <stdlib.h>

/*
	///////////////////////////////////////////////////////////
	SUBPANTALLA DIRECTORIO
	///////////////////////////////////////////////////////////
*/
void comandosDeDirectorio(void){
	mostrarHeaderDeLaConsola();
	printf("crearDirectorio <rutaDelDirectorio>\n");
	printf("renombrarDirectorio <nombreViejoDelDirectorio> <nombreNuevoDelDirectorio>\n");
	printf("moverDirectorio <rutaDelDirectorio> <rutaNuevaDelDirectorio>\n");
	printf("eliminarDirectorio <rutaDelDirectorio>\n\n");

	char** instruccionIngresada = solicitarInstruccionAlUsuario();

	t_cmdDeDirectorio comando = getComando(instruccionIngresada);

	char * parametro1;
	char * parametro2;

	switch(comando){
		case CREAR_DIRECTORIO:

			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarCrearDirectorio(parametro1);

			liberar1Parametro(parametro1);
			break;

		case RENOMBRAR_DIRECTORIO:

			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);

			ejecutarRenombrarDirectorio(parametro1, parametro2);

			liberar2Parametros(parametro1, parametro2);
			break;

		case ELIMINAR_DIRECTORIO:

			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarEliminarDirectorio(parametro1);

			liberar1Parametro(parametro1);
			break;

		case MOVER_DIRECTORIO:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);

			ejecutarMoverDirectorio(parametro1, parametro2);

			liberar2Parametros(parametro1, parametro2);
			break;


		case IR_A_MENU_PRINCIPAL_D:
			liberarInstruccion(instruccionIngresada);
			consola();
			break;

		case CERRAR_CONSOLA_D:
			liberarInstruccion(instruccionIngresada);
			exit(EXIT_SUCCESS);
			break;

		default:
			printf("Por favor, reingrese una instruccion valida \n");
			printf("Presione una tecla para continuar: ");

			char unaTecla;
			scanf("%c", &unaTecla);
			liberarInstruccion(instruccionIngresada);
			comandosDeDirectorio();
			break;
	}
	liberarInstruccion(instruccionIngresada);
	getchar();
	comandosDeDirectorio();
}


/*
	///////////////////////////////////////////////////////////
	SUBPANTALLA ARCHIVO
	///////////////////////////////////////////////////////////
*/
void comandosDeArchivoLocal(void){
	mostrarHeaderDeLaConsola();
	printf("moverArchivo <rutaVieja> <rutaNueva>\n");
	printf("renombrarArchivo <nombreViejo> <nombreNuevo>\n");
	printf("eliminarArchivo <rutaDeUnArchivo>\n");
	printf("copiarAlMDFS <nombreDelArchivo>\n\n");

	char** instruccionIngresada = solicitarInstruccionAlUsuario();

	t_cmdDeArchivoLocal comando = getComando(instruccionIngresada);
	char * parametro1;
	char * parametro2;

	switch(comando){
		case MOVER_ARCHIVO:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);

			ejecutarMoverArchivo(parametro1, parametro2);

			liberar2Parametros(parametro1, parametro2);
			break;

		case RENOMBRAR_ARCHIVO:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);

			ejecutarRenombrarArchivo(parametro1, parametro2);

			liberar2Parametros(parametro1, parametro2);
			break;

		case ELIMINAR_ARCHIVO:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarEliminarArchivo(parametro1);

			liberar1Parametro(parametro1);
			break;

		case COPIAR_AL_MDFS:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarCopiarAlMDFS(parametro1);

			liberar1Parametro(parametro1);
			break;

		case IR_A_MENU_PRINCIPAL_ARC:
			liberarInstruccion(instruccionIngresada);
			consola();
			break;

		case CERRAR_CONSOLA_ARC:
			liberarInstruccion(instruccionIngresada);
			exit(EXIT_SUCCESS);
			break;

		default:
			printf("Por favor, reingrese una instruccion valida \n");
			printf("Presione una tecla para continuar: ");

			char unaTecla;
			scanf("%c", &unaTecla);
			liberarInstruccion(instruccionIngresada);
			comandosDeArchivoLocal();
			break;
	}
	liberarInstruccion(instruccionIngresada);
	getchar();
	comandosDeArchivoLocal();
}

/*
	///////////////////////////////////////////////////////////
	SUBPANTALLA MDFS
	///////////////////////////////////////////////////////////
*/

void comandosDelMDFS(void){
	mostrarHeaderDeLaConsola();
	printf("formatear\n");
	printf("copiarAlFS <nombreDelArchivo> <rutaDeAlmacenamiento>\n");
	printf("obtenerMD5 <nombreDelArchivo>\n\n");

	char** instruccionIngresada = solicitarInstruccionAlUsuario();

	t_cmdDeMDFS comando = getComando(instruccionIngresada);
	char * parametro1;
	char * parametro2;

	switch(comando){
		case FORMATEAR_MDFS:
			ejecutarFormatearMDFS();
			break;

		case COPIAR_AL_FS:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);

			ejecutarCopiarAlFS(parametro1, parametro2);

			liberar2Parametros(parametro1, parametro2);
			break;

		case OBTENER_MD5:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarObtenerMD5(parametro1);

			liberar1Parametro(parametro1);
			break;

		case IR_A_MENU_PRINCIPAL_M:
			liberarInstruccion(instruccionIngresada);
			consola();
			break;

		case CERRAR_CONSOLA_M:
			liberarInstruccion(instruccionIngresada);
			exit(EXIT_SUCCESS);
			break;

		default:
			printf("Por favor, reingrese una instruccion valida \n");
			printf("Presione una tecla para continuar: ");

			char unaTecla;
			scanf("%c", &unaTecla);
			liberarInstruccion(instruccionIngresada);
			comandosDelMDFS();
			break;
	}
	liberarInstruccion(instruccionIngresada);
	getchar();
	comandosDeBloque();
}

/*
	///////////////////////////////////////////////////////////
	SUBPANTALLA BLOQUE
	///////////////////////////////////////////////////////////
*/

void comandosDeBloque(void){
	mostrarHeaderDeLaConsola();
	printf("verBloquesDe <nombreDelArchivo>\n");
	printf("copiarBloquesDe <nombreDelArchivo> <numeroDeBloqueAClonar>\n");
	printf("borrarBloquesDe <nombreDelArchivo> <numeroDeBloque> <numeroDeIPdelNodo> <numeroDePuertoDelNodo>\n\n");

	char** instruccionIngresada = solicitarInstruccionAlUsuario();

	t_cmdDeBloque comando = getComando(instruccionIngresada);
	char * parametro1;
	char * parametro2;
	char * parametro3;
	char * parametro4;

	int parametro2num;
	long parametro3num;
	int parametro4num;

	switch(comando){
		case VER_BLOQUES_DE:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarVerBloquesDe(parametro1);

			liberar1Parametro(parametro1);
			break;

		case COPIAR_BLOQUES_DE:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);

			parametro2num = atoi(parametro2);

			ejecutarCopiarBloquesDe(parametro1, parametro2num);

			liberar2Parametros(parametro1, parametro2);
			break;

		case BORRAR_BLOQUES_DE:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));
			parametro3 = getMemoriaParaParametro(getParametro3(instruccionIngresada));
			parametro4 = getMemoriaParaParametro(getParametro4(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);
			parametro3 = getParametro3(instruccionIngresada);
			parametro4 = getParametro4(instruccionIngresada);

			parametro2num = atoi(parametro2);
			parametro3num = atol(parametro3);
			parametro4num = atoi(parametro4);

			ejecutarBorrarBloquesDe(parametro1, parametro2num, parametro3num, parametro4num);

			liberar4Parametros(parametro1, parametro2, parametro3, parametro4);
			break;

		case IR_A_MENU_PRINCIPAL_B:
			liberarInstruccion(instruccionIngresada);
			consola();
			break;

		case CERRAR_CONSOLA_B:
			liberarInstruccion(instruccionIngresada);
			exit(EXIT_SUCCESS);
			break;

		default:
			printf("Por favor, reingrese una instruccion valida \n");
			printf("Presione una tecla para continuar: ");

			char unaTecla;
			scanf("%c", &unaTecla);
			liberarInstruccion(instruccionIngresada);
			comandosDeBloque();
			break;
	}
	liberarInstruccion(instruccionIngresada);
	getchar();
	comandosDeBloque();
}

/*
	///////////////////////////////////////////////////////////
	SUBPANTALLA NODO
	///////////////////////////////////////////////////////////
*/

void comandosDeNodo(void){
	mostrarHeaderDeLaConsola();
	printf("agregarNodo <IPdelNodo 192.123.145.56> <numeroDePuertoDelNodo> <EspacioTotalDeDatos>\n");
	printf("eliminarNodo <IPdelNodo 192.123.145.56>\n\n");

	char** instruccionIngresada = solicitarInstruccionAlUsuario();

	t_cmdDeNodo comando = getComando(instruccionIngresada);
	char * parametro1;
	char * parametro2;
	char * parametro3;

	switch(comando){
		case AGREGAR_NODO:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro2 = getMemoriaParaParametro(getParametro2(instruccionIngresada));
			parametro3 = getMemoriaParaParametro(getParametro3(instruccionIngresada));

			parametro1 = getParametro1(instruccionIngresada);
			parametro2 = getParametro2(instruccionIngresada);
			parametro3 = getParametro3(instruccionIngresada);

			int parametro2num = atoi(parametro2);
			int parametro3num = atoi(parametro3);

			ejecutarAgregarNodo(parametro1, parametro2num, parametro3num);

			liberar3Parametros(parametro1, parametro2, parametro3);
			break;

		case ELIMINAR_NODO:
			parametro1 = getMemoriaParaParametro(getParametro1(instruccionIngresada));
			parametro1 = getParametro1(instruccionIngresada);

			ejecutarEliminarNodo(parametro1);

			liberar1Parametro(parametro1);
			break;

		case IR_A_MENU_PRINCIPAL_N:
			liberarInstruccion(instruccionIngresada);
			consola();
			break;

		case CERRAR_CONSOLA_N:
			liberarInstruccion(instruccionIngresada);
			exit(EXIT_SUCCESS);
			break;

		default:
			printf("Por favor, reingrese una instruccion valida \n");
			printf("Presione una tecla para continuar: ");

			char unaTecla;
			scanf("%c", &unaTecla);
			liberarInstruccion(instruccionIngresada);
			comandosDeNodo();
			break;
	}
	liberarInstruccion(instruccionIngresada);
	getchar();
	comandosDeNodo();
}
