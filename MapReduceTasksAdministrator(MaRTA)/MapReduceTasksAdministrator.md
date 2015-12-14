Mensajes

1)DatosJob

Recibe del job un paquete del estilo:

 header |Cant archivos| Archivos | Combiner

por cada archivo recibe: tamaño del nombre | nombre archivo

El último de estos archivos se corresponde con el archivo de resultado

Envia al FS un paquete del estilo:

IpJob | pueroJob | longitud archivo | nombreArchivo


por cada archivo que usa el job


2)DatosBloques

Recibe del FS un paquete del estilo:

|header |IP JOB | Puerto JOB | TamanioNombre | Nombre del Arch | Cantidad total de bloques del Arch| Cantidad de Copias Bloque |Copias|

por cada copia recibe: 

| IP | Puerto | Numero de Bloque |

MaRTA realiza la distribucion de las tareas con y sin combiner

Envia al job la informacion de mapeo con el siguiente paquete:

|header | cantidad de copias | copias |

por cada copia se envia:

|ip nodo | puerto nodo | bloque interno | long Archivo Resultado Map | archivo ResultadoMap |


3) Error solicitud

recibe del FS el siguiente paquete:

| header | ip job | puertoJob |

MaRTA envia al proceso job un paquete vacío con un header que haga referencia a "error"

4) Operacion map erronea

Se recibe un paquete vacio con el header correspondiente

MaRTA envia un paquete vacio al proceso job con el header correspondiente a "error"

5) Operacion reduce erronea

Se recibe un paquete vacio con el header correspondiente

MaRTA envia un paquete vacio al proceso job con el header correspondiente a "error"

6) ConexionFileSystem

El FS manda un paquete del estilo:

| header | Cant Nodos | Nodos

por cada nodo manda: | ipNodo | puertoNodo |

7) OperacionMapExitosa

recibe un paquete del estilo:

| ip nodo | puerto nodo |

si todos los archivos del job fueron mapeados,MaRTA planifica el reduce.

Devuelve al job por cada archivo un paquete del siguiente estilo:

|ip nodo reductor | puerto nodo reductor 

8) OperacionReduceExitosa

Si ya se redujeron todos los archivos del job:

Envia al FS un paquete del estilo:

|ip nodo | puerto nodo | longitud archivo de resultado | archivo de resultado |

Envia al job un paquete solo con el header correspondiente a fin de operacion


9) NodoNuevo

recibe del Fs un paquete del estilo:

| ip nodo | puerto nodo | 

despues crea el nodo y lo agrega a la lista de nodos.

10) cambioDeEstado

| ipNod | puertoNodo | longEstado | estado |
