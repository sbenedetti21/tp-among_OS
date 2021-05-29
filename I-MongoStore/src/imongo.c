#include "imongo.h"

int main(int argc, char ** argv){

	leerConfig();
	crearFileSystem();
	generarOxigeno(30);
	generarOxigeno(32);
	//generarOxigeno(32);
	//llenarBlocks('f',30);
	//generarOxigeno(26);
	munmap(mapBlocks,tamanioDeBloque*cantidadDeBloques);
	close(archivoBlocks);
	
	//conectarAlCliente();

	return 0;
}