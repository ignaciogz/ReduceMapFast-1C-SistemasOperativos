/*
 * manejoDeDatos.c
 *
 *  Created on: 4/7/2015
 *      Author: utnso
 */
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <pthread.h>

#include "headers/Log.h"
#include "headers/socketsFs.h"
#include "headers/manejoDeDatos.h"
#include "headers/funcionesDeManejoDeDatos.h"
#include "mongodb/headers/mongodb.h"

//Variables Globales
extern int cantidadDeNodosNecesarios;
extern int cantNodosActivos;
extern int totalBloquesLibres;
extern t_list* listaIp;
extern t_dictionary* nodosHistorial;
extern pthread_mutex_t historialNodos;
extern pthread_mutex_t historialIP;

t_dictionary* diccionarioDeBloques;


int cargarBloquesNodo(char* nombreDeUnArchivo)
{

	long tamanioArch;
	ldiv_t bloquesArchivo;
	t_IpViejas Ip_Repetidas;
	Ip_Repetidas.Ip1=(char*)malloc(15*sizeof(char));
	Ip_Repetidas.Ip2=(char*)malloc(15*sizeof(char));
	Ip_Repetidas.historial=0;
	diccionarioDeBloques= dictionary_create();
	int repeticiones;

	tamanioArch = getFileLength(nombreDeUnArchivo);
	bloquesArchivo = ldiv(tamanioArch,TAMANIO_BLOQUE);
	if(bloquesArchivo.rem !=0){
		tamanioArch= tamanioArch + 1;
	}

	int fd;
	fd=open(nombreDeUnArchivo, O_RDWR);


	if(bloquesArchivo.rem==0)
	{
		if((muchoEspacio(bloquesArchivo,totalBloquesLibres)==1) && (muchosNodos(cantNodosActivos)==1))
		{
			repeticiones=1;
			if(distribucionGrande(nombreDeUnArchivo,bloquesArchivo, fd, Ip_Repetidas,repeticiones)==0)
				return 0;
			//Realizo las 2 copias que faltan
			int i,origen=3;
			for(i=2;i <= 3;i++){
				if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
				return 0;

			}

		}else{
			if((muchoEspacio(bloquesArchivo,totalBloquesLibres)!=1) && (muchosNodos(cantNodosActivos)==1))
			{
				int i,origen=1;
				//Realizo las 3 copias juntas
				for(i=1;i <= 3;i++){
					if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
					return 0;

				}

			}else{
				if((muchoEspacio(bloquesArchivo,totalBloquesLibres)==1) && (muchosNodos(cantNodosActivos)!=1))
				{
					repeticiones=1;
					if(distribucionGrande(nombreDeUnArchivo,bloquesArchivo, fd, Ip_Repetidas,repeticiones)==0)
						return 0;
					//Realizo las 2 copias que faltan
					int i,origen=3;
					for(i=2;i <= 3;i++){
						if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
						return 0;

						}
				}else{
					int i,origen=1;
					//Realizo las 3 copias juntas
					for(i=1;i <= 3;i++){
						if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
						return 0;
						}
					}
				}
			}
	}else{
		if((muchoEspacio(bloquesArchivo,totalBloquesLibres)==1) && (muchosNodos(cantNodosActivos)==1))
	  	{
	  		repeticiones=1;
			if(distribucionGrandeConResto(nombreDeUnArchivo,bloquesArchivo,tamanioArch,fd,Ip_Repetidas,repeticiones)==0)
	  		return 0;
			//Realizo las 2 copias con resto '\0' que faltan
	  		int i;
	  		int origen=4;
	  		for(i=2; i <= 3;i++ ){
	  			if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
	  			return 0;
	  			t_nodo nodo;
	  			nodo=nodoConPrioridad(Ip_Repetidas);
	  			if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,i)==0)
	  				return 0;

	  		}

	  	}else{
	  		if((muchoEspacio(bloquesArchivo,totalBloquesLibres)!=1) && (muchosNodos(cantNodosActivos)==1))
			{
	  			//Realizo las 3 copias con resto juntas
	  			int i,origen=1;
	  			for(i=1;i <= 3;i++){
	  				if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
	  					return 0;
				t_nodo nodo;
				nodo=nodoConPrioridad(Ip_Repetidas);
				if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,i)==0)
					return 0;
	  			}

			}else{
				if((muchoEspacio(bloquesArchivo,totalBloquesLibres)==1) && (muchosNodos(cantNodosActivos)!=1))
				{
					repeticiones=1;
					if(distribucionGrandeConResto(nombreDeUnArchivo,bloquesArchivo,tamanioArch,fd,Ip_Repetidas,repeticiones)==0)
						return 0;
					//Realizo las 2 copias con resto '\0' que faltan
					int i;
					int origen=4;
					for(i=2; i <= 3;i++ ){
						if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
							return 0;
					  	t_nodo nodo;
					  	nodo=nodoConPrioridad(Ip_Repetidas);
					  	if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,i)==0)
					  		return 0;

					  	}

	  		}else{
	  			//Realizo las 3 copias con resto juntas
	  			int i,origen=1;
	  			for(i=1;i <= 3;i++){
	  				if(distribucionNormal(nombreDeUnArchivo,bloquesArchivo,fd,Ip_Repetidas,origen,i)==0)
	  					 return 0;
	  				t_nodo nodo;
	  				nodo=nodoConPrioridad(Ip_Repetidas);
	  				if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,i)==0)
	  					return 0;
	  				}

	  		}
	  	}

    }

  }
  int dirFake=0;
  insertarArchivo_DB(nombreDeUnArchivo,tamanioArch,dirFake,diccionarioDeBloques);
  return 1;
}


