#include "mi_ram.h"



int main(int argc, char ** argv){

	t_config * config = config_create("./cfg/miram.config");
	char * puerto = config_get_string_value(config, "PUERTO");

	int listeningSocket = crear_conexionServer(puerto);

	int socketCliente;
    struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);



	socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	if(socketCliente == -1){printf("Error en la conexión");}
	else {
		printf("Conexión establecida con Discordiador \n");
	}

	close(socketCliente);
	close(listeningSocket);

	return 0;


    
}
