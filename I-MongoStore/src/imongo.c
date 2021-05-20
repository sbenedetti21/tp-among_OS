#include "imongo.h"



t_bitarray * crearBitMap(){
size_t sizeBitMap = cantidadDeBloques / 8; 
void * bitMap = malloc(sizeBitMap);

t_bitarray * punteroBitmap =  bitarray_create(bitMap, sizeBitMap);

for(int i = 0; i<cantidadDeBloques; i++){
	 bitarray_set_bit(punteroBitmap,i);
	 printf("%d",bitarray_test_bit(punteroBitmap,i));
  }
return punteroBitmap;
}

void * leerBitMap(){
 char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
 FILE * superBloque; 
 superBloque = fopen(ubicacionSuperBloque,"r");
 size_t sizeBitMap = cantidadDeBloques / 8; 
 
 fread(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
 fread(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
 
 void * bitMap = malloc(sizeBitMap);
 fread(bitMap, sizeBitMap,1,superBloque);
 t_bitarray * punteroBitmap =  bitarray_create(bitMap, sizeBitMap);
 for(int i = 0; i<cantidadDeBloques; i++){
	  printf("%d",bitarray_test_bit(punteroBitmap,i));
  }
 fclose(superBloque);
 return bitMap;
}


void crearSuperBloque(){
//crear bloque y tamanio
 char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
 FILE * superBloque; 
 superBloque = fopen(ubicacionSuperBloque,"w");
 fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
 fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
 
//crear bitmap
 t_bitarray* bitMap = crearBitMap();
 for(int i = 0; i<cantidadDeBloques; i++){
	 printf("%d",bitarray_test_bit(bitMap,i));
  }
 fwrite(bitMap,sizeof(cantidadDeBloques / 8),1,superBloque);

 fclose(superBloque);
 bitarray_destroy(bitMap);
 //-----------------
 //leerBitMap();
 //bitarray_destroy(leerBitMap());
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

	//conectarAlCliente();


	return 0;
}


