#include "headers/comandosEjecutablesSecundarios.h"

extern t_dictionary * nodosHistorial;
extern t_list* listaIp;

extern pthread_mutex_t historialNodos;
extern pthread_mutex_t historialIP;

//FUNCIONES SECUNDARIAS
char* obtenerContenidoDelBloque(int sockConsola)
{
	void* datosCabecera = malloc(sizeof(t_header));

	//Recibo solo la cabecera
	recv(sockConsola, datosCabecera, sizeof(t_header), 0);

		t_header header;
		//Desempaqueto la cabecera
		umpackHeader(&header, datosCabecera);

		free (datosCabecera);

	//Recibo el numBloque
	int datosNumBloque;
	recv(sockConsola, (int *)&datosNumBloque, sizeof(int), 0);

	//Recibo el contenido del bloque
	char* datosDelContenido = (char *) malloc(TAMANIO_BLOQUE);
	recv(sockConsola, datosDelContenido, TAMANIO_BLOQUE, 0);

	return datosDelContenido;
}


struct sockaddr_in inicializarDireccionConsola(int puerto, long int IP)
{
	struct sockaddr_in direccion;

	direccion.sin_family = AF_INET;
	direccion.sin_port = htons(puerto);
	direccion.sin_addr.s_addr = htonl(IP);
	memset(&(direccion.sin_zero),'\0', 8 );

	return direccion;
}


char * quitarBarraCerosSiFueraNecesario(char * contenido)
{
	char * contenidoNuevo;
	int cantidadDeCaracteres = strlen(contenido);
	int tamanioDelContenidoIngresado = (cantidadDeCaracteres + 1) * sizeof(char);

	if(tamanioDelContenidoIngresado < TAMANIO_BLOQUE)
	{
		int i = 0;
		cantidadDeCaracteres++;

		contenidoNuevo = (char*) malloc(sizeof(char)*cantidadDeCaracteres);

		while(contenido[i])
		{
			contenidoNuevo[i] = contenido[i];
			i++;
		}

		contenidoNuevo[i] = '\0';
		return contenidoNuevo;
	}
	return contenido;
}

