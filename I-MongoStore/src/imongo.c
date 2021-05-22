#include "imongo.h"

void liberarBitMap(t_bitarray *punteroBitmap){
	free(punteroBitmap->bitarray);
 	bitarray_destroy(punteroBitmap);
}

t_bitarray *crearBitMap(){
	size_t sizeBitMap = cantidadDeBloques / 8; 
	char * bitMap = malloc(sizeBitMap);
	return bitarray_create(bitMap, sizeBitMap);
}

t_bitarray *leerBitMap(){
	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r");
 
	fread(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
	fread(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
	
	size_t sizeBitMap = cantidadDeBloques / 8; 
 	char * bitMap = malloc(sizeBitMap);
 	t_bitarray * punteroBitmap =  bitarray_create(bitMap, sizeBitMap);
 	fread(bitMap,sizeof(t_bitarray),1,superBloque);

	for(int i = 0; i<cantidadDeBloques; i++){
		printf("%d\n",bitarray_test_bit(punteroBitmap,i));
	}

	fclose(superBloque);
	
	return punteroBitmap;
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
	char *bitMap = punteroBitmap->bitarray;
	bitarray_set_bit(punteroBitmap,0);

	fwrite(bitMap,sizeof(t_bitarray),1,superBloque);

	for(int i = 0; i<cantidadDeBloques; i++){
		printf("%d\n",bitarray_test_bit(punteroBitmap,i));
	}

	fclose(superBloque);

	liberarBitMap(punteroBitmap);
}


// void crearBloques(){
//  char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
//  leerSuperBloque();

//  FILE * blocks; 
//  blocks = fopen(ubicacionBlocks,"w");

//  fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
//  fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
//  fclose(superBloque);

// }


void crearFileSystem(){
	crearSuperBloque();
/* char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
FILE * blocks = fopen(ubicacionBlocks,"w");
*/
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


int main(int argc, char ** argv){

	leerConfig();
	crearFileSystem();
 	t_bitarray *punteroBitmap = leerBitMap();
	liberarBitMap(punteroBitmap);

	//conectarAlCliente();


	return 0;
}


