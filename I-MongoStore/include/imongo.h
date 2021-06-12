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

//Log
t_log *loggerImongoStore;

//Conectar al Discordiador
void servidorPrincipal();
void atenderDiscordiador(int);

//Informacion config
char * puertoImongoStore;
uint32_t tamanioDeBloque;
uint32_t cantidadDeBloques;
char * puntoDeMontaje;
char *  tiempoDeSinc;

//Variables del map blocks
char *mapBlocks;
size_t tamanioBlocks;
int archivoBlocks;

//Superbloque
char * ubicacionSuperBloque;
t_bitarray *punteroBitmap;

//---Prueba de hilos, struck del ilo prueba ---//
typedef struct prueba {
	tareasTripulantes tarea;
	int cantidadRecurso;
}prueba;

sem_t semBitArray;

//---------------------- LEE CONFIG -------------------------------------------//
void leerConfig(){ 
	t_config * config = config_create("./cfg/imongo.config");
	
	puntoDeMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	puertoImongoStore = config_get_string_value(config, "PUERTO");
	tiempoDeSinc = config_get_string_value(config, "TIEMPO_SINCRONIZACION");
	tamanioDeBloque = config_get_int_value(config, "BLOCK_SIZE");
    cantidadDeBloques = config_get_int_value(config, "CANTIDAD_BLOCKS");
}
//-------------------------------------------------------------------------------//
//---------------------- BITMAP  -------------------------------------------//

void crearBitMap(){
	size_t sizeBitMap = cantidadDeBloques / 8; 
	char * bitMap = (char *) malloc(sizeBitMap);
	punteroBitmap = bitarray_create(bitMap, sizeBitMap);
	
	for(int i = 0; i<cantidadDeBloques; i++){
		bitarray_clean_bit(punteroBitmap,i);
	}
}

void leerBitMap(){
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");

	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	crearBitMap();
 	fread(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);
}

int bloqueLibreBitMap(){
	int proximoBlock = 0;
	while(bitarray_test_bit(punteroBitmap,proximoBlock))
			proximoBlock++;
	bitarray_set_bit(punteroBitmap,proximoBlock);
	
	return proximoBlock;
}

void guardarBitMap(){
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");
	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);
	fclose(superBloque);
}

void liberarBitMap(){
	free(punteroBitmap->bitarray);
 	bitarray_destroy(punteroBitmap);
}

//-------------------------------------------------------------------------------//
//--------------------------------CREACION DEL FYLESISTEM ----------------------------------//
void mapearBlocks(){
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	archivoBlocks = open(ubicacionBlocks, O_RDWR , S_IRUSR | S_IWUSR);
	mapBlocks = mmap(NULL, tamanioBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBlocks,0);
	free(ubicacionBlocks);
}

void crearBlocks(){
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	FILE *archivoBlocks = fopen(ubicacionBlocks, "w");
	size_t tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	ftruncate(fileno(archivoBlocks), tamanioBlocks);
	fclose(archivoBlocks);
	mapearBlocks();
}

void crearSuperBloque(){
	ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"w");
	
	fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);
	
	fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);

 	crearBitMap();

	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);
}

void crearFileSystem(){
	mkdir(puntoDeMontaje,0777);
	crearSuperBloque();
	crearBlocks();
	mkdir(string_from_format("%s/Files",puntoDeMontaje),0777);
	mkdir(string_from_format("%s/Files/Bitacoras",puntoDeMontaje),0777);
}
//-------------------------------------------------------------------------------//
//------------------------- CREARCION ARCHIVO RECURSO  -------------------------//

t_config *creacionArchivoRecurso(char caracterLlenado, char *ubicacionArchivoRecurso){
	FILE *fileRecurso = fopen(ubicacionArchivoRecurso,"w");
	fclose(fileRecurso);

	t_config *configRecurso = config_create(ubicacionArchivoRecurso);
	 config_set_value(configRecurso,"SIZE",string_itoa(0));
	 config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(0));
	 config_set_value(configRecurso,"BLOCKS","[]");
	 config_set_value(configRecurso,"CARACTER_LLENADO",string_repeat(caracterLlenado,1));
	 config_set_value(configRecurso,"MD5_ARCHIVO","");
	 config_save_in_file(configRecurso,ubicacionArchivoRecurso);
	
	return configRecurso;
}
//------------------------------------------------------------------------------//
//------------------------- CREARCION ARCHIVO BITACORA  ------------------------------------//

