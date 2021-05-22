#ifndef imongo_H
#define imongo_H
#define BACKLOG 5 //TODO

#include "shared_utils.h"
#include <commons/txt.h>
#include <commons/bitarray.h>

char * puertoImongoStore;
uint32_t tamanioDeBloque;
uint32_t cantidadDeBloques;
char * puntoDeMontaje;



void leerConfig(){
    
	t_config * config = config_create("./cfg/imongo.config");
	      puntoDeMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	     puertoImongoStore = config_get_string_value(config, "PUERTO");//no importa??
//    char *  tiempoDeSinc = config_get_string_value(config, "TIEMPO_SINCRONIZACION");
          tamanioDeBloque = config_get_int_value(config, "BLOCK_SIZE");
          cantidadDeBloques = config_get_int_value(config, "CANTIDAD_BLOCKS");
        

}

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

void crearSuperBloque(){
//crear bloque y tamanio
 char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
 FILE * superBloque; 
 superBloque = fopen(ubicacionSuperBloque,"w");
 fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
 fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
 
//crear bitmap
 t_bitarray* bitMap = crearBitMap();
 txt_write_in_file(superBloque, bitMap);
 //fwrite(bitMap,sizeof(cantidadDeBloques / 8),1,superBloque);

 fclose(superBloque);
 bitarray_destroy(bitMap);
 //-----------------
 leerBitMap();
 //bitarray_destroy(leerBitMap());
}


void * leerBitMap(){
 char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
 FILE * superBloque; 
 superBloque = fopen(ubicacionSuperBloque,"r");
 size_t sizeBitMap = cantidadDeBloques / 8; 
 void * bitMap = malloc(sizeBitMap);

 fread(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
 printf("%d",tamanioDeBloque);
 fread(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
 printf("%d",cantidadDeBloques);
 fread(bitMap, sizeBitMap,1,superBloque);

 t_bitarray * punteroBitmap =  bitarray_create(bitMap, sizeBitMap);
 for(int i = 0; i<cantidadDeBloques; i++){
	  printf("%d",bitarray_test_bit(punteroBitmap,i));
  }
 fclose(superBloque);
}




#endif
