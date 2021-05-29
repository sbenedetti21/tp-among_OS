#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

typedef struct TCBySocket_t {
	int socket;
	TCB * tripulante;
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

bool coincideID(TCB*);

void trabajar();

void pasarTripulante(TCB *);

void tripulanteVivo(TCB *);

TCB * crearTCB(char *, uint32_t); // chequear lo de la lista

void mostrarLista(t_list *); 


#endif
