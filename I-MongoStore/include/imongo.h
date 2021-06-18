#ifndef imongo_H
#define imongo_H
#define BACKLOG 5 //TODO

#include "shared_utils.h"
#include <commons/txt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

//para borrar fileSistem
	#include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <sys/types.h>
    

//Semaforo
sem_t semaforoBloques;
sem_t semaforoArchivoRecurso;
sem_t semaforoOxigeno, semaforoBasura, semaforoComida;

//Log
t_log *loggerImongoStore;

//Preguntar si crearFileSistem
//void preguntarFileSystem(int);
//Iniciar Semaforos
//void inicializarSemaforos();
//void cerrarSemaforos();

//Conectar al Discordiador
void servidorPrincipal();
void atenderDiscordiador(int);

//Informacion config
char * puertoImongoStore;
uint32_t tamanioDeBloque;
uint32_t cantidadDeBloques;
char * puntoDeMontaje;
int tiempoDeSinc;

//Variables del map blocks
char *mapBlocks;
size_t tamanioBlocks;
int archivoBlocks;
char *mapBlocksCopia;

//Superbloque
char * ubicacionSuperBloque;
t_bitarray *punteroBitmap;

//---Prueba de hilos, struck del ilo prueba ---//
typedef struct prueba {
	tareasTripulantes tarea;
	int cantidadRecurso;
}prueba;



//----FUNCIONES----//

//leer configuracion
void leerConfig();

//creacion y utilizacion del FileSystem
void crearFileSystem();
void leerFileSystem();
void inicializarFileSystem(int);
//borar fileSSYtem existente
void borrarFileSystem(const char *);

//creacion y utilizacion del Bitmap
void crearBitmap();
void leerBitMap();
int bloqueLibreBitMap();
void guardarBitMap();
void liberarBitMap();

//creacion y utilizacion del SuperBloque
void crearSuperBloque();

//creacion y utilizacion de los bloques
void mapearBlocks();
void crearBlocks();
void llenarBlocks(char *, int , char *);
void vaciarBlocks(char , int , char *, t_config *);
void borrarUltimoBloque(t_config *);
int leerUltimoBloque(t_config *);

//creacion y utilizacion de Files
void agregarBloqueAlFile(int, char *);
void agregarSizeFile(char *, int );
int ultimoBloqueFile(char *);

// creacion y utilizacion de Archivo Recurso
void creacionArchivoRecurso(char , char *);
void generarRecurso(char *, int, uint32_t , char *);
bool consumirRecurso(char *, int , uint32_t , char *);
void descartarBasura(uint32_t , char *);
void agregarSize(t_config *, int);
void agregarBloqueAlArchivo(int, t_config * );

//creacion y eliminacion de semaforos en los recursos
void semaforoEsperaRecurso(char *);
void semaforoListoRecurso(char *);

// creacion y utilizacion de Archivo Bitacora
t_config *archivoBitacora(int);
void llenarBlocksBitcoras(char * , t_config * , char *);

// utilizacion del MSYNC (tiempo de sincronizacion) 
void sincronizacionMapBlocks();

//atender las peticiones del discordiador
void servidorPrincipal();
void atenderDiscordiador(int);

#endif