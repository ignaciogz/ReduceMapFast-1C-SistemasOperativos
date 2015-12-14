-----------------------------------------------------------------------------
1) Marta - Solicitud de los bloques que compenen un arch.
-----------------------------------------------------------------------------
-Ver si esta en la db

	Caso Afirmativo: 	Obtenemos los bloques de la DB "FORMATEADOS" y respondemos a Marta
						Solo le mandamos los BLOQUES que estan disponibles						
						
	Caso Negativo:		Respondemos "Arch NO Disponible"

Respuesta a Marta:
	IP JOB | Puerto JOB | Tamanio Nombre Arch | Nombre del Arch | Cantidad total de bloques del Arch 
	
	| Cantidad de Copias Bloque1 	
									| IP | Puerto | Numero de Bloque
									| IP | Puerto | Numero de Bloque
									| IP | Puerto | Numero de Bloque
	
	| Cantidad de Copias Bloque2 	
									| IP | Puerto | Numero de Bloque
									| IP | Puerto | Numero de Bloque
									| IP | Puerto | Numero de Bloque
									| IP | Puerto | Numero de Bloque
									| IP | Puerto | Numero de Bloque

En la representacion anterior, el bloque 1 posee 3 copias y el bloque 2
posee 5 copias.



-----------------------------------------------------------------------------
2) Marta - Solicitud de obtencion de un Arch final.
-----------------------------------------------------------------------------
- LLega la siguiente solicitud:
	
	Tamanio Nombre Arch | Nombre del Arch | IP del Nodo | Puerto del Nodo

- En base a la info anterior, le solicitamos al nodo ese archivo gracias a getFileContent.

Le mandamos al nodo:
	
	Tamanio Nombre Arch | Nombre del Arch

- El nodo nos devuelve:
	
	HASH |LargoNombreArch | NombreArch| Tamanio del Contenido | Contenido

- Guardamos en el disco, el archivo final.



-----------------------------------------------------------------------------
3) CONSOLA 
-----------------------------------------------------------------------------
3.1) Formatear MDFS
	Eliminar la DB y crearla nuevamente.

3.2) y 3.3)
	Los resolvemos con bibliotecas de c para manejo de archivos.

3.4) Copiar Arch Local al MDFS
	
	- Usar mmap
	- Usar algoritmo de nodo prioridad
	- Armar una estructura para envio posterior de una.
	- Mandar a todos los nodos correspondientes (En caso de un fallo X, RollBack aplicando modelo Transaccional. Volver a estado anterior!).
	- Todo sale OK, cargar en la DB la estructura creada anteriormente.

3.5) Copiar MDFS al FS local
	
	- Obtenemos los bloques de la DB "FORMATEADOS"
	- A los nodos les pedimos la bloque(Parte) X del archivo, con getBloque.
	- Armamos el archivo (Al ultimo bloque hay que sacarle los barra cero!).
	
	Juancho nos envia char asteriscos

Le mandomos al nodo:
	| numero de bloque

	- En caso de que el archivo no este Disponible, lo informamos

3.6) Solicitar el MD5 de un archivo del MDFS
	
	Hay que almacenar los datos en archivos para aplicarlo!

3.7.1) Ver Bloques que compenen un archivo
	
	- Listar en pantalla la info de los bloques que lo conforman. Tomado de la DB y Formateado

3.7.2) Borrar los bloques que compenen un archivo
	
	- Ir a la DB y limpiar los bloques

3.7.3) Copiar los bloques que componen un archivo (DUDA copia todos los bloques o un Bloque????)
	
	- Obtener el contenido de del bloque de algun nodo que lo contenga.
	- Enviar el contenido obtenido previamente, al bloque deseado para que almacene la nueva copia.
	- Si esta todo OK, ir la DB y Agregar una nueva copia a ese X Bloque del Archivo.

3.8) Agregar un nodo de Datos
	
	- Registrarlo / Actualizarlo en nodo historial

3.9) Eliminar un nodo de Datos
	
	- Borrarlo de nodo historial