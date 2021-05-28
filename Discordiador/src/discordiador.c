#include "discordiador.h"


int main(int argc, char ** argv){

	listaTripulantes = list_create();
	listaReady = list_create();
	listaBloqueados = list_create();
	

	t_config * configuracion = config_create("./cfg/discordiador.config");
	sem_init(&semaforoTripulantes, 0,  config_get_int_value(configuracion, "GRADO_MULTITAREA"));
int valor; 
	 sem_getvalue(&semaforoTripulantes, &valor);   

	printf("%d \n", valor);



	 pthread_t hiloConsola;
	 pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	 pthread_join(hiloConsola, NULL);

	//consola();

sem_destroy(&semaforoTripulantes);


return 0;
}
