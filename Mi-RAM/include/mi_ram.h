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
TCB * deserializar_TCB(void *);
int obtenerProximaTarea(int);

sem_t mutexProximoPID; 

// ----------------------------------------  PAGINAS

char * path_SWAP;
t_list * listaFrames;
t_list * listaTablasDePaginas;
int tamanioPagina, tamanioMemoria;
pthread_mutex_t mutexMemoriaPrincipal;
pthread_mutex_t mutexListaTablas;

typedef struct {
	uint32_t inicio;
	uint32_t ocupado;
}  t_frame;

typedef struct {
	uint32_t numeroPagina;
	uint32_t numeroFrame;
	// ultimaReferencia
	// SecondChance
} t_pagina;

int divisionRedondeadaParaArriba(int , int );
int framesDisponibles();
uint32_t buscarFrame();
void iniciarFrames();
void llenarFramesConPatota(t_list *, void *, int , int , int , int );


// ----------------------------------------  SEGMENTOS

t_list * tablaSegmentosGlobal; 
t_list * tablaDeTablasSegmentos;  // va a estar conformado por muchos struct de tipo referenciaTablaPatota
sem_t mutexTablaGlobal;
sem_t mutexTablaDeTablas; 

typedef struct{
	uint32_t pid; 
	uint32_t * tripulantesDeLaPatota;
	uint32_t tamanioTareas;
	t_list * tablaPatota; 
} referenciaTablaPatota; 

typedef struct{
	
	uint32_t base; 
	uint32_t tamanio; 
 
} t_segmento; 

uint32_t asignarMemoriaSegmentacionTCB(void *, t_list *); 
uint32_t asignarMemoriaSegmentacionPCB(void * , t_list *);
uint32_t asignarMemoriaSegmentacionTareas(char * , int , t_list * );
uint32_t encontrarLugarSegmentacion(int );
uint32_t firstFit(int );
uint32_t bestFit(int );
t_list *  obtenerSegmentosLibres(t_list * );
int buscarEspacioSegmentacion(int , int );
bool seEncuentraPrimeroEnMemoria(t_segmento * , t_segmento* );
bool segmentoMasPequenio(t_segmento * , t_segmento * );
bool cabePCB(t_segmento * );
bool cabeTCB(t_segmento * );
void imprimirSegmentosLibres();


// --------------------- Generales

t_log * loggerMiram; 
void servidorPrincipal();

void atenderDiscordiador(int);
void recibir_TCB(int);
PCB * crearPCB();

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
