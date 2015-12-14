#!/bin/bash
#Levanta los make del proyecto
#Hay que correrlo con source antes
#Levanto el path para que encuentre las librerias comunes
export LD_LIBRARY_PATH=/home/utnso/tp-2015-1c-mercurio/
cd proceso_nodo
#rm Nodo.log
rm -r compilado
mkdir compilado
cp -r src compilado
cp makefile compilado
cd compilado
make
cd ..
cd MapReduceTasksAdministrator(MaRTA)
#rm Marta.log
rm -r compilado
mkdir compilado
cp -r src compilado
cp makefile compilado
cd compilado
make
cd ..
bash job.sh | tee -a jobMake.log
cd proceso_filesystem
#rm filesystem.log
rm -r compilado
mkdir compilado
cp -r src compilado
cp makefile compilado
cd compilado
make
cd ..
