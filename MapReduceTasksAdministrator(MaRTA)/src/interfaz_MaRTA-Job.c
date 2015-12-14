#include "interfaz_MaRTA-Job.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <string.h>
#include <pthread.h>
#include "interfaz_MaRTA-filesystem.h"

extern pthread_mutex_t mutex;



void imprimirIpPendientesDeMappingYPendientesDeReduce(t_job* job)
{
	char* ip = inet_ntoa(job->direccion.sin_addr);
	printf("\033[34mIp:\033[37m%s\n",ip);
	printf("\033[34mPendientes de mapping:\033[37m%d\n",job->pend_mapping);
	printf("\033[34mPendientes de reduce:\033[37m%d\n", job->pend_reduce);
	printf("\n");
}

t_job* buscarJob(t_list* lista, struct in_addr ip , int puerto)
{
	int cantidadJobs = list_size(lista);
	int i;
	t_job* jobBuscado;
	for(i=0;i < cantidadJobs;i++)
	{
		t_job* job = list_get(lista,i);
		if((job->direccion.sin_addr.s_addr == ip.s_addr) && (job->direccion.sin_port == puerto))
		{
			jobBuscado = job;
		}
	return jobBuscado;
}
}

t_nodosDelArchivo* seleccionarNodoConMayorCantidadDeCopias(t_list* nodos)
{
	pthread_mutex_lock( &mutex );
	t_nodosDelArchivo* nodoAux; //VARIABLE PARA RECIBIR LOS GET DE LA LISTA
	t_nodosDelArchivo* nodoCandidato; //NODO CANDIDATO A SER MAPEADO
	int index = 1; //VARIABLE PARA MANEJAR LOS INGRESOS A LA LISTA DE NODOS, INICIALIZADO EN 1 YA QUE EL CICLO WHILE IGNORARA EL PRIMER ELEMENTO DE LA LISTA

	//TOMO EL PRIMER ELEMENTO DE LA LISTA Y LO MARCO COMO CANDIDATO
	nodoCandidato = list_get(nodos, 0);

	//RECORRO EL RESTO DE LA LISTA Y COMPARO LA CANTIDAD DE COPIAS DEL NODO OBTENIDO CON EL CANDIDATO Y LO REEMPLAZO EN CASO DE SER NECESARIO
	while(index < list_size(nodos))
	{
		nodoAux = list_get(nodos, index);
		if(list_size(nodoAux->copias) > list_size(nodoCandidato->copias)) //SI EL NODO AUXILIAR TIENE MAS COPIAS QUE EL CANDIDATO
			{
			nodoCandidato = nodoAux;
			}

		index++;

	}

	return nodoAux;
	pthread_mutex_unlock( &mutex );
}

//DADO UN NODO Y UN BITARRAY PONE LOS BITS CORRESPONDIENTES A CADA BLOQUE DEL ARCHIVO EN 1 PARA MARCARLO COMO QUE YA TIENE UN NODO ASIGNADO
//EN CASO DE QUE UN BLOQUE YA ESTUVIERA EN 1 (YA FUE ASIGNADO ANTERIORMENTE A OTRO NODO) SE ELIMINA ESE BLOQUE DE LAS COPIAS DEL NODO
void actualizarBitarray(t_nodosDelArchivo* nodo, t_bitarray* bitarray)
{
	int index = 0;
	t_copiasPorNodo* copiaAux;
	while(index < list_size(nodo->copias))
	{
		copiaAux = list_get(nodo->copias, index);
		if(bitarray_test_bit(bitarray, copiaAux->bloqueDeArchivo) == true)
		{
			list_remove(nodo->copias, index);
		}
		else
		{
			bitarray_set_bit(bitarray, copiaAux->bloqueDeArchivo);
			index++;
		}
	}

	return;
}

