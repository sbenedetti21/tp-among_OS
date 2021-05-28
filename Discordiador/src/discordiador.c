#include "discordiador.h"


int main(int argc, char ** argv){

	listaTripulantes = list_create();
	listaReady = list_create();
	listaBloqueados = list_create();
	

	t_config * configuracion = config_create("./cfg/discordiador.config");
	sem_init(&semaforoTripulantes, 0,  config_get_int_value(configuracion, "GRADO_MULTITAREA"));

	 pthread_t hiloConsola;
	 pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	 pthread_join(hiloConsola, NULL);



sem_destroy(&semaforoTripulantes);


return 0;
}
