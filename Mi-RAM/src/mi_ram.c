#include "mi_ram.h"

/*void aceptarConexion(structConexion);*/
void atenderDiscordiador(int);

typedef struct {

	int socket;
	struct sockaddr_in address;
	socklen_t addresslength;


} structConexion;


int main(int argc, char ** argv){

	t_config * config = config_create("./cfg/miram.config");
	char * puerto = config_get_string_value(config, "PUERTO");

	int listeningSocket = crear_conexionServer(puerto);

	void * punteroMemoria = malloc(config_get_int_value(config, "TAMANIO_MEMORIA"));


	int socketCliente;

	    struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		int status = 1;
		pthread_t receptorDiscordiador;


		while(1){



			socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
				if(socketCliente == -1){printf("Error en la conexión");}
				else {
					printf("Conexión establecida con Discordiador \n");
					pthread_create(&receptorDiscordiador, NULL, atenderDiscordiador, socketCliente);


		}
			}



		close(socketCliente);


	close(listeningSocket);

	return 0;


    
}


/*funcion hilo aceptar conexion

void aceptarConexion(structConexion datosConexion){

	int socketCliente;y
	while(1){
	 socketCliente = accept(datosConexion.socket,(struct sockaddr *) &datosConexion.address, &datosConexion.addresslength);
	if(socketCliente == -1){printf("Error en la conexión");}
		else {
			printf("Conexión establecida con Discordiador \n");

		}
	}
	close(socketCliente);

}
*/
void atenderDiscordiador(int socketCliente){

	 TCB * tripulante = malloc(sizeof(TCB));



	int status =  recv(socketCliente, (void *) tripulante, sizeof(TCB), 0);

	printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

	printf("------------------------\n");
	free(tripulante);


}


//un comentario