//ELIMINA ELEMENTOS DE LA LISTA DE NODOS Y LA LISTA DE COPIAS DE CADA NODO A MODO QUE AL FINALIZAR SOLO QUEDEN NODOS CON COPIAS DE
//BLOQUES AUN NO ASIGNADOS Y LOS NODOS SOLO POSEEN COPIAS SIN ASIGNAR
void actualizarNodos(t_nodosDelArchivo* ultimoNodoSeleccionado, t_list* nodos, t_bitarray* bitarray)
{
	pthread_mutex_lock( &mutex );
	int indexNodos = 0;
	int indexCopias = 0;
	t_nodosDelArchivo* nodoAux;
	t_copiasPorNodo* copiaAux;
	char* ipNodoSeleccionado = malloc( strlen( inet_ntoa(ultimoNodoSeleccionado->ipNodo)) + 1);
	strcpy( ipNodoSeleccionado, inet_ntoa(ultimoNodoSeleccionado->ipNodo));

	while(indexNodos < list_size(nodos))
	{
		nodoAux = list_get(nodos, indexNodos);
		if(strcmp(ipNodoSeleccionado, inet_ntoa(nodoAux->ipNodo)) == 0)
		{
			//SI EL NODO OBTENIDO DE LA LISTA ES EL MISMO QUE EL ULTIMO NODO SELECCIONADO, LO ELIMINO DE LA LISTA
			list_remove(nodos, indexNodos);
		}
		else
		{
			while(indexCopias < list_size(nodoAux->copias))
			{
				copiaAux = (t_copiasPorNodo*)list_get(nodoAux->copias,indexCopias);
				if(bitarray_test_bit(bitarray, copiaAux->bloqueDeArchivo) == true)
				//SI LA COPIA DEL NODO OBTENIDA DE LA LISTA YA FUE ELEGIDA PARA OTRO NODO, BORRO LA COPIA DE LA LISTA DEL NODO
					list_remove(nodoAux->copias, indexCopias);
				else
					indexCopias++;
			}

		if(list_size(nodoAux->copias) == 0)
		//SI EL NODO YA NO TIENE COPIAS SIN ASIGNAR ENTONCES LO BORRO DE LA LISTA
			list_remove(nodos, indexNodos);
		else
			indexNodos++;

		indexCopias = 0;

		}
	}

	return;
	pthread_mutex_unlock( &mutex );
}


//ELIGE LOS NODOS Y COPIAS QUE VAN A SER MAPEADOS PARA EL ARCHIVO
//LUEGO DE SU EJECUCION QUEDARAN REGISTRADOS LOS NODOS Y COPIAS EN LA LISTA nodosConCopiasAMapear DEL ARCHIVO
void distribuirTareasSinCombiner(t_archivo* archivo)
{
	pthread_mutex_lock( &mutex );
	t_nodosDelArchivo* nodoConMasCopias;

	while(list_size(archivo->nodos) > 0)
	{
		nodoConMasCopias = seleccionarNodoConMayorCantidadDeCopias(archivo->nodos);

		actualizarBitarray(nodoConMasCopias, archivo->bitarray);

		list_add(archivo->nodosConCopiasAMapear, nodoConMasCopias);

		actualizarNodos(nodoConMasCopias, archivo->nodos, archivo->bitarray);
	pthread_mutex_unlock( &mutex );
	}

	return;
}
t_nodosDelArchivo* buscarNodoArchivo(t_list* lista, struct in_addr ip, int puerto)
{
	pthread_mutex_lock( &mutex );
	bool nodoTieneMismaIpYPuerto (t_nodosDelArchivo* nodo)
		{
		bool value = (nodo->ipNodo.s_addr == ip.s_addr) && (nodo->puerto_nodo == puerto);
		return value;
		}
	   t_nodosDelArchivo* nodoBuscado = list_find(lista,nodoTieneMismaIpYPuerto);
	   return nodoBuscado;
	   pthread_mutex_unlock( &mutex );
}
void insertarNodoSolicitadoOCopia(struct in_addr ip,int puerto,t_copiasPorNodo* copiaNueva,t_list* nodos)
{
	pthread_mutex_lock( &mutex );
	t_nodosDelArchivo* busqueda = buscarNodoArchivo(nodos,ip,puerto);
	if(busqueda != NULL)// si el nodo ya estaba en la lista agrego solo la copia
	{
       list_add(busqueda->copias,copiaNueva);
    }
	else
	{ // si no esta, lo agrego a la lista
		t_nodosDelArchivo* nodoSolicitado = crearNodoDelArchivo(ip,puerto);
		list_add(nodoSolicitado->copias,copiaNueva);
		list_add(nodos,nodoSolicitado);
	}
	pthread_mutex_unlock( &mutex );
}

t_nodosDelArchivo* crearNodoDelArchivo(struct in_addr ip,int puerto)
{
	        t_nodosDelArchivo* nodo=malloc(sizeof(t_nodo));
			nodo->copias = list_create();
			nodo->ipNodo = ip;
			nodo->puerto_nodo = puerto;
			return nodo;
}

