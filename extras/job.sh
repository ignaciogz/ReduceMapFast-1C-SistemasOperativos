#!/bin/bash
#Levanta los make del proyecto
#Hay que correrlo con source antes
#Levanto el path para que encuentre las librerias comunes
export LD_LIBRARY_PATH=/home/utnso/tp-2015-1c-mercurio/
rm -r jobcompilado
mkdir jobcompilado
cd proceso_job
cp -r src ../jobcompilado
cp -r config ../jobcompilado
cp makefile ../jobcompilado
cd ..
cd jobcompilado
make
cd ..
