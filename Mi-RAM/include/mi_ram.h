#ifndef MI_RAM_H
#define MI_RAM_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/memory.h>
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
char * obtenerProximaTareaSegmentacion(uint32_t, uint32_t);
uint32_t obtenerDireccionTripulante(uint32_t );
uint32_t obtenerDireccionProximaTarea(uint32_t);
char * obtenerProximaTarea(uint32_t);

sem_t mutexProximoPID; 

// ----------------------------------------  PAGINAS

char * path_SWAP;
t_list * listaFrames;
t_list * listaTablasDePaginas;
int tamanioPagina, tamanioMemoria;
pthread_mutex_t mutexMemoriaPrincipal;
pthread_mutex_t mutexListaTablas;
pthread_mutex_t mutexListaFrames;
pthread_mutex_t mutexTareas;


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

typedef struct {
	uint32_t idTripulante;
	uint32_t idPatota;
	uint32_t longitudTareas;
} t_tripulanteConPID;
t_list * listaTripulantes;

char * obtenerProximaTareaPaginacion(int, int, int);
char * encontrarTareasDeTripulanteEnStream(void *, int, int, int);
void actualizarPunteroTarea(int, int, int);

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
sem_t mutexTripulantesPatotas; 
t_list * tripulantesPatotas; 


typedef struct{
	uint32_t pid;
	t_list * tablaPatota;
} referenciaTablaPatota;

typedef struct {
	uint32_t pid;
	uint32_t tid;
} referenciaTripulante;

typedef struct{
	
	int tid; 
	uint32_t base; 
	uint32_t tamanio; 
 
} t_segmento; 


uint32_t asignarMemoriaSegmentacionTCB(void *, int, t_list *); 
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
void actualizarProximaTarea(uint32_t, uint32_t);


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

void mandarPaqueteSerializado(t_buffer *, int, int);
int proximoPID = 0; 

//--------------- MAPA ---------------------
void iniciarMapa();
void agregarTripulanteAlMapa(TCB*);
void moverTripulanteEnMapa(TCB *, int , int );
void expulsarTripulanteDelMapa(TCB*);
 
#endif
