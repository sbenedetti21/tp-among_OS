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
#include <curses.h>
#define ASSERT_CREATE(nivel, id, err)                                                   \
    if(err) {                                                                           \
        nivel_destruir(nivel);                                                          \
        nivel_gui_terminar();                                                           \
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(err));  \
        return EXIT_FAILURE;                                                            \
    }


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
uint32_t obtenerDireccionTripulante(uint32_t, uint32_t );
uint32_t obtenerDireccionProximaTarea(uint32_t);
char * obtenerProximaTarea(uint32_t, uint32_t);


// ----------------------------------------  PAGINAS

int contadorLRU;

char * path_SWAP;
t_list * listaFrames;
t_list * listaFramesSwap;
t_list * listaTablasDePaginas;
t_list * listaTripulantes;
int tamanioPagina, tamanioMemoria, tamanioSwap;
pthread_mutex_t mutexMemoriaPrincipal;
pthread_mutex_t mutexListaTablas;
pthread_mutex_t mutexListaFrames;
pthread_mutex_t mutexListaFramesSwap;
pthread_mutex_t mutexTareas;
pthread_mutex_t mutexContadorLRU;

typedef struct {
	uint32_t numeroPagina;
	uint32_t numeroFrame;
	uint32_t pid;
	int bitDeValidez; // si esta en 0, esta en swap
	int ultimaReferencia;
	int bitDeUso;
} t_pagina;

typedef struct {
	uint32_t inicio;
	uint32_t ocupado;
	t_pagina * pagina;
	
}  t_frame;


typedef struct {
	int longitudTareas;
	uint32_t pid;
	t_list * listaPaginas;
} referenciaTablaPaginas;

typedef struct {
	uint32_t tid;
	uint32_t pid;
	int nroPagina;
	int offset;
	int cantidadDePaginas;
} t_tripulantePaginacion;

char * obtenerProximaTareaPaginacion(referenciaTablaPaginas* , uint32_t);
uint32_t obtenerDireccionProximaTareaPaginacion(void *);
uint32_t obtenerDireccionFrame(referenciaTablaPaginas *, uint32_t);
char * encontrarTareasDeTripulanteEnStream(void *, t_tripulantePaginacion *, referenciaTablaPaginas*);
void actualizarPunteroTarea(t_tripulantePaginacion *, t_list*, int);

int divisionRedondeadaParaArriba(int , int );
int framesDisponibles();
int framesDisponiblesSwap();
uint32_t buscarFrame();
void iniciarFrames();
void llenarFramesConPatota(t_list *, void *, int , int , int , int , uint32_t);
void llevarPaginaASwap();
t_frame * seleccionarVictima();
t_frame * buscarFrameSwap();

void dumpDeMemoriaPaginacion();


// ----------------------------------------  SEGMENTOS

t_list * tablaSegmentosGlobal; 
t_list * tablaDeTablasSegmentos;  // va a estar conformado por muchos struct de tipo referenciaTablaPatota
sem_t mutexTablaGlobal;
sem_t mutexTablaDeTablas; 
sem_t mutexTripulantesPatotas; 
sem_t mutexCompactacion; 
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
void compactarMemoria();


// --------------------- Generales

t_log * loggerMiram; 
void servidorPrincipal();

void atenderDiscordiador(int);
void recibir_TCB(int);
PCB * crearPCB(uint32_t);

typedef struct {

	int socket;
	struct sockaddr_in address;
	socklen_t addresslength; 

 
} structConexion;

void mandarPaqueteSerializado(t_buffer *, int, int);


//--------------- MAPA ---------------------
NIVEL* navePrincipal;
sem_t semaforoTerminarMapa; 
sem_t semaforoMoverTripulante;
void iniciarMapa();
void agregarTripulanteAlMapa(uint32_t, uint32_t ,uint32_t);
void moverTripulanteEnMapa(uint32_t, uint32_t , uint32_t );
void expulsarTripulanteDelMapa(uint32_t);
char idMapa(uint32_t);

#endif
