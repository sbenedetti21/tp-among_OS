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


#define BACKLOG 20
#define SIZEOF_PCB 8
#define SIZEOF_TCB 21


//Estructuras generales

//logger
t_log * loggerMiram; 
t_log * loggerSegmentacion;

//archivo de configuracion
char * esquemaMemoria;
char * algoritmoReemplazo;
char * puertoMemoria;
char * criterioSeleccion; 
char * pathSwap; 
void * memoriaPrincipal;
int		tamanioSwap; 
int 	tamanioMemoria; 
int		tamanioPagina; 


//mutex para valores compartidos 
pthread_mutex_t mutexTablaSegmentosGlobal; 
pthread_mutex_t mutexListaReferenciasPatotas; 
pthread_mutex_t mutexMemoriaPrincipal; 
pthread_mutex_t mutexCompactacion; //(ver si este es necesario o se puede obviar aplicando todos los otros mutex); 
sem_t semaforoCompactacion; 

//-----------------Segmentacion -----------

//variables globales
t_list * tablaSegmentosGlobal; 
t_list * listaReferenciasPatotaSegmentacion; 
bool compactacion = false; 
int contadorHilos = 0; 


//Estructuras necesarias


enum tipoSegmento{
	SEG_TAREAS, SEG_TCB, SEG_PCB
};

typedef struct{
	int tid;  
	int base; 
	int tamanio;
	int tipoSegmento; 
	bool ocupado; 
	int pid; 
} t_segmento; 


typedef struct{
	int pid; 
	t_list * tablaPatota;
	pthread_mutex_t semaforoPatota;  
} referenciaTablaPatota; 


void iniciarMemoria();
void servidorPrincipal();
void * atenderDiscordiador();
PCB * crearPCB(uint32_t);
void mandarPaqueteSerializado(t_buffer * , int , int );
int buscarEspacionNecesario(int, int);
void * hiloSIGUSR1(); 
void * hiloSIGUSR2();
void sig_handler(int); 

//funciones segmentacion
int buscarEspacioNecesarioSegmentacion(int , int );
uint32_t buscarSegmentoLibre(int );
uint32_t bestFitSegmentacion(int );
uint32_t firstFitSegmentacion(int );
uint32_t asignarMemoriaSegmentacionTCB(void * , int , referenciaTablaPatota * , int );
uint32_t asignarMemoriaSegmentacionPCB(void *  , referenciaTablaPatota * );
uint32_t asignarMemoriaSegmentacionTareas(char * , int , referenciaTablaPatota * , int );
void actualizarPosicionTripulanteSegmentacion(uint32_t , uint32_t , uint32_t , uint32_t );
void actualizarEstadoTripulanteSegmentacion(uint32_t , uint32_t , char );
void actualizarDireccionesTareasTCB(t_segmento *, uint32_t, uint32_t); 
void actualizarEstructurasSegmentacion(t_segmento *, uint32_t);
void eliminarTripulanteSegmentacion(uint32_t , uint32_t );
void esElUltimoTripulante(uint32_t );
char * obtenerProximaTareaSegmentacion(uint32_t , uint32_t );
void actualizarProximaTareaSegmentacion(uint32_t , uint32_t );
void compactarMemoriaSegmentacion();
void dumpMemoriaSegmentacion();
uint32_t obtenerDireccionTripulanteSegmentacion(uint32_t , uint32_t );
bool segmentoLibre(t_segmento * );
bool segmentoOcupado(t_segmento * );
bool segmentoMasPequenio(t_segmento * , t_segmento * );
bool seEncuentraPrimeroEnMemoria(t_segmento * , t_segmento* );



//--------------- MAPA ---------------------
// NIVEL* navePrincipal;
// sem_t semaforoTerminarMapa; 
// sem_t semaforoMoverTripulante;
// void iniciarMapa();
// void agregarTripulanteAlMapa(uint32_t, uint32_t ,uint32_t);
// void moverTripulanteEnMapa(uint32_t, uint32_t , uint32_t );
// void expulsarTripulanteDelMapa(uint32_t);
// char idMapa(uint32_t);
 
/*
// --------------------------------------- MEMORIA GENERAL
 
char * esquemaMemoria;
char * algoritmoReemplazo;
char * puertoMemoria;
void * memoriaPrincipal; 
char * criterioSeleccion; 

int buscarEspacioNecesario(int, int);
TCB * deserializar_TCB(void *);
char * obtenerProximaTareaSegmentacion(uint32_t, uint32_t);
uint32_t obtenerDireccionTripulante(uint32_t, uint32_t );
uint32_t obtenerDireccionProximaTarea(uint32_t);
char * obtenerProximaTarea(uint32_t, uint32_t);
void imprimirSegmentos();
void sig_handler(uint32_t senial);
void * hiloSIGUSR1();
void * hiloSIGUSR2();
void actualizarEstadoTripulante(uint32_t , uint32_t , char );
void eliminarTripulante(uint32_t, uint32_t);

bool compactacion = false; 



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
	int contadorTCB;
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
void iniciarFrames();
void llenarFramesConPatota(t_list *, void *, int , int , int , int , uint32_t);
void llevarPaginaASwap();
t_frame * buscarFrame();
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
sem_t mutexSegmentosLibres;

t_list * tripulantesPatotas; 


enum tipoSegmento {
	SEG_TAREAS, SEG_PCB, SEG_TCB
};

typedef struct{
	uint32_t pid;
	t_list * tablaPatota;
} referenciaTablaPatota;

typedef struct {
	uint32_t pid;
	uint32_t tid;
} referenciaTripulante;

typedef struct{
	
	int tipoSegmento; 
	int pid;
	int tid; 
	uint32_t base; 
	uint32_t tamanio; 
 
} t_segmento; 

t_list * tablaSegmentosLibres;
uint32_t asignarMemoriaSegmentacionTCB(void *, int, t_list *, int); 
uint32_t asignarMemoriaSegmentacionPCB(void * , t_list *);
uint32_t asignarMemoriaSegmentacionTareas(char * , int , t_list * , int);
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
void actualizarReferenciasTablas(t_segmento *, uint32_t);


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
 */


#endif
