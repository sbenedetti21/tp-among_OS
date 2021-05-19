#include "mi_ram.h"



int main(int argc, char ** argv){

	t_config * config = config_create("./cfg/miram.config");
	char * puerto = config_get_string_value(config, "PUERTO");

	int listeningSocket = crear_conexionServer(puerto);

	int socketCliente;
    struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);




		pthread_t receptorDiscordiador;

		socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
			if(socketCliente == -1){printf("Error en la conexión");}
			else {
				printf("Conexión establecida con Discordiador \n");
				atenderDiscordiador(socketCliente);
				//pthread_create(&receptorDiscordiador, NULL, atenderDiscordiador, socketCliente);

	}


	close(socketCliente);

	close(listeningSocket);

	return 0;


    
}


 void atenderDiscordiador(int socketCliente){

	 TCB * tripulante = malloc(sizeof(TCB));



	int status =  recv(socketCliente, (void *) tripulante, sizeof(TCB), 0);
	printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

	free(tripulante);


 }










