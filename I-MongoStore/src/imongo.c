#include "imongo.h"

int main(int argc, char ** argv){

	leerConfig();
	if(true)
		crearFileSystem();
	else{
		ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
		leerBitMap();
		mapearBlocks();
	}
	pthread_t hiloA;
	pthread_t hiloB;
	pthread_t hiloC;
	prueba* tripulantePruebaA = malloc(sizeof(prueba));
	prueba* tripulantePruebaB = malloc(sizeof(prueba));
	prueba* tripulantePruebaC = malloc(sizeof(prueba));
	tripulantePruebaA->cantidadRecurso = 15;
	tripulantePruebaB->cantidadRecurso = 35;
	tripulantePruebaC->cantidadRecurso = 45;
	tripulantePruebaA->tarea = GENERAR_OXIGENO;
	tripulantePruebaB->tarea = GENERAR_BASURA;
	tripulantePruebaC->tarea = GENERAR_OXIGENO;
	sem_init(&semBitArray,0,1);
	pthread_create(&hiloA, NULL, (void *) recibirTripulante, tripulantePruebaA);
	pthread_create(&hiloB, NULL, (void *) recibirTripulante, tripulantePruebaB);
	pthread_create(&hiloC, NULL, (void *) recibirTripulante, tripulantePruebaC);
	pthread_join(hiloA, NULL);
	pthread_join(hiloB, NULL);
	pthread_join(hiloC, NULL);
	sem_destroy(&semBitArray);
	free(tripulantePruebaA);
	free(tripulantePruebaB);
	free(tripulantePruebaC);
	/*
	recibirTripulante(GENERAR_OXIGENO, 15);
	recibirTripulante(GENERAR_OXIGENO, 35);
	recibirTripulante(GENERAR_COMIDA, 15);
	recibirTripulante(GENERAR_BASURA, 15);
	recibirTripulante(CONSUMIR_OXIGENO, 15);
	*/	

	/*
	generarOxigeno(15, mapBlocksAux);
	generarBasura(30, mapBlocksAux);
	generarOxigeno(32, mapBlocksAux);
	generarBasura(25, mapBlocksAux);
	generarOxigeno(32, mapBlocksAux);
	generarBasura(2, mapBlocksAux);
	generarOxigeno(26, mapBlocksAux);
	consumirOxigeno(15, mapBlocksAux);
	consumirOxigeno(60, mapBlocksAux);
	generarBasura(33, mapBlocksAux);
	*/
	
	
	//conectarAlCliente();

	printf("Prueba\n");

	//Necesario al finalizar
	munmap(mapBlocks,tamanioBlocks);
	close(archivoBlocks);
	liberarBitMap(punteroBitmap);
	return 0;
}