// -------------------------- FUNCIONES PRINCIPALES -----------------------------------
void copiarAlFS(char* nombreDelArchivo, char* rutaDeAlmacenamiento)
{
	int bloque_Obtenido_Y_Almacenado = 0;
	int EstadoOK = 0;
	int numeroDeCopia;
	int numeroDeBloque;

	if(bloquesDisponibles(nombreDelArchivo))
	{
		t_dictionary * diccionarioDeBloques;
		t_dictionary * dicDeCopias;

		diccionarioDeBloques = getBloques_DB (nombreDelArchivo);
		int cantidadTotalDeBloques = dictionary_size(diccionarioDeBloques);

		//Ingresando a un bloque X
		for (numeroDeBloque = 0; numeroDeBloque < cantidadTotalDeBloques; numeroDeBloque++)
		{
			char * nBloque = string_itoa(numeroDeBloque);

			dicDeCopias = (t_dictionary *) dictionary_get( diccionarioDeBloques, nBloque );
			free(nBloque);

			int cantidadTotalDeCopias = dictionary_size(dicDeCopias);

			//Ingresando a una copia
				for (numeroDeCopia = 1; numeroDeCopia <= cantidadTotalDeCopias; numeroDeCopia++)
				{
					char * nCopia = string_itoa(numeroDeCopia);

					t_copiaX * unaCopia = (t_copiaX*)  dictionary_get( dicDeCopias, nCopia );
					free(nCopia);

					//OBTENGO LOS DATOS CONTENIDOS EN LA COPIA:
						long int ip_NODO =  (*unaCopia).IP;
						int puerto_NODO = (*unaCopia).puerto;
						int numBloque_NODO = (*unaCopia).numBloque;

					//Libero el struct de la copia;
					free(unaCopia);

				//AQUI mando un connect para ver si estan los nodos que contienen la copia X
					//----------------Voy armando el archivo--------------------------------------
						int sockConsola;//socket conexion
						sockConsola=socket(AF_INET,SOCK_STREAM,0);
						struct sockaddr_in nodo_addr;//nodo_addr es donde voy a guardar datos del Nodo

						nodo_addr = inicializarDireccionConsola(puerto_NODO,ip_NODO);

					//------------Conexion al NODO---------------------------------------------
						if (connect(sockConsola,(struct sockaddr *)&nodo_addr,sizeof(struct sockaddr))==-1)
						{
							close(sockConsola);
							continue;
						}

					//--------------------Mensaje a el NODO que contiene la copia-----------------
						void* mensaje = malloc(sizeof(t_header)+sizeof(int));
						t_header cabecera;
						cabecera.tipoMensaje=GetBloque; //Representa un 9 en socketsFS.h
						cabecera.tamanioMensaje=sizeof(int);
						packHeader(cabecera,mensaje);
						int offset=sizeof(t_header);

						memcpy(mensaje + offset, (int*)&numBloque_NODO, sizeof(int));

					//--------------------Envio el mensaje al NODO X-----------------
						sendall(sockConsola,mensaje,&cabecera.tamanioMensaje);

						//Elimino el mensaje enviado
						free(mensaje);
					//------------Espero y Obtengo la respuesta del NODO (el contenido)--------
						char * contenido = obtenerContenidoDelBloque(sockConsola);

						//Limpio el contenido si fuera necesario
						char * contenidoAlmacenable = quitarBarraCerosSiFueraNecesario(contenido);

						if(numeroDeBloque == 0){
							//Si es el primer bloque creo el archivo, y guardo el primer contenido
							createFile(TAMANIO_BLOQUE, rutaDeAlmacenamiento, contenidoAlmacenable);
						}else{
							//Le agrego mas contenido al archivo
							FILE* archivoConsola = txt_open_for_append(rutaDeAlmacenamiento);
								txt_write_in_file(archivoConsola, contenidoAlmacenable);
							txt_close_file(archivoConsola);
						}

						free(contenidoAlmacenable);
						bloque_Obtenido_Y_Almacenado = 1;
						close(sockConsola);
						break;
				}
				if (!bloque_Obtenido_Y_Almacenado)
				{
					EstadoOK = 0;
					break;
				}
				bloque_Obtenido_Y_Almacenado = 0;
		}
	} //Uso la red solo si puedo armar el archivo
	if(EstadoOK)
	{
		printf("El archivo: %s fue almacenado correctamente en: %s", nombreDelArchivo, rutaDeAlmacenamiento);
	}
	else
	{
		printf("NO se puede obtener el archivo solicitado");
	}
	return;
}



