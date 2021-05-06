#include "discordiador.h"


int main(int argc, char ** argv){

	char * instruccion;
	char ** vectorInstruccion;

	while(1) {

		instruccion = readline("Ingrese próxima instrucción: \n");

		vectorInstruccion = string_split(instruccion, " ");

		if(strcmp(vectorInstruccion[0], "INICIAR_PATOTA") == 0) {

			//INICIAR_PATOTA 5 txt ... ... ...
			int i;
			int cantidadTripulantes = atoi(vectorInstruccion[1]);
			pthread_t tripulantes[cantidadTripulantes];

			for(i = 0; i < cantidadTripulantes; i++ ) {
				pthread_t hilo;
				tripulantes[i] = hilo;
				pthread_create(&tripulantes[i], NULL, sleep /*TODO iniciarTripulante */, 10/*posisicion??*/ );
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

return 0;
}


