#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

typedef struct tripulante_t {
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

struct tripulante_t * crearTripulante(char * posicion){

	char ** vectorPosicion = string_split(posicion, "|");
	int x = atoi(vectorPosicion[0]);
    int y = atoi(vectorPosicion[1]);
    tripulante_t* unTripulante = malloc(sizeof(tripulante_t));
	unTripulante->posicionx = x;
	unTripulante->posiciony = y;
	return unTripulante;
}

void mostrarPosicion(tripulante_t* unTripulante) {
	printf("Soy el tripulante numero %d. \n Mi posición en x = %d. \n Mi posición en y = %d. \n", unTripulante -> id,  unTripulante -> posicionx , unTripulante -> posiciony);
}


void consola(){


	while(1) {

		instruccion = readline("Ingrese próxima instrucción: \n");

		vectorInstruccion = string_split(instruccion, " ");


		if(strcmp(vectorInstruccion[0], "INICIAR_PATOTA") == 0) {

			//INICIAR_PATOTA 3 txt 1|1 1|2 1|3
			int i;
			int indice_posiciones = 3;
			int cantidadTripulantes = atoi(vectorInstruccion[1]);
			pthread_t tripulantes[cantidadTripulantes];

			for(i = 0; i < cantidadTripulantes; i++ ) {
				pthread_t hilo;

				tripulante_t* tripulante = malloc(sizeof(tripulante_t));
				if (vectorInstruccion[indice_posiciones] != NULL) {
					tripulante = crearTripulante(vectorInstruccion[3 + i]);
					indice_posiciones++;
				} else {
					tripulante = crearTripulante(posicionBase);
				}
				tripulante->id = i;
				tripulantes[i] = hilo;

				pthread_create(&tripulantes[i], NULL, mostrarPosicion , tripulante);
				pthread_join(&tripulantes[i], NULL);
			}

		}

		/*
		if(strcmp(vectorInstruccion[0], "LISTAR_TRIPULANTES") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "EXPULSAR_TRIPULANTE") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "INICIAR_PLANIFICACION") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "PAUSAR_PLANIFICACION") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "OBTENER_BITACORA") == 0) {
		}
		*/

	}
}

#endif
