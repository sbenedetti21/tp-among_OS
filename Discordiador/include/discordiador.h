#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

int proximoTID = 0;  // variable global?? se define asi?


t_list * listaReady;
t_list * listaBloqueados;

int conectarImongo();
int conectarMiRAM();
void iniciarPatota(char **);

void pasarTripulante(TCB * tripulante);
TCB * crearTCB(char *); // chequear lo de la lista


int conectarImongo(){
	t_config * config = config_create("./cfg/discordiador.config");
		char * ip = config_get_string_value(config, "IP_I_MONGO_STORE");
		char * puerto = config_get_string_value(config, "PUERTO_I_MONGO_STORE");

		return crear_conexion(ip, puerto);
}

int conectarMiRAM(){


	t_config * config = config_create("./cfg/discordiador.config");
	char * ip = config_get_string_value(config, "IP_MI_RAM_HQ");
	char * puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");

	return crear_conexion(ip, puerto);


}


void consola(){

	char * instruccion;
	char ** vectorInstruccion;


	while(1) {

		instruccion = readline("Ingrese próxima instrucción: \n");

		vectorInstruccion = string_split(instruccion, " ");


		if(strcmp(vectorInstruccion[0], "INICIAR_PATOTA") == 0) {


			iniciarPatota(vectorInstruccion);


		}

		/*if(strcmp(vectorInstruccion[0], "expulsar") == 0){

			int idexpulsado = atoi(vectorInstruccion[1]);
			//list_find(listaReady, tripulante->tid == idexpulsado); INVESTIGAR PUNTERO A FUNCION
			list_remove(listaReady, indexEliminado);

		}

		*/

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

TCB * crearTCB(char * posiciones){


		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB * tripulante = malloc(sizeof(TCB));
		tripulante->estado = 'R';
		tripulante->tid = proximoTID;
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		//tripulante->punteroPCB;  //falta
		//tripulante->proximaInstruccion; //falta

		list_add(listaReady, tripulante);

		proximoTID ++; //ver sincronizacion

		return tripulante;   //preguntar liberar malloc
	}

void pasarTripulante(TCB * tripulante){
	int socket = conectarMiRAM();
	send(socket, tripulante, sizeof(TCB), 0);
	close(socket);
}



void iniciarPatota(char ** vectorInstruccion){

	//INICIAR_PATOTA 3 txt 1|1 1|2 1|3
				char * posicionBase = "0|0";
				int i;
				int indice_posiciones = 3;
				int cantidadTripulantes = atoi(vectorInstruccion[1]);
				pthread_t tripulantes[cantidadTripulantes];

				for(i = 0; i < cantidadTripulantes; i++ ) {
					pthread_t hilo;

					TCB* tripulante = malloc(sizeof(TCB));
					if (vectorInstruccion[indice_posiciones] != NULL) {
						tripulante = crearTCB(vectorInstruccion[3 + i]);
						indice_posiciones++;
					} else {
						tripulante = crearTCB(posicionBase);
					}

					pthread_create(&tripulantes[i], NULL, pasarTripulante , tripulante);
					pthread_join(&tripulantes[i], NULL);
				}

				for(int e = 0; e < list_size(listaReady); e++){
					TCB *tripulante = list_get(listaReady,e);

					printf("Posicion: %d, ID:%d, X:%d, Y:%d \n",e,tripulante->tid, tripulante->posicionX, tripulante->posicionY);
				}

}





#endif
