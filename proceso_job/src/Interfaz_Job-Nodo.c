#include "Comunes_Job.h"
#include "Interfaz_Job-Nodo.h"
#include "ManejoMensajesJob.h"

void empaquetarPedidoDeMap(void* mensaje, t_header cabecera, char* archivoMap, long tamanioScript, char* scriptMap, bloque_t* bloque)
{
	packHeader(cabecera, mensaje);

	long desplazamiento = sizeof(t_header);
	int largoNombreArchivo = strlen(archivoMap) + 1;

	//Empaqueto el largo del nombre del archivo de map
	memcpy(mensaje + desplazamiento, (int*) &largoNombreArchivo, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	//Empaqueto el nombre del archivo de map
	memcpy(mensaje + desplazamiento, archivoMap, sizeof(char) * largoNombreArchivo);
	desplazamiento = desplazamiento + sizeof(char) * largoNombreArchivo;

	//Empaqueto el largo del contenido del archivo de map
	memcpy(mensaje + desplazamiento, (long*) &tamanioScript, sizeof(long));
	desplazamiento = desplazamiento + sizeof(long);

	//Empaqueto el contenido del archivo de map
	memcpy(mensaje + desplazamiento, scriptMap, sizeof(char) * tamanioScript);
	desplazamiento = desplazamiento + sizeof(char) * tamanioScript;

	largoNombreArchivo = strlen(bloque->archivoResultado) + 1;

	//Empaqueto el largo del nombre del archivo resultado de map
	memcpy(mensaje + desplazamiento, (int*) &largoNombreArchivo, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	//Empaqueto el nombre del archivo resultado de map
	memcpy(mensaje + desplazamiento, bloque->archivoResultado, sizeof(char) * largoNombreArchivo);
	desplazamiento = desplazamiento + sizeof(char) * largoNombreArchivo;

	//Empaqueto el numero de bloque a ser mapeado
	memcpy(mensaje + desplazamiento, (int*) &bloque->numeroBloque, sizeof(int));

	return;
}


void enviarSolicitudDeMap(nodo_t nodo, char* archivoMap)
{
	int cantidadBloques = list_size(nodo.bloquesAOperar);
	char* scriptMap = NULL;
	long tamanioScript = tamanioArchivo(archivoMap);

	bloque_t* bloque;

	t_header cabecera;

	//Defino el tipo de mensaje como solicitud de map
	cabecera.tipoMensaje = SolicitudDeMap;

	//Obtengo el contenido del archivo script de map y lo almaceno en la variable scriptMap
	getFileContent(archivoMap, scriptMap);

	void* mensaje;

	//Por cada bloque del nodo que va a ser mapeado envio una solicitud de map
	int i;
	for(i = 0; i < cantidadBloques; i++)
	{
		//obtendo el bloque
		bloque = list_get(nodo.bloquesAOperar, i);

		if(bloque->estado == SinMapear)
		{
			//Calculo el tamaño del mensaje para ese bloque
			cabecera.tamanioMensaje =	(sizeof(t_header) +
										sizeof(int) + strlen(archivoMap)+1 +
										sizeof(long) + (tamanioScript * sizeof(char)) +
										sizeof(int) + strlen(bloque->archivoResultado)+1 +
										sizeof(int));

			//reservo memoria para copiar el mensaje
			mensaje = malloc (cabecera.tamanioMensaje);

			//empaqueto el mensaje que sera enviado al nodo
			empaquetarPedidoDeMap(mensaje, cabecera, archivoMap, tamanioScript, scriptMap, bloque);

			//envio el mensaje
			sendAll(bloque->socket, mensaje, cabecera.tamanioMensaje);

			bloque->estado = Mapeado;
			list_replace(nodo.bloquesAOperar, i, bloque);

			free(mensaje);
		}
	}

	return;
}

int obtenerCantidadDeArchivosAReducir(t_list* listaDeNodos)
{
	int cantidadArchivos = 0;

	int cantidadNodos = list_size(listaDeNodos);

	nodo_t* nodo;


	int i;
	for(i = 0; i < cantidadNodos; i++)
	{
		nodo = list_get(listaDeNodos, i);

		cantidadArchivos = cantidadArchivos + list_size(nodo->bloquesAOperar);
	}

	return cantidadArchivos;
}


int enviarSolicitudDeReduce(t_list* listaDeNodos, combiner_t combiner, struct in_addr ipNodoReducer, int puertoNodoReducer, char* archivoReduce, char* archivoResultado)
{
	//Obtengo el contenido del archivo reduce
	char* scriptReduce = NULL;
	long tamanioScript = tamanioArchivo(archivoReduce);
	getFileContent(archivoReduce, scriptReduce);

	//Obtengo la cantidad de archivos que se van a reducir
	int cantidadArchivosAReducir = obtenerCantidadDeArchivosAReducir(listaDeNodos);

	//Defino la cabecera
	t_header cabecera;
	if(combiner == ConCombiner)
		cabecera.tipoMensaje = SolicitudDeReduceConCombiner;
	else
		cabecera.tipoMensaje = SolicitudDeReduceSinCombiner;

	//Calculo el tamaño que tendra la primer parte del mensaje cuya longitud conozco, luego de iran haciendo reallocs para aumentarlo segun cada archivo a reducir
	cabecera.tamanioMensaje = sizeof(t_header) + sizeof(int) + strlen(archivoReduce) + 1 + sizeof(long) + (tamanioScript * sizeof(char)) + sizeof(int) + strlen(archivoResultado) + 1 + sizeof(int);

	void* mensaje = malloc (cabecera.tamanioMensaje);

	//Empaqueto los datos del reduce de los cuales ya se su tamaño
	empaquetarDatosFijosDelReduce(mensaje, archivoReduce, scriptReduce, tamanioScript, archivoResultado, cantidadArchivosAReducir);


	long desplazamiento = cabecera.tamanioMensaje;

	int cantidadNodos = list_size(listaDeNodos);
	int cantidadDeBloques;

	nodo_t* nodo;
	bloque_t* bloque;

	int i, j;


	//Por cada bloque que se mapeo le agrego al mensaje la ip y puerto del nodo en la que esta mas el archivo que se va a reducir
	for(i = 0; i < cantidadNodos; i++)
	{
		nodo = list_get(listaDeNodos, i);

		cantidadDeBloques = list_size(nodo->bloquesAOperar);

		for(j = 0; j < cantidadDeBloques; j++)
		{
			bloque = list_get(nodo->bloquesAOperar, j);

			cabecera.tamanioMensaje = cabecera.tamanioMensaje + sizeof(struct in_addr) + sizeof(int) + sizeof(int) + strlen(bloque->archivoResultado) + 1;

			//aumento el tamaño de la memoria reservada para el mensaje para que pueda contener al nuevo bloque
			mensaje = realloc(mensaje, cabecera.tamanioMensaje);

			desplazamiento = desplazamiento + empaquetarBloqueAReducir(mensaje, nodo->ip, nodo->puerto, bloque->archivoResultado, desplazamiento);

		}

	}

	//Empaqueto la cabecera una ves que ya se cual es el temaño total del mensaje, aunque se empaquete ultimo sigue estando al principio del mensaje
	packHeader(cabecera , mensaje);

	//Defino el struct con la direccio del nodo que va a ejecutar el reduce
	struct sockaddr_in direccionNodo;

	direccionNodo.sin_addr = ipNodoReducer;
	direccionNodo.sin_family = AF_INET;
	direccionNodo.sin_port = puertoNodoReducer;
	memset(&direccionNodo.sin_zero, '\0', 8);

	//Creo el socket y lo conecto con el nodo
	 int sock = socket(AF_INET, SOCK_STREAM, 0);

	 if(connect(sock, (struct sockaddr*) &direccionNodo, sizeof(struct sockaddr_in)) == -1)
	 {
		 perror("connect nodo reducer");
		 exit(1);
	 }

	//envio el mensaje
	sendAll(sock, mensaje, cabecera.tamanioMensaje);

	free(mensaje);


	return sock;
}