int muchoEspacio(ldiv_t bloquesArchivo,int cantBloquesLibres){
	if(cantBloquesLibres >= bloquesArchivo.quot*4)
	{
		return 1;
	}else{return 0;}

}

int muchosNodos (int cantNodosActivos){
	if(cantNodosActivos >= cantidadDeNodosNecesarios + 2)
	{return 1;
	}else{return 0;}
}

long getFileLength (char* fileName)
{
	FILE* fileDescriptor = fopen(fileName, "r"); //se abre el archivo solicitado para lectura
	long fileLength;

	fseek(fileDescriptor, 0, SEEK_END); //se posiciona el cursor del archivo sobre el final
	fileLength = ftell(fileDescriptor); //se devuelve la posicion en bytes en la que esta posicionado el cursor

	fclose(fileDescriptor);

	return fileLength;
}

t_nodo nodoConPrioridad(t_IpViejas IpRepe){
	int bloquesLibres,i,cantElementosLista;
	int maxBloques=0;
	struct in_addr* IP;
	t_estado_nodo* nodoDicc;
	t_nodo nodo;
	char* IP_Lista;


	pthread_mutex_lock( &historialIP );
	cantElementosLista= list_size(listaIp);
	pthread_mutex_unlock( &historialIP );

	for(i=0;i < cantElementosLista; i++)
	{
		pthread_mutex_lock( &historialIP );
		IP=list_get(listaIp,i);
		pthread_mutex_unlock( &historialIP );

		IP_Lista = inet_ntoa(*IP);
		pthread_mutex_lock( &historialNodos );
		nodoDicc =(t_estado_nodo*) dictionary_get(nodosHistorial,IP_Lista );
		pthread_mutex_unlock( &historialNodos );

		if(nodoDicc->estado == 0)
		{
			bloquesLibres= cantBloquesLibres(nodoDicc->estadoBloques);
			if((bloquesLibres > maxBloques) && (noSeRepite(IP_Lista,IpRepe)))
			{
				maxBloques = bloquesLibres;
				nodo.Ip=nodoDicc->datosNodo.connectionAddr.sin_addr;
				nodo.bitarray=nodoDicc->estadoBloques;
			}
		}

	}
actualizarIpVieja(nodo.Ip,IpRepe);
return nodo;
}

int cantBloquesTotal(){
	int cantLista,i,tamanioArray;
	int bloquesTotal=0;//almaceno bloques total nodo
	struct in_addr* IP = malloc(sizeof(struct in_addr));
	t_estado_nodo* nodoDicc = malloc(sizeof(t_estado_nodo));

	pthread_mutex_lock( &historialIP );
	cantLista=list_size(listaIp);

	for(i=0;i < cantLista; i++){
		IP=(struct in_addr*)list_get(listaIp,i);
		pthread_mutex_unlock( &historialIP );

		pthread_mutex_lock( &historialNodos );
		nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,inet_ntoa(*IP));
		pthread_mutex_unlock( &historialNodos );

		if(nodoDicc->estado==0){
			tamanioArray = bitarray_get_max_bit(nodoDicc->estadoBloques);
			bloquesTotal = bloquesTotal + tamanioArray;
		}
	}


