#ifndef MI_RAM_H
#define MI_RAM_H
#define BACKLOG 10 //TODO

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include "shared_utils.h"

char * esquemaMemoria; 

// ----------------------------------------  PAGINAS

typedef struct {
	void * contenido;
} frame;

typedef struct {
	uint32_t numeroPagina;
	uint32_t numeroFrame;
	// ultimaReferencia
} paginaStruct;


// ----------------------------------------  SEGMENTOS

t_list * tablaSegmentos; 
typedef struct{
	uint32_t numeroSegmento; 
	uint32_t direccionBase; 
	uint32_t tamanio; 
// no se si va aca -- 	void * contenido; 
} segmento; 

t_log * loggerMiram; 
void servidorPrincipal(t_config*);

void atenderDiscordiador(int);
void recibir_TCB(int);
PCB * crearPCB();
uint32_t asignarMemoria(void *); 
uint32_t asignarMemoriaTareas(char *); 
uint32_t asignarMemoriaPaginacion(void *);
uint32_t asignarMemoriaSegmentacion(void *);
uint32_t asignarMemoriaTareasPaginacion(char *);
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