t_config *creacionArchivoBitacora(int idTripulante){
	char * ubicacionArchivoBitacora = string_from_format("%s/Files/Bitacoras/Tripulante%d.ims",puntoDeMontaje,idTripulante);
	FILE *fileBitacora= fopen(ubicacionArchivoBitacora,"w");
	fclose(fileBitacora);

	t_config * configBitacora;
	configBitacora = config_create(ubicacionArchivoBitacora);
	 config_set_value(configBitacora,"SIZE",string_itoa(0));
	 config_set_value(configBitacora,"BLOCKS","[]");
	 config_save_in_file(configBitacora,ubicacionArchivoBitacora);
	 return configBitacora;
}
//--------------------------------------------------------------------------------//
//------------------------- USAR FYLESTISTEM ------------------------------------//

int leerUltimoBloque(t_config * configGeneral){

	char *listaBlocks = config_get_string_value(configGeneral,"BLOCKS");
	int proximoBlock;

	if(listaBlocks[1]!=']'){ 
		int indiceFinal = strlen(listaBlocks);
		while(listaBlocks[indiceFinal]!=',' && indiceFinal>0)
			indiceFinal--;
		proximoBlock = atoi(string_substring_from(listaBlocks,indiceFinal+1)) - 1;
	}else{
		proximoBlock = bloqueLibreBitMap();
	}
	return proximoBlock;
}

//------------------------------------------------------------------------------//
//--------------------- CODIGO DE LOS BLOCKS DE RECURSOS  ---------------------//

void agregarBloqueAlFile(int nuevoBloque, t_config * configRecurso){
	int nuevaCantidadBloques = config_get_int_value(configRecurso,"BLOCK_COUNT");
	nuevaCantidadBloques++;
	config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(nuevaCantidadBloques));

	char *listaBlocks = config_get_string_value(configRecurso,"BLOCKS");
	char *nuevaListablocks = string_substring_until(listaBlocks,strlen(listaBlocks)-1);

	if(listaBlocks[1]!=']')
		string_append(&nuevaListablocks, ",");
	
	string_append(&nuevaListablocks, string_itoa(nuevoBloque + 1));
	string_append(&nuevaListablocks, "]");
	config_set_value(configRecurso,"BLOCKS",nuevaListablocks);
}

 
void llenarBlocks(char caracterLlenado, int cantLlenar, char *mapBlocksAux, t_config *configRecurso){
	int cantAux = cantLlenar;
	int proximoBlock = leerUltimoBloque(configRecurso);

	int nuevaSize = config_get_int_value(configRecurso,"SIZE");
	nuevaSize+=cantLlenar;
	config_set_value(configRecurso, "SIZE", string_itoa(nuevaSize));
	config_save(configRecurso);
	
	while(cantAux>0){
		
		int cant = proximoBlock * tamanioDeBloque;
		
		if(mapBlocksAux[cant]=='\0')
			agregarBloqueAlFile(proximoBlock, configRecurso);

		for(int i=0; i<cantAux && i<tamanioDeBloque;i++){
			int j = cant + i;
			if(mapBlocksAux[j]!=caracterLlenado)
				mapBlocksAux[j]=caracterLlenado;
			else
				cantAux++;
		}

		cantAux-=tamanioDeBloque;

		if(cantAux>0)
			proximoBlock = bloqueLibreBitMap();
	}

	guardarBitMap();
}

void borrarUltimoBloque(t_config *configRecurso){
	int nuevaCantidadBloques = config_get_int_value(configRecurso,"BLOCK_COUNT");
	nuevaCantidadBloques--;
	config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(nuevaCantidadBloques));

	char *listaBlocks = config_get_string_value(configRecurso,"BLOCKS");
	int indiceFinal = strlen(listaBlocks)-1;
	while(listaBlocks[indiceFinal]!=',' && indiceFinal>0)
			indiceFinal--;
	
	if(indiceFinal>0){
		listaBlocks = string_substring_until(listaBlocks, indiceFinal);
		string_append(&listaBlocks, "]");
	} else {
		listaBlocks = "[]";
	}
	config_set_value(configRecurso,"BLOCKS",listaBlocks);
}