t_copiasPorNodo* crearCopiaPorNodo(int bloqueDeArchivo,int bloqueInterno,char* archivo)
{
	t_copiasPorNodo* copia= malloc(sizeof(t_copia));
	copia->bloqueDeArchivo = bloqueDeArchivo;
	copia->bloqueInterno = bloqueInterno;
	char*nombre = string_new();
	char* numeroBloque = string_itoa(copia->bloqueDeArchivo);
	string_append(&nombre,"resultadomap");
	string_append(&nombre,archivo);
	string_append(&nombre,numeroBloque);
	string_append(&nombre,".txt");
	return copia;
}

t_archivo* buscarArchivo(char* nombreArchivo,t_job* job)
{
    int cantArch = list_size(job->archivos);
    t_archivo* archivoNuevo;
    int i =0;
    while (i < cantArch)
    {
    	t_archivo* archivo = list_get(job->archivos,i);
    	int comparacion = strcmp(archivo->archivo,nombreArchivo);
    	if(comparacion == 0)
    	{
    		archivoNuevo = archivo;
    	}
    	i++;
    }
	return archivoNuevo;
}

t_copia* buscarCopia(t_bloque* bloque,struct in_addr ip, int puerto)
{
  pthread_mutex_lock( &mutex );
  bool busqueda(t_copia* copia)
  {
	  bool value= copia->ipNodo.s_addr == ip.s_addr && copia->puerto_nodo == puerto;
	  return value;
  }
  t_copia* copiaBuscada = list_find(bloque->copias,busqueda);
  return copiaBuscada;
  pthread_mutex_unlock( &mutex );
}

void elegirYGuardarCopiaCandidata(t_bloque* bloque,t_list*listaNodos)
{
   pthread_mutex_lock( &mutex );
   t_nodo* elMasDesocupado = malloc(sizeof(t_nodo));
   int longitudLista= list_size(bloque->copias);
   t_list* listaAux = list_create();
   int i = 0;
   while(i<longitudLista)
   {
	   t_copia* copia = list_get(bloque->copias,i);
	   struct in_addr ip = copia->ipNodo;
	   int puerto = copia->puerto_nodo;
	   t_nodo* nodoBuscado = buscarNodo(listaNodos,ip,puerto);
	   list_add(listaAux,nodoBuscado);
	   i++;
   }

   buscarNodoMenosOcupado(listaAux,elMasDesocupado);
   t_copia* copiaBuscada = buscarCopia(bloque,elMasDesocupado->ip,elMasDesocupado->puerto);
   bloque->copiaCandidata = *copiaBuscada;
   aumentarRutinasMapping(*elMasDesocupado);
   list_clean(listaAux);
   list_destroy(listaAux);
   free(elMasDesocupado);
   pthread_mutex_unlock( &mutex );
}

void distribuirTareasConCombiner(t_archivo* archivo,t_list*listaNodos)
{
	void procesarArchivo(t_bloque* bloque)
	{
		elegirYGuardarCopiaCandidata(bloque,listaNodos);
	}
	pthread_mutex_lock( &mutex );
    list_iterate(archivo->bloques,elegirYGuardarCopiaCandidata);
    pthread_mutex_unlock( &mutex );
}


int longArchivosResultadoMapping(t_job * job,t_archivo* archivo)
{
	int longitud=0, i=0 , j=0 , cantNodosAMapear, largoNombre;
	if(job->soporte_combiner == si)
	{
	    cantNodosAMapear = list_size(archivo->bloques);
	    while(i < cantNodosAMapear)
	    {
	    	t_bloque * bloque = list_get(archivo->bloques,i);
	    	largoNombre = string_length(bloque->archivoResultadoMap);
	    	longitud = longitud + largoNombre;
	    	i++;
	    }
	}
	else
	{
		cantNodosAMapear = list_size(archivo->nodosConCopiasAMapear);
		while(i< cantNodosAMapear)
		{
			t_nodosDelArchivo* nodo = list_get(archivo->nodosConCopiasAMapear,i);
			int cantCopias = list_size(nodo->copias);
			while(j < cantCopias)
			{
				t_copiasPorNodo* copia = list_get(nodo->copias,j);
				largoNombre = string_length(copia->archivoResultadoMap);
				longitud = longitud + largoNombre;
				j++;
			}
            i++;
		}
	}
    return longitud;
}



void disminuirPendReduce(t_job* job)
{
	job->pend_reduce --;
}

