#include "discordiador.h"

int main(int argc, char ** argv){




	pthread_t * hiloConsola;


	pthread_create(&hiloConsola, NULL, consola, NULL);

	while(1);


return 0;
}








