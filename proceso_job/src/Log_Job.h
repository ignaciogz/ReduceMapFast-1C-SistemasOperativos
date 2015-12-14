/*
 * Log_Job.h
 *
 *  Created on: 24/7/2015
 *      Author: utnso
 */

#ifndef SRC_LOG_JOB_H_
#define SRC_LOG_JOB_H_

#include "Comunes_Job.h"
#include "ManejoMensajesJob.h"
//Define nombre del archivo log
#define LOG_FILE "Job.log"
#define str(x) #x
char* jlog_ObtenerDirectorioJob();
t_log* jlog_CrearLogConConsola();
t_log* jlog_CrearLogSinConsola();
void jlog_Conexion(t_log* logger, char* Nombre, char* Ip, int Puerto);
void jlog_Desconexion(t_log* logger, char* Nombre, char* Ip, int Puerto);
void jlog_CreaHilo(t_log* logger, char* Motivo, char* Parametros);
void jlog_FinHilo(t_log* logger, char* Motivo, char* Resultado);
void jlog_Cabecera(t_log* logger, char* Quien, t_mensaje Cabeceras);
char* jlog_ObtenerCabecera(t_mensaje cabecera);
void jlog_Info(t_log* logger, char* message);
void jlog_Error(t_log* logger, char* message);
void jlog_Trace(t_log* logger, char* message);
void jlog_Warning(t_log* logger, char* message);

#endif /* SRC_LOG_JOB_H_ */
