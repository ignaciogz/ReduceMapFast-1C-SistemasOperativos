/*
 * log_job.c

 *
 *  Created on: 27/6/2015
 *      Author: utnso
 */

#include "Log_Job.h"

/*Archivo de Log:
 * P: Pantalla L: Log
 * Conexión y desconexión al proceso MaRTA con IP y puerto. P L
 * Creación de hilos, motivo y parámetros recibidos. P L
 * Finalización de hilos, motivo y resultado P L
 * Cabeceras de mensajes enviados y recibidos de cada hilo/proceso. L
 *
 * */

pthread_mutex_t loggerMutex = PTHREAD_MUTEX_INITIALIZER;


char* jlog_ObtenerDirectorioJob() {
   char* cwd[1024];
   if (getcwd(cwd, sizeof(cwd)) != NULL) //Obtengo directorio del proceso
       fprintf(stdout, "Current working dir: %s\n", cwd);
   else
       perror("getcwd() error");
   return cwd; //Añado nombre del archivo de logeo
}


t_log* jlog_CrearLogConConsola(){
	t_log* logger = log_create(LOG_FILE, "ProcesoJob", true, LOG_LEVEL_INFO);
	return logger;
}
t_log* jlog_CrearLogSinConsola(){
	t_log* logger = log_create(LOG_FILE, "ProcesoJob", false, LOG_LEVEL_INFO);
	return logger;
}

void jlog_EliminarLog(t_log* logger){
	if(logger != NULL){
		log_destroy(logger);
	}
	return;
}

void jlog_Conexion(t_log* logger, char* Nombre, char* Ip, int Puerto) {
    char* message;

   	message = string_new();

   	string_append(&message, "Conectado: ");
   	string_append(&message, Nombre);
   	string_append(&message, ", IP: ");
   	string_append(&message, Ip);
   	string_append(&message, ", PORT: ");
   	string_append(&message, string_itoa(Puerto));
   	string_append(&message, "\n");

   	jlog_Info(logger, message);

   	free(message);

   	return;
}

void jlog_Desconexion(t_log* logger, char* Nombre, char* Ip, int Puerto) {
    char* message;
   	message = string_new();

   	string_append(&message, "Desconectado: ");
   	string_append(&message, Nombre);
   	string_append(&message, ", IP: ");
   	string_append(&message, Ip);
   	string_append(&message, ", PORT: ");
   	string_append(&message, string_itoa(Puerto));
   	string_append(&message, '\n');

   	jlog_Info(logger, message);

   	free(message);

   	return;
}

void jlog_CreaHilo(t_log* logger, char* Motivo, char* Parametros) {
    char* message;
	message = string_new();

	string_append(&message, "Creado hilo por motivo: ");
	string_append(&message, Motivo);
	string_append(&message, ", con los parametros: ");
	string_append(&message, Parametros);
	string_append(&message, "\n");

	jlog_Info(logger, message);

	free(message);

	return;
}

void jlog_FinHilo(t_log* logger, char* Motivo, char* Resultado) {
    char* message;
	message = string_new();

	string_append(&message, "Finalizado hilo por motivo: ");
	string_append(&message, Motivo);
	string_append(&message, ", con el resultado: ");
	string_append(&message, Resultado);
	string_append(&message, "\n");

	jlog_Info(logger, message);

	free(message);

	return;
}

void jlog_Cabecera(t_log* logger, char* Quien, t_mensaje Cabecera) {
    char* message;

	message = string_new();

	string_append(&message, "Mensaje de: ");
	string_append(&message, Quien);
	string_append(&message, ", Cabeceras: ");
	string_append(&message, jlog_ObtenerCabecera(Cabecera));
	string_append(&message, "\n");

	jlog_Info(logger, message);

	free(message);

	return;
}

char* jlog_ObtenerCabecera(t_mensaje cabecera){
	switch(cabecera){
	case SolicitudDeMap:
		return "SolicitudDeMap";
		break;
	case FinalizacionDeMap:
		return "FinalizacionDeMap";
		break;
	case SolicitudDeReduceSinCombiner:
		return "SolicitudeDeReduceSinCombiner";
		break;
	case FinalizacionDeReduce:
		return "FinalizacionDeReduce";
		break;
	case datosJob:
		return "datosJob";
		break;
	case pedidoMapping:
		return "pedidoMapping";
		break;
	case operacionMapExitosa:
		return "operacionMapExitosa";
		break;
	case operacionMapErronea:
		return "operacionMapErronea";
		break;
	case pedidoReduce:
		return "pedidoReduce";
		break;
	case operacionReduceExitosa:
		return "operacionReduceExitosa";
		break;
	case FinDeOperacion:
		return "FinDeOperacion";
		break;
	case Error:
		return "Error";
		break;
	default:
		return NULL;
		break;
	}

}

void jlog_Info(t_log* logger, char* message){
	pthread_mutex_lock(&loggerMutex);
	log_info(logger, message);
	pthread_mutex_unlock(&loggerMutex);
	return;

}

void jlog_Error(t_log* logger, char* message){
	pthread_mutex_lock(&loggerMutex);
	log_error(logger, message);
	pthread_mutex_unlock(&loggerMutex);
	return;

}

void jlog_Trace(t_log* logger, char* message){
	pthread_mutex_lock(&loggerMutex);
	log_trace(logger, message);
	pthread_mutex_unlock(&loggerMutex);
	return;

}

void jlog_Warning(t_log* logger, char* message){
	pthread_mutex_lock(&loggerMutex);
	log_warning(logger, message);
	pthread_mutex_unlock(&loggerMutex);
	return;

}

/*void jlog_WithThread(){
    pthread_t th1, th2;
    //PTHREAD_MUTEX_STALLED a;

    if (LOG_FILE != NULL) {

        pthread_create(&th1, NULL, (void*) jlog_Conexion("HOLA"), LOG_FILE);
        pthread_create(&th2, NULL, (void*) jlog_Conexion("HOLA"), LOG_FILE);

        pthread_join(th1, NULL);
        pthread_join(th2, NULL);
        printf("\nRevisar el archivo de log que se creo en: %s\n", LOG_FILE);
    } else {
        printf("No se pudo generar el archivo de log\n");
    }
}*/
