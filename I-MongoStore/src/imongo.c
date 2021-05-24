#include "imongo.h"

int main(int argc, char ** argv){

	leerConfig();

	crearFileSystem();
	llenarBlocks('o',132);
	munmap(elBlocks,tamanioDeBloque*cantidadDeBloques);
	
	//conectarAlCliente();

	return 0;
}