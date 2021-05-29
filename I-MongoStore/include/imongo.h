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

char * puertoImongoStore;
uint32_t tamanioDeBloque;
uint32_t cantidadDeBloques;
char * puntoDeMontaje;
//Bloques
int proximoBlock;
//Variables del map blocks
char *mapBlocks;
int archivoBlocks;

void leerConfig(){ 
	t_config * config = config_create("./cfg/imongo.config");
	
	puntoDeMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	puertoImongoStore = config_get_string_value(config, "PUERTO");//no importa??
//  char *  tiempoDeSinc = config_get_string_value(config, "TIEMPO_SINCRONIZACION");
	tamanioDeBloque = config_get_int_value(config, "BLOCK_SIZE");
    cantidadDeBloques = config_get_int_value(config, "CANTIDAD_BLOCKS");
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

void liberarBitMap(t_bitarray *punteroBitmap){
	free(punteroBitmap->bitarray);
 	bitarray_destroy(punteroBitmap);
}

t_bitarray *leerBitMap(){
	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");

	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	t_bitarray * punteroBitmap = crearBitMap();
 	fread(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);	
	return punteroBitmap;
}

int bitLibreBitMap(t_bitarray *punteroBitmap){
	while(bitarray_test_bit(punteroBitmap,proximoBlock))
			proximoBlock++;
	return proximoBlock;
}

void cambiarBitMap(t_bitarray *punteroBitmap){
	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");

	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	liberarBitMap(punteroBitmap);
	fclose(superBloque);
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
	archivoBlocks = open(ubicacionBlocks, O_RDWR , S_IRUSR | S_IWUSR);
	mapBlocks = mmap(NULL, tamanioBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBlocks,0);
}

void creacionArchivoRecurso(char * ubicacionArchivoRecurso, char *caracterLlenado){
	FILE *fileRecurso = fopen(ubicacionArchivoRecurso,"w");
	fclose(fileRecurso);
	t_config * configRecurso = config_create(ubicacionArchivoRecurso);
	config_set_value(configRecurso,"SIZE",string_itoa(64));
	config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(0));
	config_set_value(configRecurso,"BLOCKS","[]");
	config_set_value(configRecurso,"CARACTER_LLENADO",caracterLlenado);
	config_set_value(configRecurso,"MD5_ARCHIVO","");
	config_save(configRecurso);
	config_destroy(configRecurso);
}

void agregarBloqueAlFile(int nuevoBloque, char * ubicacionArchivoRecurso){
	t_config * configRecurso = config_create(ubicacionArchivoRecurso);
	config_set_value(configRecurso,"SIZE",string_itoa(64));

	int nuevaCantidadBloques = config_get_int_value(configRecurso,"BLOCK_COUNT");
	nuevaCantidadBloques++;
	config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(nuevaCantidadBloques));

	char *listaBlocks = config_get_string_value(configRecurso,"BLOCKS");
	char *nuevaListablocks = string_substring_until(listaBlocks,strlen(listaBlocks)-1);

	if(listaBlocks[1]!=']')
		string_append(&nuevaListablocks, ",");

	string_append(&nuevaListablocks, string_itoa(nuevoBloque));
	string_append(&nuevaListablocks, "]");
	config_set_value(configRecurso,"BLOCKS",nuevaListablocks);

	config_save(configRecurso);
	config_destroy(configRecurso);
}

void leerUltimoBloque(char * ubicacionArchivoRecurso){
	t_config * configRecurso = config_create(ubicacionArchivoRecurso);
	char *listaBlocks = config_get_string_value(configRecurso,"BLOCKS");
	if(listaBlocks[1]!=']'){
		int indiceFinal = strlen(listaBlocks);
		while(listaBlocks[indiceFinal]!=',' && indiceFinal>0)
			indiceFinal--;
		int largoNum = strlen(listaBlocks) - indiceFinal;
		proximoBlock = atoi(string_substring(listaBlocks,indiceFinal+1,largoNum));
	}else{
		proximoBlock = 0;
	}

	config_destroy(configRecurso);
}

void llenarBlocks(char caracterLlenado, int cantLlenar, char * ubicacionArchivoRecurso){
	int cantAux = cantLlenar;
	t_bitarray *punteroBitmap = leerBitMap();
	leerUltimoBloque(ubicacionArchivoRecurso);
	while(cantAux>0){
		bitLibreBitMap(punteroBitmap);
		int cant = proximoBlock * tamanioDeBloque;
		if(mapBlocks[cant]=='\0' || mapBlocks[cant]==caracterLlenado){
			for(int i=0; i<cantAux && i<tamanioDeBloque;i++){
				if(mapBlocks[cant+i]!=caracterLlenado)
					mapBlocks[cant+i]=caracterLlenado;
				else
					cantAux++;
			}
			cantAux-=tamanioDeBloque;

			if(cantAux>=0){
				agregarBloqueAlFile(proximoBlock, ubicacionArchivoRecurso);
				bitarray_set_bit(punteroBitmap,proximoBlock);
			}

		} else {
			proximoBlock++;
		}
		
	}

	cambiarBitMap(punteroBitmap);
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

void generarOxigeno(int cantidadALlenar){
	char * ubicacionOxigeno = string_from_format("%s/Files/Oxigeno.ims",puntoDeMontaje);
	if(access(ubicacionOxigeno, F_OK )){
		creacionArchivoRecurso(ubicacionOxigeno, "O");
	}
	llenarBlocks('O', cantidadALlenar, ubicacionOxigeno);
}

void conectarAlCliente(){
	t_config * config = config_create("./cfg/imongo.config");
	int listening_socket = crear_conexionServer(config_get_string_value(config, "PUERTO"));

	int socketCliente;
	struct sockaddr_in addr;

	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr, &addrlen);
	if(socketCliente == -1){
		printf("Error en la conexion\n");
	}else{
		printf("Conexion establecida con el Discordiador\n");
	}

	close(listening_socket);

	close(socketCliente);
}

#endif
