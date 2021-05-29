#ifndef MI_RAM_H
#define MI_RAM_H
#define BACKLOG 10 //TODO

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"

void atenderDiscordiador(int);
void recibir_TCB(int);
uint32_t crearPCB(char*);
TCB * deserializar_TCB(t_buffer *);
 
int proximoPID = 0; 


void atenderDiscordiador(int socketCliente){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	

	switch (paquete->header)
	{
	case CREAR_PCB: ; // <- preguntar label
	
		uint32_t * punteroPCB = malloc(sizeof(punteroPCB));
		*punteroPCB = crearPCB("pathTareas");

		send(socketCliente, punteroPCB, sizeof(uint32_t),0);

		free(punteroPCB);

		break;

	case CREAR_TCB: ;  // pasar desde discordiador

		int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);

		if(headerRECV) { printf("Recibi header: %d\n", paquete->header);} else{ printf("No pude recibir el header. \n");}

		int tamanioPAQUETE_RECV = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

		if(! tamanioPAQUETE_RECV){ printf("No pude recibir el tamanio del buffer \n");}

		paquete->buffer->stream = malloc(paquete->buffer->size);

		int PAQUETE_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size,0);

		if(! PAQUETE_RECV){ printf("No pude recibir el PAQUETE \n");}

		TCB * tripulante = deserializar_TCB(paquete->buffer);

			printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

			printf("------------------------\n");
			free(tripulante);

		break;

	case PRUEBA: ;

	
		break;
	
	default:	



		break;
	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	


}


TCB * deserializar_TCB(t_buffer * buffer){
	TCB * tripulante = malloc(sizeof(TCB));

	void* stream = buffer->stream;

	//Deserializamos los campos que tenemos en el buffer
	memcpy(&(tripulante->tid), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->punteroPCB), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	//	memcpy(&(tripulante->estado), stream, sizeof(char));
	//	stream += sizeof(char);

	return tripulante;
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
