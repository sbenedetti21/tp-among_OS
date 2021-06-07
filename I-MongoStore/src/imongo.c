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
	
	recibirTripulante(GENERAR_OXIGENO, 15);
	recibirTripulante(GENERAR_OXIGENO, 35);
	recibirTripulante(GENERAR_COMIDA, 15);
	recibirTripulante(GENERAR_BASURA, 15);
	recibirTripulante(CONSUMIR_OXIGENO, 15);
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