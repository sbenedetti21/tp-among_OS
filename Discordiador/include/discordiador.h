#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

typedef struct tcb_discordiador{
	uint32_t tid;
	char estado;
	uint32_t posicionX;
	uint32_t posicionY;
	//uint32_t proximaInstruccion;
	uint32_t punteroPCB;
	sem_t semaforoTrabajo;
} TCB_DISCORDIADOR;

typedef struct TCBySocket_t {
	int socket;
	TCB_DISCORDIADOR * tripulante;
} TCBySocket;


int proximoTID = 0;
sem_t semaforoTripulantes; 
 
t_list * listaTripulantes;
t_list * listaReady;
t_list * listaBloqueados;

uint32_t iniciarPCB(char*, int);
int conectarImongo();
int conectarMiRAM();
void iniciarPatota(char **);
void listarTripulantes();
void consola();

bool coincideID(TCB_DISCORDIADOR*);

void trabajar();

void pasarTripulante(TCB_DISCORDIADOR *);

void tripulanteVivo(TCB_DISCORDIADOR *);

TCB_DISCORDIADOR * crearTCB(char *, uint32_t); // chequear lo de la lista

void mostrarLista(t_list *); 



#endif
