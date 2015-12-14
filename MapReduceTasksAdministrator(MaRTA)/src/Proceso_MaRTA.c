#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "interfaz_MaRTA-filesystem.h"
#include "interfaz_MaRTA-Job.h"
#include "interfaz_MaRTA-Pantalla.h"
#include "socketsMaRTA.h"
#include "mensajes.h"
#include "configuracion.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int main() {
	struct sockaddr_in miDireccion;
	struct sockaddr_in direccionFS;
	printf("\033[1mMapReduceTasksAdministrator iniciado...\n");
	int desplazamiento,recivido;
	t_conexion conexionNueva;
	t_parametro parametros;
	t_parametroPantalla parametrosPantalla;
	t_config * configuracion;
    configuracion = config_create("Configuracion_MaRTA.cfg");
	//Se crean la lista para manejar los jobs(elementos de tipo t_job), la lista de nodos (elementos de tipo t_nodo)
	t_list* lista_jobs = list_create();
	t_list* lista_nodos = list_create();
	char*ipMarta=obtener_string(configuracion,"IP_MARTA");
	int puertoMarta= obtener_int(configuracion,"PUERTO_MARTA");
	char*ipFS=obtener_string(configuracion,"IP_FS");
	int puertoFS=obtener_int(configuracion,"PUERTO_FS");
    miDireccion = generarDireccion(ipMarta,puertoMarta);
    direccionFS = generarDireccion(ipFS,puertoFS);
    int socketMarta;


		socketMarta = socket(AF_INET, SOCK_STREAM, 0); //socket en el que el proceso va a estar escuchando nuevas solicitudes
		if (socketMarta == -1)
		{
			perror("socket");
			exit(1);
		}
    printf("\033[32m--------------------------------------------------\n");
    printf("     Informacion del archivo de configuracion     \n");
    printf("--------------------------------------------------\033[37m\n");
    printf("\n");
	printf("Direccion del proceso MapReduceTasksAdministrator:\n");
	printf("\n");
	printf("\033[32mIp:\033[37m%s\n",inet_ntoa(miDireccion.sin_addr));
	printf("\033[32mpuerto:\033[37m%d\n", ntohs(miDireccion.sin_port));
	printf("--------------------------------------------------\n");
	printf("Direccion del proceso filesystem:\n");
	printf("\n");
	printf("\033[32mIp:\033[37m%s\n",inet_ntoa(direccionFS.sin_addr));
	printf("\033[32mpuerto:\033[37m %d\n", ntohs(direccionFS.sin_port));
	printf("--------------------------------------------------\n");


	if( bind(socketMarta,(struct sockaddr*)&miDireccion,sizeof(struct sockaddr)) == -1 ) //se bindea al proceso a un puerto especifico y se comprueban errores
	{
		perror("bind");
		exit(1);
	}

	printf("esperando conexiones...\n");
	//Esperar a que filesystem este en estado operativo
	struct sockaddr_in direccionEntrante;
	int nuevoSocket;
	socklen_t size = sizeof(struct sockaddr_in);
	t_cabecera cabecera;
	void* mensaje = malloc(sizeof(t_cabecera));

	if(listen(socketMarta, 5) == -1) // el proceso se pone a la espera de conexiones y se comprueba errores
	{
		perror("listen");
		exit(1);
	}
do{

           nuevoSocket = accept(socketMarta,(struct sockaddr*) &direccionEntrante,&size);
           if(nuevoSocket == -1)
           {
        	   perror("accept");
        	   exit(1);
           }

		desplazamiento = 0;
		recivido = 0;

		//Hago el recv comprobando errores y asegurandome que reciba el paquete entero
		do{
			do{
				recivido = recv(nuevoSocket, mensaje, sizeof(t_cabecera), 0);
			}while(recivido < 0);
			desplazamiento = desplazamiento + recivido;
		}while (desplazamiento < sizeof(int));

		desempaquetarCabecera(&cabecera ,mensaje);//desempaqueto la cabecera para ver si es una conexion del file system o el envio de los datos de un nodo nuevo

	}while(cabecera.mensaje != ConexionFileSystem); //si el mensaje no es la conexion del file system vuelvo a aceptar otro mensaje



	printf("conexion con filesystem establecida!\n");
	//close(nuevoSocket);
	// agrego los nodos que vienen del fs
    parametros.cabecera.mensaje= cabecera.mensaje;
    parametros.cabecera.tamanio= cabecera.tamanio;
    parametros.conexion.direccion = direccionFS;
    parametros.conexion.socket = nuevoSocket;
    parametros.direccionFs = direccionFS;
    parametros.listaJobs = lista_jobs;
    parametros.listaNodos = lista_nodos;
    pthread_t hilo;
    pthread_create( &hilo, NULL,(void*)&atenderMensaje,&parametros);
	parametrosPantalla.hilo = hilo;
	parametrosPantalla.listaJobs = lista_jobs;
	parametrosPantalla.listaNodos = lista_nodos;
    pthread_create( &hilo, NULL,(void*)&mostrarPorPantalla,&parametrosPantalla);
    //close(nuevoSocket);
	// escucho nuevas conexiones

    while(1)
		{
		     //Acepto las solicitudes entrantes
		conexionNueva.socket = accept(socketMarta, (struct sockaddr*)&conexionNueva.direccion,(socklen_t*) &size);
		if(conexionNueva.socket == -1)
		{
			perror("accept");
					exit(1);
		}
		    // Asigno valores a la variable con los parametros

		recv(conexionNueva.socket,(int*)&parametros.cabecera.mensaje,sizeof(int),0);
		recv(conexionNueva.socket,(int*)&parametros.cabecera.tamanio,sizeof(int),0);
		parametros.conexion = conexionNueva;
        parametros.direccionFs = direccionFS;
        parametros.listaJobs = lista_jobs;
        parametros.listaNodos = lista_nodos;
		   //Defino el  hilo que se encargara de la solicitud
		pthread_t hilo;
		pthread_t hiloPantalla;
		parametrosPantalla.hilo = hilo;
		parametrosPantalla.listaJobs = lista_jobs;
		parametrosPantalla.listaNodos = lista_nodos;
          //Creo el hilo nuevo
	    pthread_create( &hilo, NULL,(void*)&atenderMensaje,&parametros);
	    pthread_create( &hilo, NULL,(void*)&mostrarPorPantalla,&parametrosPantalla);

	    }
  return 0;
};


