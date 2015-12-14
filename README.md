ReduceMapFast - Plataforma de Multiprocesamiento Paralelo
======
Inspirada en los conceptos de Google MapReduce, emulando algunas características del funcionamiento del proyecto Hadoop.

El propósito del sistema es permitir al usuario ejecutar operaciones de análisis sobre
grandes volúmenes de datos dividiendo el trabajo en subtareas y distribuyéndolas para ser
ejecutadas de manera paralela en los diversos nodos del cluster de procesamiento.

Para una mayor comprensión, utilizaremos el siguiente ejemplo:
-----------------
Imaginemos que tenemos el requerimiento de contar la cantidad de palabras escritas en un
libro impreso de 100 hojas.

Podríamos hacerlo nosotros solos con mucha paciencia, o bien podríamos darle 10 hojas a
10 personas y sumar sus resultados parciales; también podríamos dar una hoja por persona
a 100 personas y luego sumar los 100 resultados parciales.

Cada uno de estos escenario demanda menor tiempo de procesamiento (contar palabras)
pero requiere de mayor disponibilidad de recursos (personas) e incrementa el tiempo de
planificación y organización (llevar el control de quién tiene cada hoja y luego recopilar y
sumar los resultados parciales).

Claramente debe haber un equilibro entre el nivel de distribución y el tamaño de la tarea para
así optimizar el rendimiento. Una incorrecta planificación (por ejemplo, darle de contar una
palabra a cada persona) no sólo no aumenta la performance, sino que la puede disminuir
drásticamente.

La plataforma que permitirá, dado un set de datos genérico (el libro en el ejemplo),
aplicarle una rutina de mapping (separarlo por palabras) y luego aplicar una rutina de reducción (suma) sobre los resultados parciales
distribuidos en diversos nodos de un mismo cluster.

Ahora, extendamos el ejemplo anterior a contar las palabras en un conjunto de libros .
Entenderá que no demanda el mismo esfuerzo un libro escrito en Arial 10 que un libro con
ilustraciones para niños. Dado que el resultado final (la cantidad de palabras) solo se puede
obtener al concluir todas las tareas en las que dividió el trabajo, es necesario para maximizar
la utilización de los recursos tener una distribución balanceada de tareas haciéndolas lo más
equitativas posibles.

Comprenderá el valor de dividir los set de datos en bloques de igual tamaño y así lograr que
todas las tareas demanden el mismo esfuerzo y poder así lograr una planificación determinística.

ARQUITECTURA:
-----------------

![Sin titulo](arquitectura.jpeg)

PROCESO DESARROLLADO:
-----------------
