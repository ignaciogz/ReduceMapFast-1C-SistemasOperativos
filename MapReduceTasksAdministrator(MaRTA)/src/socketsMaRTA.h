#ifndef SOCKETSMARTA_H_
#define SOCKETSMARTA_H_
#include <netinet/in.h>
#include <arpa/inet.h>
#include<commons/collections/list.h>
#include "interfaz_MaRTA-Job.h"
#include "interfaz_MaRTA-filesystem.h"
//tipos de mensaje que puede haber en la cabecera(si falta alguno despues se agrega)
typedef enum {
	//Mensajes con File System
	ConexionFileSystem = 0,
	pedidoBloques = 1,
	datosBloques = 2,
	copiarArchivoDeResultado = 3,
	errorSolicitud = 4,
	nodoNuevo = 5,
	cambioDeEstado = 6,

	//Mensajes con Job
	datosJob= 16,
	pedidoMapping = 17,
	operacionMapExitosa = 18,
	operacionMapErronea = 19,
	pedidoReduce = 20,
	operacionReduceExitosa = 21,
	operacionReduceErronea = 22,
	FinDeOperacion = 23,
	Error = 24
}t_mensaje;

/// estructura del header
typedef struct {
	t_mensaje mensaje;
	int tamanio;
}t_cabecera;

/* tipo de dato para la conexion*/
typedef struct{
	int socket;
	struct sockaddr_in direccion;
}t_conexion;

int buscarIndiceJob(t_job* job, t_list* lista_jobs);

//dada una ip en formato char* y un numero de puerto, crea una direccion
struct sockaddr_in obtenerDireccion (int port, char* ip);

//agrega la cabecera al mensaje
void empaquetarCabecera(t_cabecera header, void* message);

//dada una cabecera para por referencia y un paquete,guarda en la cabecera los datos de la cabecera del paquete
void desempaquetarCabecera(t_cabecera* header, void* package);

//envia un mensaje asegurandose que no se mande incompleto
void enviarTodo(int socket, void* message, int length);

//dado un mensaje,un job, la longitud del mensaje, el desplazamiento(correspondiente a la cabecera) y un nombre de archivo(con su longitud) crea el paquete para solicitar los bloques que componen dicho archivo al FS
void crearPaqueteSolicitud(void* mensaje, t_job job, int longitudMensaje,int desplazamiento,char*nombreArchivo,int long_archivo, int indice);

//dada una conexion con los datos del job, desempaqueta el job, lo agrega a la lista de jobs y ademas envia al filesystem los nombres de los archivos con los que trabaja el job
void atenderJob(t_conexion conexion,t_list* listaJobs,struct sockaddr_in direccionFS,int socket);

//dada una conexion proveniente del proceso job y una lista de jobs, desempaqueta los datos del job, inicializa la variable de tipo job y agrega el job a la lista de jobs
t_job* desempaquetarJob (t_conexion conexion, t_list* lista_jobs);

// dado un puntero a un job y el tamaño de un archivo, desempaqueta los datos del archivo, inicializa a la variable de tipo t_archivo y la guarda en la lista de archivos del job
void cargarArchivo(t_conexion conexion,t_job* job,int tamanio_arch);

//Dada una conexion de FS con los datos de un nuevo nodo ingresado al sistema y una lista de nodos, agrega ese nodo a la lista de nodos
void cargarNodo(t_conexion conexion,t_list* listaNodos);

void atenderBloque(t_conexion conexion,t_list*listaJobs,t_list* listaNodos,int socket);

void crearPaqueteMappingConCombiner(t_archivo* archivo,void* mensaje,int longitudTotal,int desplazamiento,int cantCopias,t_list* listaNodos);

void crearPaqueteMappingSinCombiner(t_archivo*archivo,void*mensaje,int longitudTotal,int desplazamiento,int cantCopias,t_list* listaNodos);



//dado una conexion que contiene un paquete del estilo
void cargarBloque(t_conexion conexion,t_list*listaJobs,t_archivo* archivoBuscado,t_job*jobBuscado);

struct sockaddr_in generarDireccion(char*ip,int puerto);

// dado un mensaje, un nombre de archivo de resultado y un nodo, deja al mensaje de la siguiente forma: Cabecera | Tamanio Nombre Arch | Nombre del Arch | IP del Nodo | Puerto del Nodo
void crearPaqueteResultado (void* mensaje, char* archivo_resultado,struct in_addr ipNodo,int puertoNodo,int long_archivo,int desplazamiento);

void recibirErrorSolicitud(t_conexion conexion,int indice);


void crearPaqueteMapping(t_combiner combiner,t_archivo* archivo,void* mensaje,int longitudTotal,int desplazamiento,t_list* listaNodos,int cantCopias);


void gestionarReduce(t_conexion conexion,t_list* listaJobs,t_list* listaNodos,struct sockaddr_in direccionFs);

void vincularSocket(int* socketNuevo, struct sockaddr_in direccionJob);


void operacionErronea(t_conexion conexion);

void enviarFinDeOperacion(t_conexion conexion);

void gestionarResultadoMap(t_conexion conexionJob,t_list*listaJobs,t_list* listaNodos);

void recibiryModificarEstado(t_conexion conexion,t_list* listaNodos);

void mandarACopiarArchivoResultado(char* archivo_resultado,struct in_addr ipNodo,int puertoNodo,struct sockaddr_in direccionFs);

void mandarPedidoReduce(t_copia copia,t_conexion conexion);


#endif /* SOCKETSMARTA_H_ */
