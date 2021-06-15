#ifndef MI_RAM_H
#define MI_RAM_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include "shared_utils.h"

#define BACKLOG 10 //TODO
#define SIZEOF_PCB 8
#define SIZEOF_TCB 21

// --------------------------------------- MEMORIA GENERAL

char * esquemaMemoria;
char * algoritmoReemplazo;
char * puertoMemoria;
void * memoriaPrincipal; 

int buscarEspacioNecesario(int, int);

// ----------------------------------------  PAGINAS

t_list * listaFrames;
t_list * listaTablasDePaginas;
int tamanioPagina, tamanioMemoria;
char * path_SWAP;

typedef struct {
	uint32_t inicio;
	uint32_t ocupado;
}  t_frame;

t_list * listaFrames;

typedef struct {
	uint32_t numeroPagina;
	uint32_t numeroFrame;
	// ultimaReferencia
	// SecondChance
} t_pagina;

int divisionRedondeadaParaAriba(int , int );
int framesDisponibles();
uint32_t buscarFrame();
void iniciarFrames();


// ----------------------------------------  SEGMENTOS

t_list * tablaSegmentos; 
typedef struct{
	uint32_t numeroSegmento; 
	uint32_t direccionBase; 
	uint32_t tamanio; 
// no se si va aca -- 	void * contenido; 
} segmento; 

t_log * loggerMiram; 
void servidorPrincipal();

void atenderDiscordiador(int);
void recibir_TCB(int);
PCB * crearPCB();
uint32_t asignarMemoria(void *); 
uint32_t asignarMemoriaTareas(char *); 
uint32_t asignarMemoriaSegmentacion(void *);
uint32_t asignarMemoriaTareasSegmentacion(char *);
//TCB * deserializar_TCB(void *);

typedef struct {

	int socket;
	struct sockaddr_in address;
	socklen_t addresslength; 

 
} structConexion;


int proximoPID = 0; 

//--------------- MAPA ---------------------
void iniciarMapa();
void agregarTripulanteAlMapa(TCB*);
void moverTripulanteEnMapa(TCB *, int , int );
void expulsarTripulanteDelMapa(TCB*);
 
#endif
