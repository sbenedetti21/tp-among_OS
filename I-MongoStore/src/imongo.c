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
	generarOxigeno(15);
	generarBasura(30);
	generarOxigeno(32);
	generarBasura(25);
	generarOxigeno(32);
	generarBasura(2);
	generarOxigeno(26);
	consumirOxigeno(15);
	consumirOxigeno(60);
	generarBasura(33);
	
	//conectarAlCliente();

	printf("Prueba\n");

	//Necesario al finalizar
	msync(mapBlocks, tamanioDeBloque*cantidadDeBloques, MS_SYNC);
	munmap(mapBlocks,tamanioDeBloque*cantidadDeBloques);
	close(archivoBlocks);
	liberarBitMap(punteroBitmap);
	return 0;
}