void eliminarJob(t_list* listaJobs,struct in_addr ip, int puerto)
{
	bool tieneMismaIpYPuerto(t_job*job)
	{
		bool value =(job->direccion.sin_addr.s_addr == ip.s_addr) && (job->direccion.sin_port == puerto);
		return value;
	}
	list_remove_by_condition(listaJobs,tieneMismaIpYPuerto);
}

t_bloque* inicializarBloque(t_job* job,int nroBloque,t_archivo* archivo)
{
	t_bloque* bloqueNuevo = malloc(sizeof(t_bloque));
	bloqueNuevo->copias = list_create();
	char* numeroBloque = string_itoa(nroBloque);
	char*nombre = string_new();
	string_append(&nombre,job->archivo_resultado);
	string_append(&nombre,archivo->archivo);
	string_append(&nombre,numeroBloque);
	string_append(&nombre,".txt");
	return bloqueNuevo;
}
void asignarPendMapping(t_job* job, int cantidad)
{
	job->pend_mapping = cantidad;
}

void gestionarCombiner(t_combiner combiner,t_archivo* archivo,t_list*listaNodos)
{
	if(combiner == si)
	{
		distribuirTareasConCombiner(archivo,listaNodos);
	}
	else
	{
		distribuirTareasSinCombiner(archivo);
	}
}

int cantidadDeCopiasQueMapean(t_combiner combiner,t_archivo archivo)
{
	int cantidad = 0;
	if(combiner == si)
	{ // hay una copia candidata por bloque, por lo tanto la cantidad de copias candidatas sera la misma que de bloques
		cantidad = list_size(archivo.bloques);
	}
	else
	{// hay una lista con los nodos q van a mapear (nodosConCopiasAMapear) dentro de c/u estan las copias candidatas
	    int i = 0;
		int	cantidadNodos = list_size(archivo.nodosConCopiasAMapear);
		while(i < cantidadNodos)
		{
			t_nodosDelArchivo* nodo = list_get(archivo.nodosConCopiasAMapear,i);
			int cantidadCopias = list_size(nodo->copias);
			cantidad = cantidad + cantidadCopias;
			i++;
		}
	}
	return cantidad;
}

t_copia seleccionarNodoReductor(t_job* job)
{
	int cantidad_archivos,i,cantidadCopias;
	t_copia copia; // como solo me interesa la ip y el puerto da igual si lo q retorno es un tipo t_copia o un t_nodosDelArchivo
	copia.bloqueInterno = 0; // para no dejarla incompleta :)
	int maximo = 0;
	if(job->soporte_combiner == si)
 	{ // Da igual en cual se reduzca asique tomo la ip y puerto de la copia candidata del primer bloque del primer archivo de la lista de archivos del job
 		t_archivo* archivo = list_get(job->archivos,0);
        t_bloque* bloque = list_get(archivo->bloques,0);
 		t_copia  copiaCandidata =  bloque->copiaCandidata; //en ella se va a reducir
 		copia.ipNodo = copiaCandidata.ipNodo;
 		copia.puerto_nodo = copiaCandidata.puerto_nodo;
 	}
 	else
 	{ //En este caso primero me interesa saber los nodos con mayor cantidad de copias de cada archivo. Una vez obtenido esto, el elegido para hacer el reduce sera el que mayor cantidad de copias tenga
 		i = 0;
 		cantidad_archivos = list_size(job->archivos);

 		t_archivo* archivo = list_get(job->archivos,0);
 		t_nodosDelArchivo* guachin = list_get(archivo->nodosConCopiasAMapear,0);
 		maximo = list_size(guachin->copias);
 		copia.ipNodo = guachin->ipNodo;
 		copia.puerto_nodo = guachin->puerto_nodo;
 		while(i < cantidad_archivos)
 		{
 		      t_archivo* archivo = list_get(job->archivos,i);
 			  t_nodosDelArchivo*nodoCandidato = seleccionarNodoConMayorCantidadDeCopias(archivo->nodosConCopiasAMapear);
 			  cantidadCopias = list_size(nodoCandidato->copias);
 			  if(cantidadCopias > maximo)
 			  {
 				 copia.ipNodo = nodoCandidato->ipNodo;
 				 copia.puerto_nodo = nodoCandidato->puerto_nodo;
 				 maximo = list_size(nodoCandidato->copias);
 			  }
 			  i++;
 		}
 	}
	return copia;
}
