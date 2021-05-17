#ifndef imongo_H
#define imongo_H
#define BACKLOG 5 //TODO

#include "shared_utils.h"

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



#endif
