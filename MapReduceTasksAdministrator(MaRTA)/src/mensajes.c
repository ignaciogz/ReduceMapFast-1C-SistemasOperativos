#include "mensajes.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <pthread.h>
#include "interfaz_MaRTA-filesystem.h"
#include "interfaz_MaRTA-Job.h"
#include "interfaz_MaRTA-Pantalla.h"
#include <pthread.h>




void atenderMensaje(t_parametro* parametros)
{
	int puerto=0;
	char* ipJob=string_new();
	t_job* job;
	int indice = 0;
	t_cabecera cabecera;
	int socketNuevo=0,i=0,cantidadNodos=0;
	void* mensaje;

	switch(parametros->cabecera.mensaje)
	{

	case datosJob:               atenderJob(parametros->conexion,parametros->listaJobs,parametros->direccionFs,socketNuevo);
	                             break;

	case datosBloques:           atenderBloque(parametros->conexion,parametros->listaJobs,parametros->listaNodos,socketNuevo);
	                             break;

	case errorSolicitud:         recibirErrorSolicitud(parametros->conexion,indice);
								 job = list_get(parametros->listaJobs, indice);
	                             struct sockaddr_in direccionJob=job->direccion;
	                             //vincularSocket(&socketNuevo,direccionJob);
                                 cabecera.mensaje = errorSolicitud;
                                 cabecera.tamanio= 0;
                                 mensaje = malloc(sizeof(t_cabecera));
                                 empaquetarCabecera(cabecera,mensaje);
                                 enviarTodo(job->sock,mensaje,sizeof(t_cabecera)); // el mensaje a enviar tiene solo el header
                                 free(mensaje);
                                 list_remove(parametros->listaJobs,indice);
                                 free(job);
                                 //close(socketNuevo);
                                 break;

	case operacionMapErronea:    operacionErronea(parametros->conexion);
	                             break;

	case operacionReduceErronea: operacionErronea(parametros->conexion);
                                 break;

	case ConexionFileSystem:     cargarNodo(parametros->conexion,parametros->listaNodos);
	                             break;

	case operacionMapExitosa:    gestionarResultadoMap(parametros->conexion,parametros->listaJobs,parametros->listaNodos);
                                 break;

	case operacionReduceExitosa: gestionarReduce(parametros->conexion,parametros->listaJobs,parametros->listaNodos,parametros->direccionFs);
	                             break;

	case nodoNuevo:              cargarNodo(parametros->conexion,parametros->listaNodos);
	                             break;

	case cambioDeEstado:         recibiryModificarEstado(parametros->conexion,parametros->listaNodos);
	                             break;
	}
}
