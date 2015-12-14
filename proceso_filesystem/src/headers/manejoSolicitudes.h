/*
 * manejoSolicitudes.h
 *
 *  Created on: 5/6/2015
 *      Author: utnso
 */
#ifndef HEADERS_MANEJOSOLICITUDES_H_
#define HEADERS_MANEJOSOLICITUDES_H_

typedef struct{
	t_connection connection;
	t_header header;
}t_data;

//Acepta las conexiones
void* hiloSecundario(void* dataConnection);
//Dada la cabezera de un mensaje elije que solicitud es y realiza las acciones necesarias para satisfacerla
void* execRequest(t_data* data);
//Acepta conexiones de nodo y en caso de llegada marta se la ignora
void execConexion(t_header header, t_connection conexion);
#endif /* HEADERS_MANEJOSOLICITUDES_H_ */

