/*
 * Proceso_Job.c
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */

#include "Comunes_Job.h"
#include "Configuracion.h"
#include "ManejoMensajesJob.h"
#include "Interfaz_Job-Marta.h"

int main(int argc, char* argv[])
{
	printf("\033[32m Job initialized, waiting for connections..\033[37m\n");
	t_config* configuracion = config_create("config/config_job.cfg");
	t_log* logger = jlog_CrearLogConConsola();
	t_log* loggersin = jlog_CrearLogSinConsola();

	char* ipMarta = obtener_string(configuracion, "IP_MARTA");

	int puertoMarta = obtener_int(configuracion, "PUERTO_MARTA");

	char* ipJob = obtener_string(configuracion, "IP_JOB");

	int puertoJob = obtener_int(configuracion, "PUERTO_JOB");

	char* archivoMaper = obtener_string(configuracion, "MAPPER");

	char* archivoReduce = obtener_string(configuracion, "REDUCE");

	char* combiner = obtener_string(configuracion, "COMBINER");
	combiner_t soporteCombiner;
	if(strcmp(combiner, "SI"))
		soporteCombiner = ConCombiner;
	else
		soporteCombiner = SinCombiner;

	char* archivoResultado = obtener_string(configuracion, "RESULTADO");

	int cantidadArchivosAOperar = 0;

	//los archivos a operar se encontraran todos en una cadena separados por comas
	char* archivosAOperar = obtener_string(configuracion, "ARCHIVOS");

	char** archivos = string_split(archivosAOperar, ",");

	datosJob_t job;

	job.archivoMap = archivoMaper;
	job.archivoReduce = archivoReduce;
	job.archivoResultado = archivoResultado;
	job.archivosAOperar = list_create();

	int i;
	for(i = 0; *(archivos+i) != NULL; i++){
		list_add(job.archivosAOperar, *(archivos+i));
		cantidadArchivosAOperar = cantidadArchivosAOperar + 1;
	}
	job.combiner = soporteCombiner;
	inet_aton(ipMarta, &job.ipMarta);
	job.puertoMarta = puertoMarta;

	t_header cabecera;
	t_list* listaDeNodos = list_create();

	datoSolicitud_t solicitud;

	solicitud.job = job;
	solicitud.listaDeNodos = listaDeNodos;

	int nbytes;

	int socketMarta = enviarAMartaDatosDelJob(job, logger, loggersin);

	int socketJob;
	if((socketJob = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("No se pudo obtener el socket.");
		jlog_Error(logger, "No se pudo obtener el socket.");
		exit(1);
	}

	struct sockaddr_in addrJob;
	inet_aton(ipJob, &addrJob.sin_addr);
	addrJob.sin_family = AF_INET;
	addrJob.sin_port = puertoJob;
	memset(&addrJob.sin_zero, '\0', 8);

	if(bind(socketJob, (struct sockaddr*) &addrJob, sizeof(struct sockaddr_in)) == -1)
	{
		perror("No se pudo bindear el socket");
		jlog_Error(logger, "No se pudo bindear el socket");
		exit(1);
	}

	if(listen(socketJob,BACKLOG) == -1){
		perror("No se pudo hacer el Listen.");
		jlog_Error(logger, "No se pudo hacer el Listen.");
		exit(1);
	}

	fd_set master, readSet;
	//master tiene todos los fd; el readset tiene solo los que se pueden leer;

	FD_ZERO(&master);
	FD_SET(socketJob, &master);
	FD_SET(socketMarta, &master);

	int maxfd = socketJob;

	void* threadResult;

	while(1)
	{
		readSet = master;

		select(maxfd +1, &readSet, NULL, NULL, NULL );
		//i es el fd.
		for(i = 3; i < maxfd; i++) //Desde 3 por el STDIN, STDOUT,STDERR
		{
			if(FD_ISSET(i, &readSet)) // Socket dentro del readSet
			{
				nbytes = recv(i, (t_header*) &cabecera, sizeof(t_header), 0);
				if(nbytes <= 0)
				{
					/*if(nbytes < 0)
					{
						jlog_Error(logger, "recv");
						exit(1);
					}
					else
					{*/
						if(i == socketMarta)
						{
							printf("Desconexion del proceso MaRTA, finalizando job \n");
							exit(1);
						}
						notificarMarta(job.ipMarta, job.puertoMarta, operacionMapErronea, logger, loggersin);
						break;
					//}
				}

				//Defino el  hilo que se encargara de la solicitud
				pthread_t newThread;

				solicitud.cabecera = cabecera;
				solicitud.sock = i;

				//Creo el hilo nuevo
				pthread_create( &newThread, NULL, (void*)&manejarSolicitudes, &solicitud);
				/*char* parametros = string_new();
				string_append(parametros, "solicitud.cabecera");
				string_append(parametros, "solicitud.job");
				string_append(parametros, "solicitud.listaDeNodos");
				string_append(parametros, "solicitud.sock");
				jlog_CreaHilo(logger, "mapping", parametros);
				free(parametros);*/

				if(cabecera.tipoMensaje == pedidoMapping)
				{
					maxfd = agregarSocketsASet(&master, listaDeNodos);
					jlog_Cabecera(loggersin, "Nodo", cabecera.tipoMensaje);
					//Agrega los socket de los nodos al master.
				}
				if(cabecera.tipoMensaje == pedidoReduce)
				{
					pthread_join(newThread, &threadResult);
					jlog_Cabecera(loggersin, "Nodo", cabecera.tipoMensaje);
					jlog_FinHilo(logger, "FinHilo", (char*) &threadResult);
					break;
				}

				if(cabecera.tipoMensaje == ErrorSolicitud)
				{
					//jlog_Error(logger, "Imposible finalizar el job por problemas en el sistema");
					//jlog_Cabecera(loggersin, "Nodo", cabecera.tipoMensaje);
					printf("El archivo a operar no se encuentra disponible \n");
					exit(1);
				}

				if(cabecera.tipoMensaje == FinalizacionDeMap)
				{
					//remuevo el socket del bloque del set master
					FD_CLR(i, &master);
				}



			}
		}
	}
	//La solicitud de reduce ya fue mandada pero espero la respuesta
    //Luego del ciclo comienzo con el reduce
	FD_ZERO(&master);

	FD_SET(socketJob, &master);
	FD_SET(*(int*)threadResult, &master); //Agrego socket donde mande el reduce

	if(socketJob > (*(int*)threadResult))
		maxfd = socketJob;
	else
		maxfd = (*(int*)threadResult);
	//Veo cual es el mayor para poder ver el próximo select
	while(1)
		{
			readSet = master;

			select(maxfd +1, &readSet, NULL, NULL, NULL );
			//Solo se ocupa del nodo de reduce y de marta
			for(i = 3; i < maxfd; i++)
			{
				if(FD_ISSET(i, &readSet))
				{
					nbytes = recv(i, (t_header*) &cabecera, sizeof(t_header), 0);
					if(nbytes <= 0)
					{
						if(nbytes < 0)
						{
							jlog_Error(logger, "recv");
							exit(1);
						}
						else
						{
							notificarMarta(job.ipMarta, job.puertoMarta, operacionReduceErronea, logger, loggersin);
							break;
						}
					}

					//Defino el  hilo que se encargara de la solicitud
					pthread_t newThread;

					solicitud.cabecera = cabecera;
					solicitud.sock = i;

					//Creo el hilo nuevo
					pthread_create( &newThread, NULL, (void*)&manejarSolicitudes, &solicitud);

					if(cabecera.tipoMensaje == Error)
					{
						jlog_Error(logger, "Imposible finalizar el job por problemas en el sistema");
						jlog_Cabecera(loggersin, "Nodo", cabecera.tipoMensaje);
						exit(1);
					}

					if(cabecera.tipoMensaje == FinDeOperacion)
					{
						jlog_Error(logger, "El job ha finalizado correctamente");
						jlog_Cabecera(loggersin, "Nodo", cabecera.tipoMensaje);
						break;
					}

				}
			}
		}
	jlog_EliminarLog(logger);
	jlog_EliminarLog(loggersin);
	return EXIT_SUCCESS;
}