free(nodoDicc);
return bloquesTotal;
}




int cantBloquesLibres(t_bitarray* estadBloques){
	int i,bit,resultado,bit_posicion;
	int bloquesLibres=0;
	bit=bitarray_get_max_bit(estadBloques);
	resultado=bit/8;
		for(i=0;i < resultado;i++)
		{
			bit_posicion=bitarray_test_bit(estadBloques,8*i);
			if(bit_posicion==0){
				bloquesLibres=bloquesLibres+1;
			}
		}
	return bloquesLibres;
}

int obtenerBloqueLibre(t_bitarray* bitarray){
	int bit,i,resultado,bit_posicion;
	bit = bitarray_get_max_bit(bitarray);
	resultado = bit/8;
	for(i=0;i < resultado;i++)
	{
		bit_posicion = bitarray_test_bit(bitarray,i);
		if(bit_posicion==0){
			break;
		}
	}
return i;
}

int cantDividirArchivo(int bloquesArch){
	if(bloquesArch <= 3 ){
		return 1;
	}else{
		if(bloquesArch <= 20 ){
			return 2;
		}else{ return 3; }
	}

}

int distribucionGrande(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, int fd,t_IpViejas IpRepe,int repeticiones)
{
		/*variables*/
		t_nodo nodo;
     	nodo=nodoConPrioridad(IpRepe);
		int bloquesLibresNodo;
		int origen = 3;

		bloquesLibresNodo = cantBloquesLibres(nodo.bitarray);
		if(bloquesLibresNodo >= bloquesArchivo.quot)
		{
			int i;
			for(i=0; i < bloquesArchivo.quot; i++)
			{
				if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
					return 0;
			}
		}else{
			int i=0;
			while(i < bloquesLibresNodo)
			{
				if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
					return 0;
					i++;
			}
			int bloquesSinMap;
			bloquesSinMap= bloquesArchivo.quot - bloquesLibresNodo;
			int j;
			nodo=nodoConPrioridad(IpRepe);
			for(j=0; j < bloquesArchivo.quot; j++)
						   {
								if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
									return 0;
						   }

	         }
return 1;
}

int distribucionNormal(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, int fd,t_IpViejas IpRepe, int origen,int repeticiones)
{
	int i;
	for(i=0; i < bloquesArchivo.quot; i++)
		{
		 	t_nodo nodo;
			nodo=nodoConPrioridad(IpRepe);
			if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
			return 0;
		}
return 1;
}

t_nodo distribucionMediana(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, int fd,t_IpViejas IpRepe,int repeticiones)
{
   /*variables*/
	int cantADividir;
	cantADividir = cantDividirArchivo(bloquesArchivo.quot);
	int i;
	int origen =8;
	div_t dividirArchivo = div(bloquesArchivo.quot,cantADividir);
	int bloquesLibresNodo;

	for(i=0; i < cantADividir; i++)
	{
		t_nodo nodo;
		nodo=nodoConPrioridad(IpRepe);

		bloquesLibresNodo = cantBloquesLibres(nodo.bitarray);
		int j=0;
		while(j < dividirArchivo.quot)
		{	//Modificar este mando de mensaje i*j+TamanioBLoque
			mandarMensaje(nodo,fd,i,origen,repeticiones);
			j++;
		}

	}
	t_nodo nodo;
	nodo=nodoConPrioridad(IpRepe);
	if(dividirArchivo.rem != 0)
	{
		int z;
		for(z=0; z < dividirArchivo.rem; z++)
		{
			mandarMensaje(nodo,fd,i,origen,repeticiones);

		}

	}
return nodo;
}