void obtenerMD5(char* nombreDelArchivo)
{
	int bloque_Obtenido_Y_Almacenado = 0;
	int EstadoOK = 0;
	int numeroDeBloque;
	int numeroDeCopia;

	if(bloquesDisponibles(nombreDelArchivo))
	{

		t_dictionary * diccionarioDeBloques;
		t_dictionary * dicDeCopias;

		diccionarioDeBloques = getBloques_DB (nombreDelArchivo);
		int cantidadTotalDeBloques = dictionary_size(diccionarioDeBloques);

		//Ingresando a un bloque X
		for (numeroDeBloque = 0; numeroDeBloque < cantidadTotalDeBloques; numeroDeBloque++)
		{
			char * nBloque = string_itoa(numeroDeBloque);

			dicDeCopias = (t_dictionary *) dictionary_get( diccionarioDeBloques, nBloque );
			free(nBloque);

			int cantidadTotalDeCopias = dictionary_size(dicDeCopias);

			//Ingresando a una copia
				for (numeroDeCopia = 1; numeroDeCopia <= cantidadTotalDeCopias; numeroDeCopia++)
				{
					char * nCopia = string_itoa(numeroDeCopia);

					t_copiaX * unaCopia = (t_copiaX*)  dictionary_get( dicDeCopias, nCopia );
					free(nCopia);

					//OBTENGO LOS DATOS CONTENIDOS EN LA COPIA:
						long int ip_NODO =  (*unaCopia).IP;
						int puerto_NODO = (*unaCopia).puerto;
						int numBloque_NODO = (*unaCopia).numBloque;

					//Libero cosas;
					free(unaCopia);

				//AQUI mando un connect para ver si estan los nodos que contienen la copia X
					//----------------Voy armando el archivo--------------------------------------
						int sockConsola;//socket conexion
						sockConsola=socket(AF_INET,SOCK_STREAM,0);
						struct sockaddr_in nodo_addr;//nodo_addr es donde voy a guardar datos

						nodo_addr = inicializarDireccionConsola(puerto_NODO,ip_NODO);

					//------------Conexion al NODO---------------------------------------------
						if (connect(sockConsola,(struct sockaddr *)&nodo_addr,sizeof(struct sockaddr))==-1)
						{
							close(sockConsola);
							continue;
						}

					//--------------------Mensaje a el NODO que contiene la copia-----------------
						void* mensaje = malloc(sizeof(t_header)+sizeof(int));
						t_header cabecera;
						cabecera.tipoMensaje=GetBloque;
						cabecera.tamanioMensaje=sizeof(int);
						packHeader(cabecera,mensaje);
						int offset=sizeof(t_header);

						memcpy(mensaje + offset, (int*)&numBloque_NODO, sizeof(int));

					//--------------------Envio el mensaje al NODO X-----------------
						sendall(sockConsola,mensaje,&cabecera.tamanioMensaje);

						//Elimino el mensaje enviado
						free(mensaje);

					//------------Espero y Obtengo la respuesta del NODO (el contenido)--------
						char * contenido = obtenerContenidoDelBloque(sockConsola);

						//Limpio el contenido si fuera necesario
						char * contenidoAlmacenable = quitarBarraCerosSiFueraNecesario(contenido);

						if(numeroDeBloque == 0){
							//Si es el primer bloque creo el archivo, y guardo el primero contenido
							createFile(TAMANIO_BLOQUE, "/tmp/archConsola.txt", contenidoAlmacenable);
						}else{
							//Le agrego mas contenido al archivo
							FILE* archivoConsola = txt_open_for_append("/tmp/archConsola.txt");
								txt_write_in_file(archivoConsola, contenidoAlmacenable);
							txt_close_file(archivoConsola);
						}

						free(contenidoAlmacenable);
						bloque_Obtenido_Y_Almacenado = 1;
						close(sockConsola);
						break;
				}

				if (!bloque_Obtenido_Y_Almacenado)
				{
					EstadoOK = 0;
					break;
				}
				bloque_Obtenido_Y_Almacenado = 0;
		}
	} //Uso la red solo si puedo armar el archivo

	if(EstadoOK){
		char md5[32 + 1];

    	GetMD5("/tmp/archConsola.txt", md5);
		printf("El MD5 del archivo: %s es: %s", nombreDelArchivo, md5);
	}else{
		printf("NO se puede obtener el archivo solicitado");
	}

	remove("/tmp/archConsola.txt");

	return;
}



