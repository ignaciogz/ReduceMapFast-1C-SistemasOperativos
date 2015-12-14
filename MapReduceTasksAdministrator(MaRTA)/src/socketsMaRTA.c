#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include "socketsMaRTA.h"
#include <commons/string.h>
#include <pthread.h>

// extern pthread_mutex_t mutex; agregar si hacen falta

void vincularSocket(int* socketNuevo, struct sockaddr_in direccionJob)
{
	*socketNuevo = socket(AF_INET, SOCK_STREAM, 0);
	if(*socketNuevo == -1)
	{
		perror("socket");
		exit(1);
	}

	if(connect(*socketNuevo,(struct sockaddr*)&direccionJob,sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		exit(1);
	}

}
struct sockaddr_in generarDireccion(char*ip,int puerto)
{
	struct sockaddr_in direccion;
	direccion.sin_family= AF_INET;
	inet_aton(ip,&direccion.sin_addr);
	direccion.sin_port = htons(puerto);
	memset(&(direccion.sin_zero), '\0', 8);
	return direccion;
}
void empaquetarCabecera(t_cabecera header, void* message){
	int offset;
	memcpy(message, (t_mensaje*)&header.mensaje, sizeof(t_mensaje));
	offset = sizeof(t_mensaje);
	memcpy(message + offset, (int*)&header.tamanio, sizeof(int));
	return;
}

void desempaquetarCabecera(t_cabecera* header, void* package){
	memcpy((t_mensaje*)&header->mensaje, package, sizeof(t_mensaje));
	memcpy((int*) &header->tamanio, package, sizeof(int));
	return;
}

void enviarTodo(int socket, void* message, int length){
	int offset = 0;
	int sent = 0;
    do{
	sent = send(socket, message + offset, length, 0);
	offset = offset + sent;
	length = length - sent;
	}while(length != 0 );
	return;
}

int buscarIndiceJob(t_job* job, t_list* lista_jobs)
{
	 int indice = 0;
	 bool encontrado = false;
	 t_job* jobBuscado;

	 while(indice < list_size(lista_jobs) && !encontrado)
	 {
		jobBuscado  = list_get(lista_jobs, indice);

		if(jobBuscado == job)
		{
			encontrado = true;
			return indice;
		}

		indice++;
	 }

	 return -1;

}



t_job* desempaquetarJob(t_conexion conexion, t_list* lista_jobs)
{

	int socket = conexion.socket;
	struct sockaddr_in direccion = conexion.direccion;
	int cant_archivos = 0,tamanio_arch = 0;
	t_job* job_a_agregar= malloc(sizeof(t_job));
	job_a_agregar->pend_reduce = 0;
	job_a_agregar->pend_mapping = 0;
	job_a_agregar->archivos = list_create();
	recv (socket,(int*) &cant_archivos , sizeof(int),0);
	int i =1;
	while (i <= cant_archivos)
	{
		if ( i < cant_archivos)
		{ // si no es archivo de resultado, cargo los archivos a la lista de archivos del job
		cargarArchivo(conexion,job_a_agregar,tamanio_arch);
		 i++;
		}
		else
		{ //si es archivo de resultado, lo guardo en el campo del job
		recv(socket, (int*)&tamanio_arch,sizeof(int),0);
		job_a_agregar->archivo_resultado = malloc(tamanio_arch);
		recv(socket,(char*)job_a_agregar->archivo_resultado,tamanio_arch,0 );
		i++;
		}
	}
 recv(socket,(int*)&job_a_agregar->soporte_combiner,sizeof(int),0);
 job_a_agregar->direccion = direccion;
 job_a_agregar->sock = conexion.socket;
 list_add (lista_jobs,job_a_agregar );

 return job_a_agregar;
}

void cargarArchivo(t_conexion conexion,t_job* job,int tamanio_arch)
{
	t_archivo* archivoNuevo=malloc(sizeof(t_archivo));
    recv(conexion.socket,(int*)&tamanio_arch,sizeof(int),0);
	char*archivo = malloc(tamanio_arch);
	recv(conexion.socket,archivo, tamanio_arch,0);
	archivoNuevo->archivo = archivo;
	archivoNuevo->nodos = list_create();
	archivoNuevo->bloques = list_create();
	list_add(job->archivos,archivoNuevo);
}
void crearPaqueteSolicitud(void* mensaje, t_job job, int longitudMensaje,int desplazamiento,char*nombreArchivo,int long_archivo, int indice)
{
    memcpy( mensaje + desplazamiento , (int*)&indice , sizeof(int) ); // tamanio arch
    desplazamiento = desplazamiento + sizeof(int) ;
    memcpy( mensaje + desplazamiento ,(int*) &long_archivo , sizeof(int) ); // tamanio arch
    desplazamiento = desplazamiento + sizeof(int) ;
    memcpy( mensaje + desplazamiento,(char*)nombreArchivo, long_archivo ); //nombre arch
}
void atenderJob(t_conexion conexion,t_list* listaJobs,struct sockaddr_in direccionFS,int socket)
{

	t_job* jobBuscado = desempaquetarJob(conexion,listaJobs);
	int indice = buscarIndiceJob(jobBuscado, listaJobs);
	t_cabecera cabecera;
	int cantArchivos = list_size(jobBuscado->archivos);
	int i = 0;
	void* mensaje;
	while(i < cantArchivos) //armo y mando el paquete al FS por cada archivo de la lista de archivos del job
		{
		t_archivo* archivo = list_get(jobBuscado->archivos,i);
	int long_archivo = string_length(archivo->archivo)+1;
	cabecera.mensaje =	pedidoBloques;
	                //      ip job                puerto job  long nombre   nombre archivo
	cabecera.tamanio =  sizeof(int)+ sizeof(int)+ long_archivo;
	int longitudMensaje = cabecera.tamanio + sizeof(t_cabecera);
	mensaje = malloc(longitudMensaje);
	empaquetarCabecera(cabecera,mensaje);
	int desplazamiento = sizeof(t_cabecera);
	crearPaqueteSolicitud(mensaje,*jobBuscado,longitudMensaje,desplazamiento,archivo->archivo,long_archivo, indice);
	vincularSocket(&socket,direccionFS);
	enviarTodo(socket, mensaje,longitudMensaje);
	//free(mensaje);
	i++;
	close(socket);
	}
}


void cargarNodo(t_conexion conexion,t_list* listaNodos)
{

  t_nodo* nodoNuevo;
  int cantidad=0;
  recv(conexion.socket,(int*)&cantidad,sizeof(int),0);
  int i=0;
  while(i < cantidad)
  {
  nodoNuevo = malloc(sizeof(t_nodo));
  nodoNuevo->tareas_en_ejecucion = list_create();
  recv(conexion.socket,(struct in_addr*)&nodoNuevo->ip , sizeof(struct in_addr),0);
  recv(conexion.socket,(int*)&nodoNuevo->puerto,sizeof(int),0);
  nodoNuevo->cant_rutinas_mapper = 0;
  nodoNuevo->cant_rutinas_reduce = 0;
  nodoNuevo->estado = Conectado;
  list_add(listaNodos,nodoNuevo);
  i++;
  }
}


void cargarBloque(t_conexion conexion,t_list*listaJobs,t_archivo* archivoBuscado,t_job*jobBuscado)
{   //inicializo las variables
	struct in_addr ipJob;
	int socket = conexion.socket;
	char* nombreArchivo;
	int cantidad_bloques=0,i,j,tamanioNombre=0,cantCopias=0,puertoJob=0, indice;
	// recivo la ip y puerto del job para buscar el job en la lista de jobs. Despues el nombre de arcivo, para buscar el archivo en la lista de archivos del job
	//recv(socket,(struct in_addr*)&ipJob, sizeof(struct in_addr),0);
	//recv(socket,(int*)&puertoJob,sizeof(int),0);
	recv(socket,(int*)&indice,sizeof(int),0);
	jobBuscado = list_get(listaJobs, indice);
	recv(socket,(int*)&tamanioNombre,sizeof(int),0);
	nombreArchivo = malloc(tamanioNombre);
	recv(socket,nombreArchivo,tamanioNombre,0);
	archivoBuscado = buscarArchivo(nombreArchivo,jobBuscado);
	// recivo la cantidad de bloques del archivo
	recv(socket,(int*)&cantidad_bloques,sizeof(int),0);
	char *bitarray= string_new();
	archivoBuscado->bitarray=bitarray_create(bitarray,cantidad_bloques);
	for(i=0;i<cantidad_bloques;i++)
	{ //por cada bloque de archivo recibido: inicializo el dato de tipo t_bloque que va a ir a la lista de bloques(manejo con combiner) y el dato de tipo t_nodosDelArchivo que ira a la lista de nodos(manejo sin combiner)
	  t_bloque* bloqueNuevo=inicializarBloque(jobBuscado,i,archivoBuscado);
	  //recivo la cantidad de copias
	  recv(socket,(int*)&cantCopias,sizeof(int),0);
	  for(j=0;j<cantCopias;j++)
            {
		     t_copia* copiaNueva = malloc(sizeof(t_copia));
		     t_copiasPorNodo* copiaNodo = crearCopiaPorNodo(i,j,archivoBuscado->archivo);
		     recv(socket,(struct in_addr*)&copiaNueva->ipNodo, sizeof(struct in_addr),0);
	         recv(socket,(int*)&copiaNueva->puerto_nodo,sizeof(int),0);
        	 recv(socket,(int*)&copiaNueva->bloqueInterno,sizeof(int),0);
        	 // si el nodo no  esta en la lista de nodos del archivo lo agrego, sino solo agrego el numero de copia a la lista de copias de ese nodo
        	 insertarNodoSolicitadoOCopia(copiaNueva->ipNodo,copiaNueva->puerto_nodo,copiaNodo,archivoBuscado->nodos);
        	 list_add(bloqueNuevo->copias,copiaNueva); // agregola  copia a la lista  de copias del bloque
        	 }
	list_add(archivoBuscado->bloques,bloqueNuevo); // ya tengo el bloque inicializado, lo agrego a la lista de bloques
	}
}

void atenderBloque(t_conexion conexion,t_list*listaJobs,t_list* listaNodos,int socket)
{   //inicializo las variables
	t_archivo* archivo = malloc(sizeof(t_archivo));
	t_job* job= malloc(sizeof(t_job));
	cargarBloque(conexion,listaJobs,archivo,job);
	vincularSocket(&socket,job->direccion);
	t_cabecera cabecera;
	void* mensaje;
	int longitudTotal;
	int desplazamiento;
	int cantidadCopiasMapeantes = cantidadDeCopiasQueMapean(job->soporte_combiner,*archivo);
	int longArchivosResultadoMap = longArchivosResultadoMapping(job,archivo);
	gestionarCombiner(job->soporte_combiner,archivo,listaNodos);
	cabecera.mensaje = pedidoMapping;
	cabecera.tamanio = sizeof(int) + longArchivosResultadoMap;
	if(job->soporte_combiner == si) // solo queda agregar el tamaño de las copias
	{
		cabecera.tamanio = cabecera.tamanio + cantidadCopiasMapeantes * sizeof(t_copia);
    }
	else
   {
		cabecera.tamanio = cabecera.tamanio + cantidadCopiasMapeantes * sizeof(t_copiasPorNodo);
   }
   longitudTotal = cabecera.tamanio + sizeof(t_cabecera);
   mensaje = malloc(longitudTotal);
   empaquetarCabecera(cabecera,mensaje);
   desplazamiento = sizeof(t_cabecera);
   crearPaqueteMapping(job->soporte_combiner,archivo,mensaje,longitudTotal,desplazamiento,listaNodos,cantidadCopiasMapeantes);
   enviarTodo(socket,mensaje,longitudTotal);
   close(socket);
   free(mensaje);
}

void crearPaqueteMapping(t_combiner combiner,t_archivo* archivo,void* mensaje,int longitudTotal,int desplazamiento,t_list* listaNodos,int cantCopias)
{
	if (combiner == si)
	{
		crearPaqueteMappingConCombiner(archivo,mensaje,longitudTotal,desplazamiento,cantCopias,listaNodos);
	}
	else
	{
		crearPaqueteMappingSinCombiner(archivo,mensaje,longitudTotal,desplazamiento,cantCopias,listaNodos);
	}
}
void crearPaqueteMappingConCombiner(t_archivo* archivo,void* mensaje,int longitudTotal,int desplazamiento,int cantCopias,t_list* listaNodos)
{//cantCopias + { ip + puerto + bloque interno +longArchResMap + archResultadoMap}
    int i = 0;
	memcpy(mensaje + desplazamiento,(int*)&cantCopias,sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);
	while(i < cantCopias) // recordar que  en este caso cantCopias = list_size(archivo->bloques)
	{
		t_bloque* bloque =(t_bloque*) list_get(archivo->bloques,i);
		// Busco el nodo de la lista nodos del sistema que se corresponde con la copia candidata del bloque obtenido. A este nodo se le sumará 1 en la cantidad de tareas mapper y se agregara a su lista de tareas la leyenda "mapping"
		t_nodo* nodo = buscarNodo(listaNodos,bloque->copiaCandidata.ipNodo,bloque->copiaCandidata.puerto_nodo);
		nodo->cant_rutinas_mapper = nodo->cant_rutinas_mapper +1;
		char * tarea = "mapping";
		list_add(nodo->tareas_en_ejecucion,tarea);
		// una vez hecho esto ya puedo armar el paquete :
		memcpy( mensaje + desplazamiento , (struct in_addr*)&bloque->copiaCandidata.ipNodo, sizeof(struct in_addr));
		desplazamiento = desplazamiento + sizeof(struct in_addr);
		memcpy(mensaje + desplazamiento,(int*)&bloque->copiaCandidata.puerto_nodo,sizeof(int));
		desplazamiento = desplazamiento + sizeof(int);
		memcpy( mensaje + desplazamiento , (int*)&bloque->copiaCandidata.bloqueInterno, sizeof(int));
	    desplazamiento = desplazamiento + sizeof(int);
	    int longArch = string_length(bloque->archivoResultadoMap)+1;
	    memcpy( mensaje + desplazamiento , (int*)&longArch, sizeof(int));
	    desplazamiento = desplazamiento + sizeof(int);
	    memcpy( mensaje + desplazamiento ,(char*)&bloque->archivoResultadoMap,longArch);
	    desplazamiento = desplazamiento + longArch;
	    i++;
	}
}

void crearPaqueteMappingSinCombiner(t_archivo*archivo,void*mensaje,int longitudTotal,int desplazamiento,int cantCopias,t_list* listaNodos)
{//cantCopias + { ip + puerto + bloque interno +longArchResMap + archResultadoMap}
	int cantNodos = list_size(archivo->nodosConCopiasAMapear);
	int i = 0,j,cantCopiasNodo;
	memcpy(mensaje + desplazamiento,(int*)&cantCopias,sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);
	while (cantNodos < i)
	{
	   j = 0;
	   t_nodosDelArchivo * nodo = list_get(archivo->nodosConCopiasAMapear,i);
	   // Busco nodo en la lista de nodos el nodo con misma ip y puerto. Sumo 1 a la cant de tareas mapping y agrego la leyenda "mapping" en las tareas que esta realizando
       t_nodo* nodoLista = buscarNodo(listaNodos,nodo->ipNodo,nodo->puerto_nodo);
       char * tarea = "mapping";
       list_add(nodoLista->tareas_en_ejecucion,tarea);
       nodoLista->cant_rutinas_mapper = nodoLista->cant_rutinas_mapper +1;
       cantCopiasNodo = list_size(nodo->copias);
       while(j < cantCopiasNodo)
       { // por cada copia agrego los siguientes datos (ip y puerto son las del nodoDelArchivo):
    	  t_copiasPorNodo * copia = list_get(nodo->copias,j);
    	  memcpy( mensaje + desplazamiento , (struct in_addr*)&nodo->ipNodo, sizeof(struct in_addr));
    	  desplazamiento = desplazamiento + sizeof(struct in_addr);
    	  memcpy(mensaje + desplazamiento,(int*)&nodo->puerto_nodo,sizeof(int));
    	  desplazamiento = desplazamiento + sizeof(int);
    	  memcpy(mensaje + desplazamiento,(int*)&copia->bloqueInterno,sizeof(int));
    	  desplazamiento = desplazamiento + sizeof(int);
    	  int longArch = string_length(copia->archivoResultadoMap)+1;
    	  memcpy(mensaje + desplazamiento,(int*)&longArch,sizeof(int));
    	  desplazamiento = desplazamiento + sizeof(int);
    	  memcpy( mensaje + desplazamiento ,(char*)copia->archivoResultadoMap,longArch);
    	  desplazamiento = desplazamiento + longArch;
    	  j++;
       }
       i++;
    }
}


void mandarPedidoReduce(t_copia copia,t_conexion conexion)
{
		int desplazamiento;
		t_cabecera cabecera;
		int longitudMensaje;
		cabecera.mensaje = pedidoReduce;
		cabecera.tamanio = sizeof(struct in_addr) + sizeof(int);
		longitudMensaje = cabecera.tamanio+ sizeof(t_cabecera);
		void * mensaje = malloc(longitudMensaje);
		empaquetarCabecera(cabecera,mensaje);
		desplazamiento= sizeof(t_cabecera);
	    memcpy( mensaje + desplazamiento,(struct in_addr*)&copia.ipNodo,sizeof(struct in_addr) );
		desplazamiento = desplazamiento + sizeof(struct in_addr);
		memcpy( mensaje + desplazamiento,(int*)&copia.puerto_nodo,sizeof(int));
		desplazamiento = desplazamiento + sizeof(int);
		enviarTodo(conexion.socket,mensaje,longitudMensaje);
		free(mensaje);
}




void gestionarResultadoMap(t_conexion conexionJob,t_list*listaJobs,t_list* listaNodos)
{
	struct in_addr ipNodo;
	int puertoNodo=0;
	t_copia candidatoParaReducir;
	recv(conexionJob.socket,(struct in_addr*)&ipNodo, sizeof(int),0);
	recv(conexionJob.socket,(int*)&puertoNodo, sizeof(int),0);
	t_job* jobBuscado = buscarJob(listaJobs,conexionJob.direccion.sin_addr, conexionJob.direccion.sin_port);
		if (jobBuscado->pend_mapping >0)
		{ //si todavia no terminaron de mappear todos los archivos, resto 1 a los pendientes de mapping del job, tambien debo eliminar una tarea "mapping" de las tareas en ejecucion del nodo correspondiente
		jobBuscado->pend_mapping =jobBuscado->pend_mapping -1 ;
	    t_nodo * nodo = buscarNodo(listaNodos,ipNodo,puertoNodo);
	    nodo->cant_rutinas_mapper = nodo->cant_rutinas_mapper -1;
	    eliminarTarea(nodo->tareas_en_ejecucion,"mapping");
		}
		else
		{ // si ya terminaron de mappear todas las copias del archivo, hay que mandar a reducir
		candidatoParaReducir = seleccionarNodoReductor(jobBuscado);
		mandarPedidoReduce(candidatoParaReducir,conexionJob);
		}
}


void gestionarReduce(t_conexion conexion,t_list* listaJobs,t_list* listaNodos,struct sockaddr_in direccionFs)
{
	struct in_addr ipNodo;
	int puertoNodo=0;
	recv(conexion.socket,(struct in_addr*)&ipNodo,sizeof(struct in_addr),0);
	recv(conexion.socket,(int*)&puertoNodo,sizeof(int),0);
	t_job* jobBuscado = buscarJob(listaJobs,conexion.direccion.sin_addr, conexion.direccion.sin_port);
	if(jobBuscado->pend_reduce > 0)
	{ // si todavia no terminaron de reducir todos los archivos del job, resto 1 a los pendientes de reduce y elimino una tarea "reduce" de las tareas en ejecucion del nodo correspondiente
		jobBuscado->pend_reduce = jobBuscado->pend_reduce -1;
		t_nodo* nodo = buscarNodo(listaNodos,ipNodo,puertoNodo);
		nodo->cant_rutinas_reduce = nodo->cant_rutinas_reduce -1;
		eliminarTarea(nodo->tareas_en_ejecucion,"reduce");
	}
	else
	{ // esta parte esta bien
	mandarACopiarArchivoResultado(jobBuscado->archivo_resultado,ipNodo,puertoNodo,direccionFs);
	enviarFinDeOperacion(conexion);
	eliminarJob(listaJobs,conexion.direccion.sin_addr,conexion.direccion.sin_port);
	}
}

void mandarACopiarArchivoResultado(char* archivo_resultado,struct in_addr ipNodo,int puertoNodo,struct sockaddr_in direccionFs)
{
	t_cabecera cabecera;
	int desplazamiento,longitudMensaje,long_archivo,socketFs;
	cabecera.mensaje = copiarArchivoDeResultado;
	long_archivo=string_length(archivo_resultado)+1;
	cabecera.tamanio = sizeof(struct in_addr) + sizeof(int) + sizeof(int)+ long_archivo;
	longitudMensaje = cabecera.tamanio+ sizeof(t_cabecera);
	void * mensaje = malloc(longitudMensaje);
	empaquetarCabecera(cabecera,mensaje);
	desplazamiento= sizeof(t_cabecera);
	memcpy( mensaje + desplazamiento , (int*) &long_archivo , sizeof(int) ); // tamanio arch
	desplazamiento = desplazamiento + sizeof(int) ;
	memcpy( mensaje + desplazamiento,(char*) archivo_resultado , long_archivo); //nombre arch
	desplazamiento = desplazamiento + long_archivo;
	memcpy( mensaje + desplazamiento, (struct in_addr*)&ipNodo , sizeof(struct in_addr)); // ip nodo
	desplazamiento = desplazamiento + sizeof(struct sockaddr_in);
	memcpy(mensaje + desplazamiento, (int*)&puertoNodo, sizeof(int)); //puerto nodo*/
    vincularSocket(&socketFs,direccionFs);
	enviarTodo(socketFs,mensaje,longitudMensaje);
	free(mensaje);
	close(socketFs);
 }



void recibirErrorSolicitud(t_conexion conexion,int indice)
{

	recv(conexion.socket,(int*)&indice,sizeof(int),0);


}

void operacionErronea(t_conexion conexion)
{
	t_cabecera cabecera;
	void* mensaje;
	cabecera.mensaje = Error;
    cabecera.tamanio= 0;
	mensaje = malloc(sizeof(t_cabecera));
	empaquetarCabecera(cabecera,mensaje);
	enviarTodo(conexion.socket,mensaje,sizeof(t_cabecera));
	free(mensaje);
}

void enviarFinDeOperacion(t_conexion conexion)
{
	t_cabecera cabecera;
	void * mensaje = malloc(sizeof(t_cabecera));
	cabecera.mensaje = FinDeOperacion;
	cabecera.mensaje = 0;
	empaquetarCabecera(cabecera,mensaje);
	enviarTodo(conexion.socket,mensaje,sizeof(t_cabecera));
	free(mensaje);
}

void recibiryModificarEstado(t_conexion conexion,t_list* listaNodos)
{ // ip + puerto + estado
	struct in_addr ipNodo;
	int puertoNodo=0;
	int estado= 0;
	recv(conexion.socket,(struct in_addr*)&ipNodo, sizeof(struct in_addr),0);
	recv(conexion.socket,(int*)&puertoNodo, sizeof(int),0);
	recv(conexion.socket,estado,sizeof(int),0);
	t_nodo* nodo = buscarNodo(listaNodos,ipNodo,puertoNodo);
	nodo->estado = estado;
}
