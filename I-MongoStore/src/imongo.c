#include "imongo.h"

int main(int argc, char ** argv){

	leerConfig();
	crearFileSystem();
	llenarBlocks('o',30);
	llenarBlocks('o',32);
	llenarBlocks('o',32);
	llenarBlocks('f',30);
	llenarBlocks('o',26);
	munmap(elBlocks,tamanioDeBloque*cantidadDeBloques);
	
	//conectarAlCliente();

	return 0;
}