int bloquesDisponibles(char * nombreDelArchivo)
//Tira connects para ver si se puede acceder a al menos 1 copia de cada bloque.
{
	int copiaAccesible = 0;
	int EstadoOK = 1;
	int numeroDeBloque;
	int numeroDeCopia;

	t_dictionary * diccionarioDeBloques;
	t_dictionary * dicDeCopias;

	diccionarioDeBloques = getBloques_DB (nombreDelArchivo);
	int cantidadTotalDeBloques = dictionary_size (diccionarioDeBloques);

	//Ingresando a un bloque X
	for (numeroDeBloque = 0; numeroDeBloque < cantidadTotalDeBloques; numeroDeBloque++)
	{
		char * nBloque = string_itoa(numeroDeBloque);

		dicDeCopias = (t_dictionary *) dictionary_get( diccionarioDeBloques, nBloque );
		free(nBloque);

		int cantidadTotalDeCopias = dictionary_size(dicDeCopias);

		//Ingresando a una copia
			for (numeroDeCopia = 1; numeroDeCopia <= cantidadTotalDeCopias; numeroDeCopia++)
			{
				char * nCopia = string_itoa(numeroDeCopia);

				t_copiaX * unaCopia = (t_copiaX*) dictionary_get( dicDeCopias, nCopia );
				free(nCopia);

				//OBTENGO LOS DATOS CONTENIDOS EN LA COPIA:
					long int ip_NODO =  (*unaCopia).IP;
					int puerto_NODO = (*unaCopia).puerto;

				//Libero el struct de la copia;
				free(unaCopia);

			//AQUI mando un connect para ver si estan los nodos que contienen la copia X
				//----------------Voy armando el archivo--------------------------------------
					int sockNodoX;//socket conexion
					sockNodoX=socket(AF_INET,SOCK_STREAM,0);
					struct sockaddr_in nodo_addr;//nodo_addr es donde voy a guardar datos del Nodo

					nodo_addr = inicializarDireccionConsola(puerto_NODO,ip_NODO);

				//------------Conexion al NODO---------------------------------------------
					if (connect(sockNodoX,(struct sockaddr *)&nodo_addr,sizeof(struct sockaddr))==-1)
					{
							struct in_addr Ip;
							Ip.s_addr= ip_NODO;

							char* aux = inet_ntoa(Ip);
							char* ipChar = malloc(sizeof(char)* strlen(aux));
							memcpy(ipChar, aux, (sizeof(char)* strlen(aux)));

							t_estado_nodo* nodoDicc;
							pthread_mutex_lock( &historialNodos );
								nodoDicc=(t_estado_nodo*)dictionary_get(nodosHistorial,ipChar);
							pthread_mutex_unlock( &historialNodos );


							free(aux); free(ipChar);

							desconectarNodo(nodoDicc);

						close(sockNodoX);
						continue;
					}
					copiaAccesible = 1;
					close(sockNodoX);
					break;
			}
			if (!copiaAccesible)
				{
					EstadoOK = 0;
					break;
				}
			copiaAccesible = 0;
	}

	if(EstadoOK)
	{
		return EstadoOK;
	}
	else
	{
		return EstadoOK;
	}
}


bool noSeRepiteV2(char* Ip_Lista, char ** ipHistorial, int cantidadTotalDeCopias)
{
	int i;

	for (i = 1; i <= cantidadTotalDeCopias; i++)
	{
		if(strcmp(Ip_Lista,ipHistorial[i])==0)
		{
			int j;
			for(j=0; j<=cantidadTotalDeCopias; j++)
			{
				free(ipHistorial[j]);
			}
			free(ipHistorial);
			return false;
		}
	}
	  int j;
	  for(j=0; j<=cantidadTotalDeCopias; j++)
	  {
		free(ipHistorial[j]);
	  }
	  free(ipHistorial);

	return true;
}


t_nodo nodoConPrioridadV2(char ** ipHistorial, int cantidadTotalDeCopias)
{
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
			if((bloquesLibres > maxBloques) && (noSeRepiteV2(IP_Lista, ipHistorial, cantidadTotalDeCopias)))
			{
				maxBloques = bloquesLibres;
				nodo.Ip=nodoDicc->datosNodo.connectionAddr.sin_addr;
				nodo.bitarray=nodoDicc->estadoBloques;
			}
		}
	}
	return nodo;
}
