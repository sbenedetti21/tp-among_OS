#include "mi_ram.h"

NIVEL* navePrincipal;



int main(int argc, char ** argv){

	loggerMiram = log_create("miram.log", "mi_ram.c", 0, LOG_LEVEL_INFO); 

	t_config * config = config_create("./cfg/miram.config");

	char * esquemaMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA"); 

	//if (strcmp(esquemaMemoria, "PAGINACION") == 0) .... 
	//if (strcmp(esquemaMemoria, "SEGMENTACION") == =) ....

	void * punteroMemoria = malloc(config_get_int_value(config, "TAMANIO_MEMORIA"));

	pthread_t servidor;
	pthread_create(&servidor, NULL, servidorPrincipal, config);

	pthread_t mapa;
	pthread_create(&mapa, NULL, iniciarMapa, NULL);
	 
	pthread_join(servidor, NULL);
	pthread_join(mapa, NULL);
	free(punteroMemoria);

	return 0; 
}

void servidorPrincipal(t_config * config) {
	char * puerto = config_get_string_value(config, "PUERTO");
	int listeningSocket = crear_conexionServer(puerto);

	int socketCliente;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
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
}


void atenderDiscordiador(int socketCliente){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);

	if(headerRECV) { printf("Recibi header: %d\n", paquete->header);} else{ printf("No pude recibir el header. \n");}

	int tamanioPAQUETE_RECV = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

	if(! tamanioPAQUETE_RECV){ printf("No pude recibir el tamanio del buffer \n");}

	paquete->buffer->stream = malloc(paquete->buffer->size);

	int PAQUETE_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size,0);

	if(! PAQUETE_RECV){ printf("No pude recibir el PAQUETE \n");}
	

	switch (paquete->header)
	{
	case CREAR_PCB: ; 
	
		uint32_t * punteroPCB = malloc(sizeof(punteroPCB));
		*punteroPCB = crearPCB("pathTareas");

		char * tareas = malloc(paquete->buffer->size);
		tareas = deserializar_Tareas(paquete->buffer);

		printf("%s \n", tareas);

		send(socketCliente, punteroPCB, sizeof(uint32_t),0);

		free(punteroPCB);

		break;

	case CREAR_TCB: ; 



		TCB * tripulante = deserializar_TCB(paquete->buffer);

			printf("ID: %d \n X: %d \n Y: %d \n ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);
			//agregarTripulanteAlMapa(tripulante);

			printf("------------------------\n");
			free(tripulante);

		break;

	case PRUEBA: ;

	
		break;
	
	default:	



		break;
	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	


}


TCB * deserializar_TCB(t_buffer * buffer){
	TCB * tripulante = malloc(sizeof(TCB));

	void* stream = buffer->stream;

	//Deserializamos los campos que tenemos en el buffer
	memcpy(&(tripulante->tid), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->punteroPCB), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->estado), stream, sizeof(char));
	stream += sizeof(char);

	return tripulante;
}

char * deserializar_Tareas(t_buffer * buffer) {
	int tamanioBuffer = buffer->size;

	char * tareas = malloc(tamanioBuffer);
	tareas = buffer->stream;
	//string_append(&tareas, "\0");
	return tareas;
}

uint32_t crearPCB(char* tareas){
	// uint32_t punteroTareas = *tareas; //ESTO ESTA RARI

	PCB * patota = malloc(sizeof(PCB));
	patota->pid = proximoPID; 				
	// patota->tareas = punteroTareas; // hABRIA QUE ALMACENAR LA PATOTA EN ALGUN LADO

	proximoPID++;


	uint32_t a = patota->pid;

	free(patota);
	return a;
}


// ------------------------------------------------------ MAPA ----------------------------------------------

void iniciarMapa() {
	
	nivel_gui_inicializar();
	NIVEL* navePrincipal = nivel_crear("Nave Principal");
	nivel_gui_dibujar(navePrincipal);
	sleep(1000);

}

void agregarTripulanteAlMapa(TCB* tripulante) {
	char id = '0';
	id = '0' + tripulante->tid;
	printf("%c", id);
	int posicionX = tripulante->posicionX;
	int posicionY = tripulante->posicionY;
	personaje_crear(navePrincipal, id, posicionX, posicionY);
}

void moverTripulanteEnMapa(TCB * tripulante, int x, int y) {
	char id = '0' + tripulante->tid;
	item_mover(navePrincipal, id, x, y);
}

void expulsarTripulanteDelMapa(TCB* tripulante) {
	char id = '0' + tripulante->tid;
	item_borrar(navePrincipal, id);
}