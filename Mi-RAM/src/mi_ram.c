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



	// nivel_gui_inicializar();
	// NIVEL* navePrincipal = nivel_crear("Nave Principal");
	// nivel_gui_dibujar(navePrincipal);
	// //pthread_t mapa;
	// //pthread_create(&mapa, NULL, iniciarMapa, NULL);
	// //pthread_join(mapa, NULL);
	 
	pthread_join(servidor, NULL);
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
		if(socketCliente == -1){printf("Error en la conexión"); log_info(loggerMiram, "error en la conexion con Discordiador");}
		else {
			log_info(loggerMiram, "Conexión establecida con Discordiador");
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
	if(headerRECV) { log_info(loggerMiram, "Recibi header: %d\n", paquete->header);} else{ log_error(loggerMiram, "No se pudo recibir el header");}
	
	int BUFFER_RECV  = 0;
	
	int statusTamanioBuffer = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);
	if(! statusTamanioBuffer){ log_error(loggerMiram, "No se pudo recibir el tamanio del buffer ");}

	paquete->buffer->stream = malloc(paquete->buffer->size);

	BUFFER_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream

	if(! BUFFER_RECV){ log_error(loggerMiram,"No se pudo recibir el buffer");}


	switch (paquete->header)
	{
	case INICIAR_PATOTA: ; 

		void* stream = malloc(paquete->buffer->size);
		stream = paquete->buffer->stream;

		// Deserializamos tareas
		int tamanioTareas;
		memcpy(&tamanioTareas, stream, sizeof(int));
		stream += sizeof(int);
		char* tareas = malloc(tamanioTareas);
		memcpy(tareas, stream, tamanioTareas);
		stream += tamanioTareas;

		printf("%s \n", tareas);

		//Deserializar TCBs
		int cantidadTCBs = 0;
		memcpy(&cantidadTCBs, stream, sizeof(int));
		stream += sizeof(int);

		
		printf("cantidad tcbs: %d \n", cantidadTCBs);

		for(int i = 0 ; i < cantidadTCBs ;  i++ ){
			TCB * tripulante = malloc(sizeof(TCB));
			//tripulante = deserializar_TCB(stream);

			uint32_t id, x, y = 0;
			char e = 0;

<<<<<<< HEAD
		// Meter en memoria al TCB

			log_info(loggerMiram, "tripulante recibido. ID: %d, X: %d, Y: %d ", tripulante->tid, tripulante->posicionX, tripulante->posicionY);
=======
			//Deserializamos los campos que tenemos en el buffer
			memcpy(&id, stream, sizeof(uint32_t));
			stream += sizeof(uint32_t);
>>>>>>> pruebaCsnexionPCB

			memcpy(&x, stream, sizeof(uint32_t));
			stream += sizeof(uint32_t);

<<<<<<< HEAD
			//	int personaje = personaje_crear(navePrincipal, '0', tripulante->posicionX, tripulante->posicionY); 
			//	nivel_gui_dibujar(navePrincipal); 
=======
			memcpy(&y, stream, sizeof(uint32_t));
			stream += sizeof(uint32_t);
>>>>>>> pruebaCsnexionPCB

			memcpy(&e, stream, sizeof(char));
			stream += sizeof(char);

			tripulante->tid = id;
			tripulante->posicionX = x;
			tripulante->posicionY = y;
			tripulante->estado = e;

			printf("ID: %d \n", tripulante->tid);
		}

		break;

	case PEDIR_TAREA: ;

	
		break;
	
	default:	



		break;
	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	


}


// TCB * deserializar_TCB(void * stream){ // Agregar stuff
// 	TCB * tripulante = malloc(sizeof(TCB));

// 	//Deserializamos los campos que tenemos en el buffer
// 	memcpy(&(tripulante->tid), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);

// 	memcpy(&(tripulante->posicionX), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);

// 	memcpy(&(tripulante->posicionY), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);

// 	memcpy(&(tripulante->estado), stream, sizeof(char));
// 	stream += sizeof(char);

// 	return tripulante;
// }

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
	id = '0' + tripulante->tid; //aca el tid es un uint 
	printf("%c", id);
	int posicionX = tripulante->posicionX;
	int posicionY = tripulante->posicionY;
	personaje_crear(navePrincipal, id, posicionX, posicionY);
	nivel_gui_dibujar(navePrincipal);
}

void moverTripulanteEnMapa(TCB * tripulante, int x, int y) {
	char id = '0' + tripulante->tid;
	item_mover(navePrincipal, id, x, y);
}

void expulsarTripulanteDelMapa(TCB* tripulante) {
	char id = '0' + tripulante->tid;
	item_borrar(navePrincipal, id);
}