void vaciarBlocks(char caracterVaciado, int cantAVaciar, char *mapBlocksAux, t_config *configRecurso){
	int cantAux = cantAVaciar;
	
	int nuevaSize = config_get_int_value(configRecurso,"SIZE");
	nuevaSize-=cantAVaciar;
	config_set_value(configRecurso, "SIZE", string_itoa(nuevaSize));
	config_save(configRecurso);

	int proximoBlock = leerUltimoBloque(configRecurso);

	while(cantAux>0){
		int cant = proximoBlock * proximoBlock;
		
		for(int i = tamanioDeBloque; cantAux>0 && i>0; i--){
			int j = cant + i - 1;
			
			if(mapBlocksAux[j]==caracterVaciado){
				mapBlocksAux[j]='\0';
				cantAux--;
			}
		}

		if(mapBlocksAux[cant]=='\0'){
			bitarray_clean_bit(punteroBitmap,proximoBlock);
			borrarUltimoBloque(configRecurso);
		}
	}

	guardarBitMap();
}

//-------------------------------------------------------------------------------//

//--------------------- CODIGO DE LOS BLOCKS DE BITACORAS  ----------------------------------------//

void agregarBloqueABitacora(int nuevoBloque, t_config * configBitacora){
	char *listaBlocksBitacora = config_get_string_value(configBitacora,"BLOCKS");
	char *nuevaListablocks = string_substring_until(listaBlocksBitacora,strlen(listaBlocksBitacora)-1);

	if(listaBlocksBitacora[1]!=']')
		string_append(&nuevaListablocks, ",");
	
	string_append(&nuevaListablocks, string_itoa(nuevoBloque + 1));
	string_append(&nuevaListablocks, "]");
	config_set_value(configBitacora,"BLOCKS",nuevaListablocks);
	config_save(configBitacora);
}

void llenarBlocksBitcoras(char * informacionDeLenado, t_config * configBitacora, char *mapBlocksAux){
	int cantLlenado = 0;
	int cantFaltante = strlen(informacionDeLenado);

	int proximoBlock = leerUltimoBloque(configBitacora);
	
	int nuevaSize = config_get_int_value(configBitacora,"SIZE");
	nuevaSize+=cantFaltante;
	config_set_value(configBitacora, "SIZE", string_itoa(nuevaSize));
	config_save(configBitacora);

	while(cantFaltante>0){
		int marcador = proximoBlock * tamanioDeBloque;

		if(mapBlocksAux[marcador]=='\0')
			agregarBloqueABitacora(proximoBlock, configBitacora);

		int limite = marcador+tamanioDeBloque;
		while(mapBlocksAux[marcador] !='\0' && marcador < limite)
			marcador++;

		for(int i=marcador; 0<cantFaltante && i<limite;i++){
			mapBlocksAux[i] = informacionDeLenado[cantLlenado];
			cantFaltante--;
			cantLlenado++;
		}

		if(cantFaltante>0)
			proximoBlock = bloqueLibreBitMap();
	}

	guardarBitMap();
}

//-------------------------------------------------------------------------------//
//------------------------------MENSAJES QUE RECIBE -------------------------------------------//

void generarRecurso(char *recurso, int cantidadALlenar, uint32_t *idTripulante, char *mapBlocksAux){
	log_info(loggerImongoStore,string_from_format("Tripulante %d genera %d de %s",*idTripulante, cantidadALlenar, recurso));
	
	t_config *configBitacora = creacionArchivoBitacora(*idTripulante);
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea Generar %s\n",recurso),configBitacora,mapBlocksAux);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);
	t_config *configRecurso;

	if(access(ubicacionArchivoRecurso, F_OK ))
		configRecurso = creacionArchivoRecurso(recurso[0], ubicacionArchivoRecurso);
	else
		configRecurso = config_create(ubicacionArchivoRecurso);

	llenarBlocks(recurso[0], cantidadALlenar, mapBlocksAux, configRecurso);

	llenarBlocksBitcoras(string_from_format("Se finaliza la tarea Generar %s\n",recurso),configBitacora,mapBlocksAux);

	config_save(configRecurso);
	config_destroy(configRecurso);
	config_save(configBitacora);
	config_destroy(configBitacora);
	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,string_from_format("Se genero %d de %s",cantidadALlenar, recurso));
}

