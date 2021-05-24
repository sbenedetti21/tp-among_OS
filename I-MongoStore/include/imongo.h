#ifndef imongo_H
#define imongo_H
#define BACKLOG 5 //TODO

#include "shared_utils.h"
#include <commons/txt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <commons/bitarray.h>
#include <fcntl.h>

char * puertoImongoStore;
uint32_t tamanioDeBloque;
uint32_t cantidadDeBloques;
char * puntoDeMontaje;
char *elBlocks;

void leerConfig(){
    
	t_config * config = config_create("./cfg/imongo.config");
	
	puntoDeMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	puertoImongoStore = config_get_string_value(config, "PUERTO");//no importa??
//  char *  tiempoDeSinc = config_get_string_value(config, "TIEMPO_SINCRONIZACION");
	tamanioDeBloque = config_get_int_value(config, "BLOCK_SIZE");
    cantidadDeBloques = config_get_int_value(config, "CANTIDAD_BLOCKS");
        

}

void liberarBitMap(t_bitarray *punteroBitmap){
	free(punteroBitmap->bitarray);
 	bitarray_destroy(punteroBitmap);
}

t_bitarray *crearBitMap(){
	size_t sizeBitMap = cantidadDeBloques / 8; 
	char * bitMap = (char *) malloc(sizeBitMap);
	t_bitarray * punteroBitmap = bitarray_create(bitMap, sizeBitMap);
	
	for(int i = 0; i<cantidadDeBloques; i++){
		bitarray_clean_bit(punteroBitmap,i);
	}

	return punteroBitmap;
}

int leerBitMap(){
	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");

	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	t_bitarray * punteroBitmap = crearBitMap();
 	fread(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	int bloqueLibre = 0;
	while(bitarray_test_bit(punteroBitmap,bloqueLibre))
		bloqueLibre++;

	bitarray_set_bit(punteroBitmap,bloqueLibre);

	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	liberarBitMap(punteroBitmap);
	fclose(superBloque);	
	return bloqueLibre;
}

void crearSuperBloque(){
//crear bloque y tamanio
	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"w");
	
	fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);
	
	fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);

//crear bitmap
 	t_bitarray *punteroBitmap = crearBitMap();
/*
	bitarray_set_bit(punteroBitmap,0); //Cambia de 0 a 1 el primer bloque
	bitarray_set_bit(punteroBitmap,15);	//Cambia de 0 a 1 el ultimo bloque
	bitarray_set_bit(punteroBitmap,5);	//Cambia de 0 a 1 el sexto bloque
*/	
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);
	liberarBitMap(punteroBitmap);
}

void mapearBlocks(){
	//Usar mmap() para acceder al archivo block
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	size_t tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	int archivoBlocks = open(ubicacionBlocks, O_RDWR , S_IRUSR | S_IWUSR);
	elBlocks = mmap(NULL, tamanioBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBlocks,0);
}

void llenarBlocks(char caracterLlenado, int sizeLlenar){
	int sizeAux = sizeLlenar;
	while(sizeAux>0){
		int proximoBlock = leerBitMap();
		int cant = proximoBlock * tamanioDeBloque;
		for(int i=0; i<sizeAux && i<tamanioDeBloque;i++){
			elBlocks[cant+i]=caracterLlenado;
		}
		sizeAux-=tamanioDeBloque;
	}
}

void crearBlocks(){
	//Crea archivo vacio con el tamanio
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	FILE *archivoBlocks = fopen(ubicacionBlocks, "w");
	size_t tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	ftruncate(fileno(archivoBlocks), tamanioBlocks);
	fclose(archivoBlocks);
	mapearBlocks();
}

void crearFileSystem(){
	mkdir(puntoDeMontaje,0777);
	crearSuperBloque();
	crearBlocks();
	mkdir(string_from_format("%s/Files",puntoDeMontaje),0777);
	mkdir(string_from_format("%s/Files/Bitacoras",puntoDeMontaje),0777);
}

void conectarAlCliente(){
	t_config * config = config_create("./cfg/imongo.config");
	int listening_socket = crear_conexionServer(config_get_string_value(config, "PUERTO"));

	int socketCliente;
	struct sockaddr_in addr;

	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr, &addrlen);
	if(socketCliente == -1){
		printf("Error en la conexion");
	}else{
		printf("Conexion establecida con el Discordiador");
	}

	close(listening_socket);

	close(socketCliente);
}

#endif
