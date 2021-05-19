#include "discordiador.h"

int proximoTID = 0 ;



void pasarTripulante(TCB * tripulante);
TCB * crearTCB(char *, t_list *);

int main(int argc, char ** argv){

	t_list * listaListos = list_create();
	char * instruccion;
	char ** vectorInstruccion;





	while(1) {

		instruccion = readline("Ingrese próxima instrucción: \n");

		vectorInstruccion = string_split(instruccion, " ");

			if(strcmp(vectorInstruccion[0], "conectarHilos") == 0){

				int cantidadHilos = atoi(vectorInstruccion[1]);
				pthread_t hilos[cantidadHilos];
				int contadorHilos = 0;

				for(int i=0; i < cantidadHilos; i++){
					pthread_t hilo;
					hilos[i] = hilo;
					TCB * tripulante = crearTCB(vectorInstruccion[2+i], listaListos);
					pthread_create(&hilos[i], NULL, (void *) pasarTripulante, tripulante);
					printf("SOY el hilo %d \n", contadorHilos);
					contadorHilos ++;
					pthread_join(&hilos[i], NULL);
				}

				for (int z = 0; z < cantidadHilos; z++){
					TCB * trip = lista_get(listaListos, z);
					printf("Soy el tripulante %d y espero en la lista \n", trip->tid);
				}
			}

			if(strcmp(vectorInstruccion[0], "conectarmiram") == 0){
				int cantidadHilos = atoi(vectorInstruccion[1]);
				pthread_t hilos[cantidadHilos];
				int contadorHilos = 0;

				for(int i=0; i < cantidadHilos; i++){
									pthread_t hilo;
									hilos[i] = hilo;

									pthread_create(&hilos[i], NULL, (void *) conectarMiRAM, NULL);
									printf("SOY el hilo %d \n", contadorHilos);
									contadorHilos ++;
									pthread_join(&hilos[i], NULL);
								}



			}


		}



return 0;
}


void pasarTripulante(TCB * tripulante){
	int socket = crear_conexion( "127.0.0.1","3500");
	send(socket, tripulante, sizeof(TCB), 0);
	close(socket);
}

TCB * crearTCB(char * posiciones, t_list * lista){


		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB * tripulante = malloc(sizeof(TCB));
		//tripulante->estado = 'R';
		tripulante->tid = proximoTID;
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		tripulante->tarea = atoi( vectorPosiciones[2]);
		//tripulante->punteroPCB;  //falta
		//tripulante->proximaInstruccion; //falta

		proximoTID ++; //ver sincronizacion

		int indexTripulante = lista_add(lista, tripulante);

		return tripulante;   //preguntar liberar malloc

	}

