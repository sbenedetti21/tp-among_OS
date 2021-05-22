#ifndef MI_RAM_H
#define MI_RAM_H
#define BACKLOG 10 //TODO

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"

void atenderDiscordiador(int);


void atenderDiscordiador(int socketCliente){

	 TCB * tripulante = malloc(sizeof(TCB));



	int status =  recv(socketCliente, (void *) tripulante, sizeof(TCB), 0);

	printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

	printf("------------------------\n");
	free(tripulante);


}

#endif
