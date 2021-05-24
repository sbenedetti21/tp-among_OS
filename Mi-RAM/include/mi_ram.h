#ifndef MI_RAM_H
#define MI_RAM_H
#define BACKLOG 10 //TODO

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"

void atenderDiscordiador(int);
uint32_t crearPCB(char*);

int proximoPID = 0; 


void atenderDiscordiador(int socketCliente){
	int* header;

	int prueba = recv(socketCliente,(void*) header , sizeof(int) , 0);
	printf("Recibi header\n");

	switch (*header)
	{
	case CREAR_PCB: ; // <- preguntar label
		char * pathTareas = "CULO";
		printf("Entre CREAR PCB EN SWITCH\n");

		//recv(socketCliente,pathTareas,sizeof(char*),MSG_WAITALL);
		
		uint32_t punteroPCB = crearPCB(pathTareas);

		send(socketCliente, punteroPCB, sizeof(uint32_t),0);

		break;

	case CREAR_TCB: ;

			TCB * tripulante = malloc(sizeof(TCB));
			int status =  recv(socketCliente, (void *) tripulante, sizeof(TCB), MSG_WAITALL); 

			printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

			printf("------------------------\n");
			free(tripulante);


		break;
	
	default:
		break;
	}


	

}


uint32_t crearPCB(char* tareas){
	printf("Entre crear PCB FUNCION\n");
	uint32_t punteroTareas = *tareas; //ESTO ESTA RARI

	PCB * patota = malloc(sizeof(PCB));
	patota->pid = proximoPID; 				
	patota->tareas = punteroTareas;

	proximoPID++;

	return 100;
}

#endif
