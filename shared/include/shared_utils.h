
#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <semaphore.h>
#include <inttypes.h>

int crear_conexion(char *, char*);

int crear_conexionServer(char*);


// TCB - Tripulante Control Block -- ocupa 17 bytes (no)

typedef struct tcb{
	uint32_t tid;
	char estado;
	uint32_t posicionX;
	uint32_t posicionY;
	uint32_t proximaInstruccion;
	uint32_t punteroPCB;
} TCB;


// preguntar sobre tipo de datos - deberían ser punteros? ---- es el numero de la direccion logica?

// PCB - Patota Control Block -- ocupa 8 bytes

typedef struct pcb{
	uint32_t pid;
	uint32_t tareas;
} PCB;

enum protocoloMIRAM{
	INICIAR_PATOTA, PEDIR_TAREA, HAY_TAREA, NO_HAY_TAREA
};

typedef struct Dato{
	TCB * tripulante; 
	int header; 
}DatosTripulante;

typedef struct {
	uint32_t size;	// Tamaño del payload
	void * stream; //Payload
}t_buffer;

typedef struct {
	int header;
	t_buffer* buffer;
}t_paquete;

typedef struct{
	char * descripcionTarea;
	int posicionX;
	int posicionY;
	int tiempo;
	int parametro;
	bool tareaTerminada;
} tarea_struct;

//Tareas
typedef enum {
	CONSUMIR_OXIGENO,
	GENERAR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA
}tareasTripulantes;

typedef struct {
	int parametro;
	uint32_t tid;
}t_parametro;

typedef struct {
	PCB * pcb;
	int tamanioTareas;
	char * tareas;
	int cantidadTCB;
	// lista TCBs ???
} PCB_TAREAS_TCB_struct;



#endif