/*
 * funcionesDeManejoDeDatos.h
 *
 *  Created on: 16/7/2015
 *      Author: utnso
 */

#ifndef HEADERS_FUNCIONESDEMANEJODEDATOS_H_
#define HEADERS_FUNCIONESDEMANEJODEDATOS_H_

//Almacena registro dentro de un numero de copia en el diccionario
void  agregarDatosCopiaNormal(t_estado_nodo* nodoDicc, int i, int bloqueAEscribir,int repeticiones);
void agregaCopia(t_dictionary* dicDeCopias, char* numeroCopia, long IP, int PUERTO, int NUMBLOQUE);
t_copiaX* registroCreate(long IP, int PUERTO, int NUMBLOQUE);
void agregarDatosCopiaGrande(t_estado_nodo* nodoDicc, int i, int bloqueAEscribir,int repeticiones);
void agregarCopia1ConResto(t_estado_nodo* nodoDicc, int bloqueAEscribir);
void agregaCopia23ConResto(t_estado_nodo* nodoDicc,int i,int bloqueAEscribir);
void LiberarEspacios();


#endif /* HEADERS_FUNCIONESDEMANEJODEDATOS_H_ */
