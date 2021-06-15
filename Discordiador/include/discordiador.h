#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

// /home/utnso/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt

//flag para liberar puerto 

typedef struct tcb_discordiador{
	uint32_t tid;
	char estado;
	uint32_t posicionX;
	uint32_t posicionY;
	sem_t semaforoTrabajo;
	uint32_t pid;
} TCB_DISCORDIADOR;


typedef struct TCBySocket_t {
	int socket;
	TCB_DISCORDIADOR * tripulante;
} TCBySocket;


int proximoTID = 0;
sem_t semaforoTripulantes; 
sem_t consultarSiHayVacios;
sem_t esperarAlgunTripulante; 
sem_t consultarSiHayVacios2;

t_log * loggerDiscordiador; 
t_list * listaTripulantes;
t_list * listaReady;
t_list * listaBloqueados;
t_list * listaTrabajando;


uint32_t iniciarPCB(char*, int);
int conectarImongo();
int conectarMiRAM();
void iniciarPatota(char **);
void listarTripulantes();
void consola();
void mandarPaqueteSerializado(t_buffer*,int, int);
void serializarYMandarPCB(char*,int,int, t_list *);
void serializarYMandarTarea(int, tareasTripulantes, uint32_t);
void gestionarTarea(tarea_struct * , uint32_t);


bool coincideID(TCB_DISCORDIADOR*);

void ponerATrabajar();
void trasladarseA(int,int,TCB_DISCORDIADOR*);

void pasarTripulante(TCB_DISCORDIADOR *);

void tripulanteVivo(TCB_DISCORDIADOR *);

TCB_DISCORDIADOR * crearTCB(char *); // chequear lo de la lista

void mostrarLista(t_list *); 

char * leerTareas(char *);

#endif