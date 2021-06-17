#include "imongo.h"

int main(int argc, char ** argv){
	loggerImongoStore = log_create("imongo.log", "imongo.c", 0, LOG_LEVEL_INFO); 

	leerConfig();
	struct stat st = {0};


	if(stat(puntoDeMontaje,&st) == -1){
		crearFileSystem();
		log_info(loggerImongoStore, "---------CREO FILESYSTEM----------");
	} else{	
		leerFileSystem();
		log_info(loggerImongoStore, "---------LEYO FILESYSTEM----------");
	}


	mapBlocksCopia = malloc(tamanioBlocks);
	memcpy(mapBlocksCopia, mapBlocks, tamanioBlocks);

	sem_init(&semaforoBloques, 0,  1 ); 
	sem_init(&semaforoArchivoRecurso, 0, 1);
	sem_init(&semaforoOxigeno, 0, 1);
	sem_init(&semaforoBasura, 0, 1);
	sem_init(&semaforoComida, 0, 1);

    pthread_t servidor;
    pthread_create(&servidor, NULL, servidorPrincipal, NULL);
    pthread_join(servidor, NULL);

	

//CREACION DEL HILO PRUEBA CON TOMI
/* 
	pthread_t hiloA;
	prueba* tripulantePruebaA = malloc(sizeof(prueba));
	tripulantePruebaA->cantidadRecurso = 15;
	tripulantePruebaA->tarea = GENERAR_OXIGENO;
	sem_init(&semBitArray,0,1);
	pthread_create(&hiloA, NULL, (void *) recibirTripulante, tripulantePruebaA);
	pthread_join(hiloA, NULL);
	sem_destroy(&semBitArray);
	free(tripulantePruebaA);
*/
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
	


	//Necesario al finalizar
	munmap(mapBlocks,tamanioBlocks);
	close(archivoBlocks);
	guardarBitMap();
	liberarBitMap(punteroBitmap);
	sem_destroy(&semaforoArchivoRecurso);
	sem_destroy(&semaforoBloques);
	sem_destroy(&semaforoOxigeno);
	sem_destroy(&semaforoBasura);
	sem_destroy(&semaforoComida);
	free(mapBlocksCopia);
	return 0;
}