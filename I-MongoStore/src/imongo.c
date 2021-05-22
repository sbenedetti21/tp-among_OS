#include "imongo.h"

int main(int argc, char ** argv){

	leerConfig();

	crearFileSystem();
 	t_bitarray *punteroBitmap = leerBitMap();
	for(int i = 0; i<cantidadDeBloques; i++){
		printf("%d",bitarray_test_bit(punteroBitmap,i));
	}
	printf("\n");
	liberarBitMap(punteroBitmap);
	
	//conectarAlCliente();

	return 0;
}