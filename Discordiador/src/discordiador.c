#include "discordiador.h"


int main(int argc, char ** argv){

	
	listaReady = list_create();
	listaBloqueados = list_create();

	 pthread_t hiloConsola;
	 pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	 pthread_join(hiloConsola, NULL);

	//consola();





return 0;
}
