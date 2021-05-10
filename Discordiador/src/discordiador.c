#include "discordiador.h"

void mostrarPosicion(char *);

int main(int argc, char ** argv){

	char * instruccion;
	char ** vectorInstruccion;


struct Tripulante{
		int id;
		int posicionx;
		int posiciony;

	};







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
				struct Tripulante tripulante;
				tripulante.id = 0;
				tripulantes[i] = hilo;
				pthread_create(&tripulantes[i], NULL, mostrarPosicion , tripulante );
				pthread_join(&tripulantes[i], NULL);

			}

			// Preguntar memory leaks con valgrind

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

/*
 *
 *
 * inicializar una sola funcion y pasarle un flag que me diga que funcion ejecutar
 * */

void mostrarPosicion(struct Tripulante * unTripulante){
	/*if(posicion != 0){*/
	char ** vectorPosicion = string_split(posicion, "|");
	int x = atoi(vectorPosicion[0]);
	int y =  atoi(vectorPosicion[1]);
	unTripulante->posicionx = x;
	unTripulante->posiciony = y;
	printf("Soy el tripulante numero %d. \n Mi posición en x = %d. \n Mi posición en y = %d. \n", unTripulante -> id,  x ,y);
	/*
	else{
		printf("Mi posición en x = 0. \n Mi posicion en y = 0. \n");
	}*/
	sleep(5);
	return ;
}