int distribucionGrandeConResto(char* nombreDeUnArchivo, ldiv_t bloquesArchivo, long tamanioArch, int fd,t_IpViejas IpRepe, int repeticiones)
{
	/*variables*/
    t_nodo nodo;
   	nodo=nodoConPrioridad(IpRepe);
	int bloquesLibresNodo;
	int origen=4;

	bloquesLibresNodo = cantBloquesLibres(nodo.bitarray);
	if(bloquesLibresNodo >= bloquesArchivo.quot)
	{
		int i;
		for(i=0; i < bloquesArchivo.quot; i++)
		{
			if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
			return 0;
		}
			bloquesLibresNodo = cantBloquesLibres(nodo.bitarray);
			if(bloquesLibresNodo==1)
			{
				if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,repeticiones)==0)
					return 0;
			}else{
				nodo=nodoConPrioridad(IpRepe);
				if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,repeticiones)==0)
				return 0;
			}

	}else{
	  	int i=0;
	  	while(i < bloquesLibresNodo)
		{
	  		if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
	  		return 0;
	  		i++;
	  	}
	  	int bloquesSinMap;
		bloquesSinMap= bloquesArchivo.quot - bloquesLibresNodo;
	  	int j;
	  	for(j=0; j < bloquesArchivo.quot; j++)
	  		{
	  		if(mandarMensaje(nodo,fd,i,origen,repeticiones)==0)
	  		return 0;
	  		}

	  	bloquesLibresNodo = cantBloquesLibres(nodo.bitarray);

		if(bloquesLibresNodo==1)
		{
			if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,repeticiones)==0)
			return 0;

		}else{
	  		nodo=nodoConPrioridad(IpRepe);
	  		if(mandarMensajeConResto(nodo,fd,tamanioArch,bloquesArchivo.rem,origen,repeticiones)==0)
	  		return 0;
	  	}
	}

return 1;
}

int mandarMensajeConResto(t_nodo nodo, int fd, long tamanioArch, long resto, int origen, int repeticiones)
{
	char* mapeado;
	char* IP;
	t_estado_nodo* nodoDicc;
	char* aux;
	ldiv_t indice= ldiv(tamanioArch,TAMANIO_BLOQUE);

	mapeado = (char*) mmap(0,resto, PROT_READ, MAP_SHARED,fd,indice.quot*TAMANIO_BLOQUE);

	aux=(char*)malloc(TAMANIO_BLOQUE);
	memcpy(aux,mapeado,resto);
	memset(aux + resto  ,'\0', TAMANIO_BLOQUE - resto);

	t_header cabecera;
	int offset=0;
	int tamanioPaquete;
	int sockNodo;
	int bloqueAEscribir;
	void* mensaje=malloc(sizeof(t_header) +(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int));

	cabecera.tipoMensaje=SetBloque;
	cabecera.tamanioMensaje=(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int);
	packHeader(cabecera,mensaje);


	pthread_mutex_lock( &historialNodos );
	IP = inet_ntoa(nodo.Ip);
	nodoDicc = (t_estado_nodo*)dictionary_get(nodosHistorial,IP);
	pthread_mutex_unlock( &historialNodos );

	bloqueAEscribir= obtenerBloqueLibre(nodoDicc->estadoBloques);
	offset=offset + sizeof(t_header);
	memcpy(mensaje + offset,(int*)&bloqueAEscribir,sizeof(int));
	offset = offset + sizeof(int);
	memcpy(mensaje + offset, aux, TAMANIO_BLOQUE);

	bitarray_set_bit(nodoDicc->estadoBloques,bloqueAEscribir);

	sockNodo=socket(AF_INET,SOCK_STREAM,0);
	int flag=1;
	if(connect(sockNodo,(struct sockaddr *)&nodoDicc->datosNodo.connectionAddr,sizeof(struct sockaddr))==-1)
	 {
		LiberarEspacios();
		bitarray_clean_bit(nodoDicc->estadoBloques,bloqueAEscribir);
		logearDesconexionNodo(nodoDicc);
		desconectarNodo(nodoDicc);
		printf("Error al cargar el archivo");
		flag= 0;
	 }else{
		 if(repeticiones==1){
			 agregarCopia1ConResto(nodoDicc,bloqueAEscribir);
		 }else{
			 agregaCopia23ConResto(nodoDicc,repeticiones,bloqueAEscribir);
		 }
	 }

	verificarConexionNodo(nodoDicc);
	tamanioPaquete=sizeof(t_header)+(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int);
	sendall(sockNodo,mensaje,&tamanioPaquete);
	munmap(mapeado,mapeado = (char*)TAMANIO_BLOQUE);
	close(sockNodo);
	free(aux);
	free(mensaje);
	return flag;

}

