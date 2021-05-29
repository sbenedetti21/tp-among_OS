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
	generarOxigeno(30);
	generarOxigeno(32);
	generarOxigeno(32);
	//llenarBlocks('f',30);
	generarOxigeno(26);
	consumirOxigeno(30);
	
	//conectarAlCliente();

	printf("Prueba\n");

	//Necesario al finalizar
	msync(mapBlocks, tamanioDeBloque*cantidadDeBloques, MS_SYNC);
	munmap(mapBlocks,tamanioDeBloque*cantidadDeBloques);
	close(archivoBlocks);
	liberarBitMap(punteroBitmap);
	return 0;
}