#include "imongo.h"

int main(int argc, char ** argv){

	t_config *config = config_create("./cfg/imongo.config");
	int listening_socket = crear_conexionServer(config_get_string_value(config, "PUERTO"));


	int socketCliente;
	struct sockaddr_in addr;

	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr, &addrlen);
	if(socketCliente == -1){
		printf("Error en la conexion");
	}else{
		printf("Conexion establecida con el Discordiador");
	}

	close(listening_socket);

	close(socketCliente);

	return 0;
}


