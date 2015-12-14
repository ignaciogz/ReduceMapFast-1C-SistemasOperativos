/*
 * comandosEjecutables.h
 *
 *  Created on: 8/5/2015
 *      Author: Ignacio Gutierrez
*/

#ifndef COMANDOS_EJECUTABLES_H_
#define COMANDOS_EJECUTABLES_H_

	/*DECLARACION DE PROTOTIPOS DE COMANDOS*/
	void ejecutarCrearDirectorio(char* rutaDelDirectorio);
	void ejecutarRenombrarDirectorio(char* nombreViejoDelDirectorio, char* nombreNuevoDelDirectorio);
	void ejecutarEliminarDirectorio(char* rutaDelDirectorio);
	void ejecutarMoverDirectorio(char* rutaDelDirectorio, char* rutaNuevaDelDirectorio);

	void ejecutarMoverArchivo(char* rutaVieja, char* rutaNueva);
	void ejecutarRenombrarArchivo(char* nombreViejo, char* nombreNuevo);
	void ejecutarEliminarArchivo(char* rutaDeUnArchivo);
	void ejecutarCopiarAlMDFS(char* nombreDelArch);

	void ejecutarFormatearMDFS(void);
	void ejecutarCopiarAlFS(char* nombreDelArchivo, char* rutaDeAlmacenamiento);
	void ejecutarObtenerMD5(char* nombreDelArchivo);

	void ejecutarVerBloquesDe(char* nombreDelArchivo);
	void ejecutarCopiarBloquesDe(char* nombreDelArchivo, int bloqueAClonar);
	void ejecutarBorrarBloquesDe(char* nombreDelArchivo, int numBloque, long ip_nodo, int puerto_nodo);

	void ejecutarAgregarNodo(char* IPdelNodo, int PuertoDelNodo, int EspacioDeDatos);
	void ejecutarEliminarNodo(char* IPdelNodo);
	/*FIN DE DECLARACION DE PROTOTIPOS*/

#endif
