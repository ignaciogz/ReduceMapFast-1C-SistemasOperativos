################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

USER_OBJS :=
LIBS := -lcommons -lcunit -lpthread

O_SRCS := 
C_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
OBJS := 
C_DEPS := 
EXECUTABLES := 
# Every subdirectory with source files must be described here
SUBDIRS := \
src \

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Configuracion.c \
../src/Interfaz_Job-Marta.c \
../src/Interfaz_Job-Nodo.c \
../src/Log_Job.c \
../src/ManejoMensajesJob.c \
../src/Proceso_Job.c \
../src/comunes_job.c 

OBJS += \
./src/Configuracion.o \
./src/Interfaz_Job-Marta.o \
./src/Interfaz_Job-Nodo.o \
./src/Log_Job.o \
./src/ManejoMensajesJob.o \
./src/Proceso_Job.o \
./src/comunes_job.o 

C_DEPS += \
./src/Configuracion.d \
./src/Interfaz_Job-Marta.d \
./src/Interfaz_Job-Nodo.d \
./src/Log_Job.d \
./src/ManejoMensajesJob.d \
./src/Proceso_Job.d \
./src/comunes_job.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Proceso_Job

# Tool invocations
Proceso_Job: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "Proceso_Job" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Proceso_Job
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