int mandarMensaje(t_nodo nodo, int fd, int i, int origen, int repeticiones)
{
	char* mapeado;
	char* IP;
	t_estado_nodo* nodoDicc;
	int sockNodo;

	mapeado = (char*) mmap(0,TAMANIO_BLOQUE, PROT_READ, MAP_SHARED,fd,i*TAMANIO_BLOQUE);

	t_header cabecera;
	int offset=0;
	int tamanioPaquete;
	int bloqueAEscribir;
	void* mensaje=malloc(sizeof(t_header) +(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int));

	cabecera.tipoMensaje=SetBloque;
	cabecera.tamanioMensaje=(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int);
	packHeader(cabecera,mensaje);

	pthread_mutex_lock( &historialNodos );
	IP = inet_ntoa(nodo.Ip);
	nodoDicc = (t_estado_nodo*)dictionary_get(nodosHistorial,IP);
	pthread_mutex_unlock( &historialNodos );

	bloqueAEscribir= obtenerBloqueLibre(nodoDicc->estadoBloques);
	offset=offset + sizeof(t_header);
	memcpy(mensaje + offset,(int*)&bloqueAEscribir,sizeof(int));
	offset = offset + sizeof(int);
	memcpy(mensaje + offset, mapeado, TAMANIO_BLOQUE);

	bitarray_set_bit(nodoDicc->estadoBloques,bloqueAEscribir);

	sockNodo=socket(AF_INET,SOCK_STREAM,0);
	int flag=1;
	if(connect(sockNodo,(struct sockaddr *)&nodoDicc->datosNodo.connectionAddr,sizeof(struct sockaddr))==-1)
		 {//en caso de que falle conexion, libero diccionario
			LiberarEspacios();
			bitarray_clean_bit(nodoDicc->estadoBloques,bloqueAEscribir);
			logearDesconexionNodo(nodoDicc);
			desconectarNodo(nodoDicc);
			printf("Error al cargar el archivo");
			flag=0;

		 }else{//si no falla se debe llenar el diccionario
			 switch(origen){
			 case 1:	if(repeticiones==1)
			 	 	 	 {
				 	 	 	 agregarDatosCopiaGrande(nodoDicc,i,bloqueAEscribir,repeticiones);
				 	 	 	 break;
			 	 	 	 }else{
			 	 	 		 agregarDatosCopiaNormal(nodoDicc,i,bloqueAEscribir,repeticiones);
			 	 	 		 break;
			 	 	 	 }
			 case 2:	agregarDatosCopiaNormal(nodoDicc,i,bloqueAEscribir,repeticiones);
			 	 	 	break;
			 case 3:	agregarDatosCopiaGrande(nodoDicc,i,bloqueAEscribir,repeticiones);
			 	 	 	break;
			 case 4: 	if(repeticiones==1)
 	 	 	 	 	 	 {
	 	 	 	 	 	 	 agregarDatosCopiaGrande(nodoDicc,i,bloqueAEscribir,repeticiones);
	 	 	 	 	 	 	 break;
 	 	 	 	 	 	 }else{
 	 	 	 	 	 		 agregarDatosCopiaNormal(nodoDicc,i,bloqueAEscribir,repeticiones);
 	 	 	 	 	 		 break;
 	 	 	 	 	 	 }
			 default: break;
			 }

		 }
	verificarConexionNodo(nodoDicc);
	tamanioPaquete=sizeof(t_header)+(TAMANIO_BLOQUE*sizeof(char)) + sizeof(int);
	sendall(sockNodo,mensaje,&tamanioPaquete);
	munmap(mapeado,TAMANIO_BLOQUE);
	free(mensaje);
	close(sockNodo);
	return flag;
}

bool noSeRepite(char* Ip_Lista, t_IpViejas IpRepe){
	bool n;
	if((strcmp(Ip_Lista,IpRepe.Ip1)==0)||(strcmp(Ip_Lista,IpRepe.Ip2)==0)){
		n=false;
	}else{n=true;}
return n;
}

void actualizarIpVieja(const struct in_addr IP, t_IpViejas IpRepe){
	char* IP_Nueva;
	IP_Nueva=inet_ntoa(IP);
	switch(IpRepe.historial){
	case 0: strcpy(IpRepe.Ip1,IP_Nueva);
			IpRepe.historial=1;
			break;
	case 1:	strcpy(IpRepe.Ip2,IP_Nueva);
			IpRepe.historial=2;
			break;
	case 2: strcpy(IpRepe.Ip1,IP_Nueva);
			IpRepe.historial=1;
			break;
	default:break;
	}

}



