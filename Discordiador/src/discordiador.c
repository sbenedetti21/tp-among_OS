#include "discordiador.h"

int main(int argc, char ** argv){

//prueba
return conectarMIRam();

}

int conectarMIRam(){


	t_config * config = config_create("./cfg/discordiador.config");
	char * ip = config_get_string_value(config, "IP_MI_RAM_HQ");
	char * puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");

	return crear_conexion(ip, puerto);



}
