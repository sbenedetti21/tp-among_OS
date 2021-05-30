#ifndef MI_RAM_H
#define MI_RAM_H
#define BACKLOG 10 //TODO

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"

t_log * loggerMiram; 
void servidorPrincipal(t_config*);
void iniciarMapa();

void atenderDiscordiador(int);
void recibir_TCB(int);
uint32_t crearPCB(char*);
TCB * deserializar_TCB(t_buffer *);
char * deserializar_Tareas(t_buffer * );
 
int proximoPID = 0; 

 
#endif
