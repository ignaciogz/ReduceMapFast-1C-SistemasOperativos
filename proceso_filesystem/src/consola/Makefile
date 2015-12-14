CC := gcc

OBJS := consola.o subPantallasDeLaConsola.o comandosEjecutables.o funcionesDeSubPantallas.o
HDRC := headers/consola.h headers/subPantallasDeLaConsola.h
HDRS := headers/subPantallasDeLaConsola.h headers/funcionesDeSubPantallas.h headers/comandosEjecutables.h

#All
all: compila


#Compila
compila: $(OBJS)
	$(CC) -lcommons -o compila $(OBJS) 

consola.o: consola.c $(HDRC)
	$(CC) -c consola.c
subPantallasDeLaConsola.o: subPantallasDeLaConsola.c $(HDRS)
	$(CC) -c subPantallasDeLaConsola.c
funcionesDeSubPantallas.o: funcionesDeSubPantallas.c  headers/funcionesDeSubPantallas.h
	$(CC) -lcommons -c funcionesDeSubPantallas.c 
comandosEjecutables.o: comandosEjecutables.c headers/comandosEjecutables.h
	$(CC) -c comandosEjecutables.c


#Clean
clean:
	rm compila $(OBJS)