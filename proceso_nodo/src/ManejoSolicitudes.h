/*
 * ManejoSolicitudes.h
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */


#ifndef MANEJOSOLICITUDES_H_
#define MANEJOSOLICITUDES_H_

typedef struct{
	t_connection connection;
	char* espacioDatos;
	char* directorioTemporal;
	struct in_addr miIp;
	struct in_addr fsIp;
	int miPuerto;
	int fsPuerto;
}t_data;

//Dado los datos de una conexion recive la cabecera del mensaje y la envia para ejecutar la solicitud
void* receiveRequest(t_data* dataConnection);

//Dada la cabezera de un mensaje elije que solicitud es y realiza las acciones necesarias para satisfacerla
void execRequest(t_header header, t_connection connection, char* espacioDatos, char* directorioTemporal, struct in_addr myip,
					int miPuerto, struct in_addr fsIp, int fsPuerto);
#endif /* MANEJOSOLICITUDES_H_ */
