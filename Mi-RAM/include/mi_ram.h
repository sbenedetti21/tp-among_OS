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
	int* header = malloc(sizeof(int));

	int prueba = recv(socketCliente,(void*) header , sizeof(int) , 0);
	printf("Recibi header: %d\n", *header);

	switch (*header)
	{
	case CREAR_PCB: ; // <- preguntar label
	
		uint32_t * punteroPCB = malloc(sizeof(punteroPCB));
		*punteroPCB = crearPCB("pathTareas");

		send(socketCliente, punteroPCB, sizeof(uint32_t),0);

		free(punteroPCB);

		break;

	case CREAR_TCB: ;  // pasar desde discordiador

			TCB * tripulante = malloc(sizeof(TCB));
			int status =  recv(socketCliente, (void *) tripulante, sizeof(TCB), 0); 

			printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

			printf("------------------------\n");
			free(tripulante);


		break;

	case PRUEBA: ;

	
		break;
	
	default:	



		break;
	}

	free(header);


	

}


uint32_t crearPCB(char* tareas){
	// uint32_t punteroTareas = *tareas; //ESTO ESTA RARI

	PCB * patota = malloc(sizeof(PCB));
	patota->pid = proximoPID; 				
	// patota->tareas = punteroTareas; // hABRIA QUE ALMACENAR LA PATOTA EN ALGUN LADO

	proximoPID++;


	uint32_t a = patota->pid;

	free(patota);
	return a;
}

#endif