bool consumirRecurso(char *recurso, int cantidadAConsumir, uint32_t *idTripulante, char *mapBlocksAux){
	log_info(loggerImongoStore,string_from_format("Tripulante %d consume %d de %s",*idTripulante,cantidadAConsumir,recurso));
	
	t_config *configBitacora = creacionArchivoBitacora(*idTripulante);
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea Consumir %s\n",recurso),configBitacora,mapBlocksAux);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);
	t_config *configRecurso;

	if(access(ubicacionArchivoRecurso, F_OK )){
		log_info(loggerImongoStore,"No hay oxigeno que consumir");
		return false;
	}

	configRecurso = config_create(ubicacionArchivoRecurso);
	vaciarBlocks(recurso[0], cantidadAConsumir, mapBlocksAux, configRecurso);

	llenarBlocksBitcoras(string_from_format("Se finaliza la tarea Consumir %s\n",recurso),configBitacora,mapBlocksAux);

	config_save(configRecurso);
	config_destroy(configRecurso);
	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,string_from_format("Se consumio %d de %s",cantidadAConsumir,recurso));
	return true;
}

void descartarBasura(uint32_t *idTripulante, char *mapBlocksAux){
	log_info(loggerImongoStore,string_from_format("Tripulante %d descarta Basura",*idTripulante));
	
	t_config *configBitacora = creacionArchivoBitacora(*idTripulante);
	llenarBlocksBitcoras("Comienza la ejecucion de la tarea Descartar Basura\n",configBitacora,mapBlocksAux);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/Basura.ims",puntoDeMontaje);
	t_config *configRecurso = config_create(ubicacionArchivoRecurso);
	int cantidadBasura = config_get_int_value(configRecurso, "SIZE");
	vaciarBlocks('B', cantidadBasura, mapBlocksAux, configRecurso);

	llenarBlocksBitcoras("Se finaliza la tarea Descartar Basura\n",configBitacora,mapBlocksAux);

	config_save(configRecurso);
	config_destroy(configRecurso);
	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,"Se descarto la Basura");
}

//-------------------------------------------------------------------------------//

//--------------------------------ATENDER DISCORDIADOR--------------------------------//
void servidorPrincipal() {
	int listeningSocket = crear_conexionServer(puertoImongoStore);
	int socketCliente;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	pthread_t receptorDiscordiador;


	while(1){
		socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
		if(socketCliente == -1){
			printf("Error en la conexión");
		} else {
			pthread_create(&receptorDiscordiador, NULL, atenderDiscordiador, socketCliente);
		}
	}

	close(socketCliente);
	close(listeningSocket);
}


void atenderDiscordiador(int socketCliente){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	t_parametro * parametroS = malloc(sizeof(int)); 

	int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);

	int tamanioPAQUETE_RECV = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	int PAQUETE_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size,0);

	memcpy(&(parametroS->parametro), paquete->buffer->stream, sizeof(int));
	paquete->buffer->stream += sizeof(int);

	char *mapBlocksAux = malloc(tamanioBlocks);
	memcpy(mapBlocksAux, mapBlocks, tamanioBlocks);

	uint32_t * tid = malloc(sizeof(uint32_t));
	tid = &parametroS->tid;   // ID TRIPULANTE
	
	switch (paquete->header)
	{
	case GENERAR_OXIGENO:
		generarRecurso("Oxigeno",parametroS->parametro,tid,mapBlocksAux);
		break;
	case CONSUMIR_OXIGENO:
		consumirRecurso("Oxigeno",parametroS->parametro,tid,mapBlocksAux);
		break;
	case GENERAR_COMIDA:
		generarRecurso("Comida",parametroS->parametro,tid,mapBlocksAux);
		break;
	case CONSUMIR_COMIDA:
		consumirRecurso("Comida", parametroS->parametro,tid,mapBlocksAux);
		break;
	case GENERAR_BASURA:
		generarRecurso("Basura",parametroS->parametro,tid,mapBlocksAux);
		break;
	case DESCARTAR_BASURA:
		descartarBasura(tid,mapBlocksAux);
		break;

	default:
		break;
	}

	memcpy(mapBlocks, mapBlocksAux, tamanioBlocks);
	msync(mapBlocks, tamanioBlocks, MS_SYNC);
	free(mapBlocksAux);

}

#endif