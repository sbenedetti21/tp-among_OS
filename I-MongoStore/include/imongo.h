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

//signal 
#include <signal.h>


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
char **posicionesSabotaje;

//Variables de mapeo del Blocks
size_t tamanioBlocks;
char *mapBlocks;
int archivoBlocks;
char *mapBlocksCopia;

//Variables del mapeo del SuperBloque
size_t tamanioSuperBloqueBlocks;
char *mapSuperBloque;
int archivoSuperBloque;
//char *ubicacionSuperBloque;

//Superbloque
t_bitarray *punteroBitmap;
size_t tamanioBitMap;

//-----------FUNCIONES-----------//
//LEE CONFIG
void leerConfig();

//ARCHIVO BLOCKS Y SUPERBLOQUE
void crearBlocks();
void mapearBlocks();
void crearSuperBloque();
void mapearSuperBloque();

//USO DEL FILESYSTEM
void crearFileSystem();
void leerSuperBloque();
void leerFileSystem();
void inicializarFileSystem(int);
void borrarFileSystem(const char *);

//USO DEL BITMAP
void crearBitmap();
void guardarBitMap();
int bloqueLibreBitMap();
void liberarBloqueBitMap(int);
void liberarBitMap();

//CODIGO DE LOS FILES Y BITACORAS
int leerUltimoBloque(t_config *);
void agregarBloqueArchivo(int, t_config * );
void agregarSizeArchivo(t_config *, int);

//CREARCION ARCHIVO BITACORA
t_config *crearArchivoBitacora(int);

//CODIGO DE LOS BLOCKS DE BITACORAS
void llenarBlocksBitcoras(char *, uint32_t);

//SEMAFOROS PARA LOS FILES DE RECURSOS
void semaforoEsperaRecurso(char *);
void semaforoListoRecurso(char *);

//CODIGO DE LOS FILES DE RECURSOS
void creacionFileRecurso(char , char *);
void agregarBloqueFile(int, char *);
void agregarSizeFile(char *, int );
int ultimoBloqueFile(char *);
int conseguirSizeBlocks(char *);
void borrarBloqueFile(char *, int);

//CODIGO MD5
char *generarMD5(char *);
void actualizarMD5(char * );

//CODIGO DE LOS BLOCKS DE RECURSOS
void llenarBlocksRecursos(char *, int);
void vaciarBlocksRecursos(char *, int);

//MENSAJES QUE RECIBE
void generarRecurso(char *, int, uint32_t);
bool consumirRecurso(char *, int , uint32_t);
void descartarBasura(uint32_t);
char *conseguirStringBlocks(int);
char * conseguirBitacora(uint32_t);

//SABOTAJES
void pruebaDeSabotaje();
int verificarBlocksBitMap(char *);
int verificarBitacoraBitMap();
bool sabotajeSuperBloque();
int llenarBloqueConRecurso(char, int, int);
bool verificarListBlocks(char *);
bool verificarBlockCount(char *);
int reemplazarSizeBloque(int);
bool verificarSize(char *);
bool sabotajeFile();

//SIGNAL
void llegoElSignal();

//SINCRONIZACION BLOCKS
void sincronizacionMapBlocks();

//atender las peticiones del discordiador
void serializarYMandarPosicionSabotaje(uint32_t, uint32_t);
int socketParaSabotajes;
void servidorPrincipal();
void deserealizarPosicion(t_paquete*);
void atenderDiscordiador(int);
uint32_t deserializarPedidoBitacora(t_paquete*);
void serializarYMandarBitacora(char *, int);
void deserializarNuevaPosicion(t_paquete*);
void deserializarTareaIO(t_paquete*);
void deserializarTerminoTarea(t_paquete *);
void deserializarInicioTareaNormal(t_paquete *);
void deserializarTripulanteFSCK(t_paquete*);


#endif