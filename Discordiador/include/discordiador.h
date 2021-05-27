#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

#include "shared_utils.h"

int proximoTID = 0;




t_list * listaReady;
t_list * listaBloqueados;

uint32_t iniciarPCB(char*, int);
int conectarImongo();
int conectarMiRAM();
void iniciarPatota(char **);
void listarTripulantes();
bool coincideID(TCB*);

void pasarTripulante(TCB *);
TCB * crearTCB(char *, uint32_t); // chequear lo de la lista


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


		if(strcmp(vectorInstruccion[0], "I") == 0) {


			iniciarPatota(vectorInstruccion);

			
		}

		/* if(strcmp(vectorInstruccion[0], "expulsar") == 0){
			//TODO TERMINAR EXPULSAR_TRIPULANTE

			bool coincideID(TCB* tripulante){
				return tripulante->tid ==  atoi(vectorInstruccion[1]);
			}

			list_remove_by_condition(listaReady,  coincideID);

		}*/
		

	
		
		if(strcmp(vectorInstruccion[0], "LISTAR_TRIPULANTES") == 0) {
			
			listarTripulantes();

		}


		/*
		if(strcmp(vectorInstruccion[0], "EXPULSAR_TRIPULANTE") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "INICIAR_PLANIFICACION") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "PAUSAR_PLANIFICACION") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "OBTENER_BITACORA") == 0) {
		}

		
		*/

				for(int e = 0; e < list_size(listaReady); e++){
					TCB *tripulante = list_get(listaReady,e);

					printf("Posicion: %d, ID:%d, X:%d, Y:%d \n",e,tripulante->tid, tripulante->posicionX, tripulante->posicionY);
				}

	}

}



TCB * crearTCB(char * posiciones, uint32_t punteroAPCB){


		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB * tripulante = malloc(sizeof(TCB));
		tripulante->estado = 'R';
		tripulante->tid = proximoTID;
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		tripulante->punteroPCB = punteroAPCB;  
		//tripulante->proximaInstruccion; //falta

		list_add(listaReady, tripulante);

		proximoTID ++; //ver sincronizacion

		return tripulante;   //preguntar liberar malloc
	}





void iniciarPatota(char ** vectorInstruccion){

	int socket = conectarMiRAM();

	uint32_t punteroPCB = iniciarPCB(vectorInstruccion[2], socket);

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
						tripulante = crearTCB(vectorInstruccion[3 + i],punteroPCB);
						indice_posiciones++;
					} else {
						tripulante = crearTCB(posicionBase, punteroPCB);
					}

					void pasarTripulante(TCB * tripulante){ 
						send(socket, tripulante, sizeof(TCB), 0);
					}

					pthread_create(&tripulantes[i], NULL, pasarTripulante , tripulante);
					pthread_join(tripulantes[i], NULL);
				}

	close(socket);


}


void listarTripulantes(){

printf("--------------------------------------------------------- \nEstado actual de la nave: %s    \n\n", temporal_get_string_time("%d/%m/%y %H:%M:%S"));

	for(int i = 0; i < list_size(listaReady) ; i++){

		TCB *tripulante = list_get(listaReady,i);
		// PCB *patota = tripulante->punteroPCB;

		printf("Tripulante: %d    Patota:    Estado: %c \n", tripulante->tid , /* patota->pid, */ tripulante->estado);

	}

printf("--------------------------------------------------------- \n");

}


uint32_t iniciarPCB(char * pathTareas, int socket){
	uint32_t * punteroPCB = malloc(sizeof(uint32_t));
	int * punteroACrearPCB = malloc(sizeof(int));
	*punteroACrearPCB = CREAR_PCB;
	
	send(socket, punteroACrearPCB, sizeof(int) , 0);	//VER SERIALIZACION DINAMICA

	int prueba = recv(socket, (void*)punteroPCB, sizeof(uint32_t),0);

	return *punteroPCB;
}


#endif
