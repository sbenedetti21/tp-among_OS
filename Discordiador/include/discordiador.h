#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

typedef struct {
		int id;
		int posicionx;
		int posiciony;
} tripulante_t;

int conectarImongo();
int conectarMiRAM();

struct tripulante_t *crearTripulante(char *);
void mostrarPosicion(tripulante_t*);


int conectarImongo(){
	t_config * config = config_create("./cfg/discordiador.config");
		char * ip = config_get_string_value(config, "IP_I_MONGO_STORE");
		char * puerto = config_get_string_value(config, "PUERTO_I_MONGO_STORE");

		return crear_conexion(ip, puerto);
}

int conectarMiRam(){


	t_config * config = config_create("./cfg/discordiador.config");
	char * ip = config_get_string_value(config, "IP_MI_RAM_HQ");
	char * puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");

	return crear_conexion(ip, puerto);

}

#endif
