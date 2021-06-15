#include "mi_ram.h"



NIVEL* navePrincipal;



int main(int argc, char ** argv){

	loggerMiram = log_create("miram.log", "mi_ram.c", 0, LOG_LEVEL_INFO);
	leerConfig();

	//if (strcmp(esquemaMemoria, "PAGINACION") == 0) .... 
	//if (strcmp(esquemaMemoria, "SEGMENTACION") == =) ....

	memoriaPrincipal = malloc(tamanioMemoria);
	iniciarMemoria();

	pthread_t servidor;
	pthread_create(&servidor, NULL, servidorPrincipal, puertoMemoria);

	// nivel_gui_inicializar();
	// NIVEL* navePrincipal = nivel_crear("Nave Principal");
	// nivel_gui_dibujar(navePrincipal);
	// //pthread_t mapa;
	// //pthread_create(&mapa, NULL, iniciarMapa, NULL);
	// //pthread_join(mapa, NULL);
	 
	pthread_join(servidor, NULL);
	free(memoriaPrincipal);

	return 0; 
}

void leerConfig(){

	t_config * config = config_create("./cfg/miram.config");
	esquemaMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA"); 
	algoritmoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	tamanioMemoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	puertoMemoria = config_get_string_value(config, "PUERTO");
	tamanioPagina = config_get_int_value(config, "TAMANIO_PAGINA");
	path_SWAP = config_get_string_value(config, "PATH_SWAP");

}

void servidorPrincipal() {
	int listeningSocket = crear_conexionServer(puertoMemoria);

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
		char* tareas = malloc(tamanioTareas + 1);
		memcpy(tareas, stream, tamanioTareas);
		stream += tamanioTareas;

		string_append(&tareas, "|");
		tamanioTareas++;

		printf("%s \n", tareas);

		//Deserializar TCBs
		int cantidadTCBs = 0;
		memcpy(&cantidadTCBs, stream, sizeof(int));
		stream += sizeof(int); //lo que sigue en el stream son los tcbs

		//Nos aseguramos de que hay espacio para recibir la patota 

		int hayLugar = buscarEspacioNecesario(tamanioTareas, cantidadTCBs); 
		

		printf("cantidad tcbs: %d \n", cantidadTCBs);

		if(hayLugar){

			if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
				int memoriaNecesaria = SIZEOF_PCB + tamanioTareas + SIZEOF_TCB * cantidadTCBs;
				int framesNecesarios = divisionRedondeadaParaAriba(memoriaNecesaria, tamanioPagina);
				void * streamPatota = malloc(memoriaNecesaria);

				t_list * tablaDePaginas = list_create();

				PCB * pcb = crearPCB();

				int offset = 0;
				memcpy(streamPatota + offset, &(pcb->pid), sizeof(uint32_t) );
				offset += sizeof(uint32_t);
				memcpy(streamPatota + offset, &(pcb->tareas), sizeof(uint32_t) );
				offset += sizeof(uint32_t);
				memcpy(streamPatota, (void *) tareas, tamanioTareas);
				offset += tamanioTareas;



				llenarFramesConPatota(tablaDePaginas, streamPatota, framesNecesarios, cantidadTCBs, tamanioTareas);
			}

			if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

			}

			
			// uint32_t direccionPCB = asignarMemoria(pcb); 

			// uint32_t direccionTareas = asignarMemoriaTareas(tareas); 
			
			// 	//preguntar como asignar direccion de tareas  


			// for(int i = 0 ; i < cantidadTCBs ;  i++ ){
			// TCB * tripulante = malloc(sizeof(TCB));
			// //tripulante = deserializar_TCB(stream);

			// //Deserializamos los campos que tenemos en el buffer
			// memcpy(&(tripulante->tid), stream, sizeof(uint32_t));
			// stream += sizeof(uint32_t);

			// memcpy(&(tripulante->posicionX), stream, sizeof(uint32_t));
			// stream += sizeof(uint32_t);

			// memcpy(&(tripulante->posicionY), stream, sizeof(uint32_t));
			// stream += sizeof(uint32_t);

			// memcpy(&(tripulante->estado), stream, sizeof(char));
			// stream += sizeof(char);

			// tripulante->punteroPCB = direccionPCB; 
			// tripulante->proximaInstruccion = direccionTareas; 

			// uint32_t direccionLogica = asignarMemoria(tripulante); 

			// printf("ID: %d \n", tripulante->tid);
			// }

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

PCB * crearPCB(){
	// uint32_t punteroTareas = *tareas; //ESTO ESTA RARI

	PCB * patota = malloc(sizeof(PCB));
	patota->pid = proximoPID;
	patota->tareas = SIZEOF_PCB;
	
	//deberia tener un semaforo mutex
	proximoPID++;
	
	return patota; 
}


// ------------------------------------------------------ MAPA ----------------------------------------------

void iniciarMapa() {
	
	nivel_gui_inicializar();
	NIVEL* navePrincipal = nivel_crear("Nave Principal");
	nivel_gui_dibujar(navePrincipal);

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


// ------------ASIGNAR MEMORIA-------------

void iniciarMemoria() {
	
	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		listaFrames = list_create();
		listaTablasDePaginas = list_create();
		iniciarFrames();	
	}
}

int buscarEspacioNecesario(int tamanioTareas, int cantidadTripulantes) {

	// if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
	// 	t_list * copiaSegmentosOcupados = list_duplicate(tablaSegmentosGlobal); 
	// 	t_list * copiaSegmentosLibres = obtenerSegmentosLibres(copiaSegmentosOcupados);

	// 	bool cabenTareasEnElSegmento(t_segmento * segmento){

	// 		if((segmento->tamanio) >= tamanioTareas){
	// 			return true; 
	// 		}
	// 		else{
	// 			return false; 
	// 		}
	// 	}
		
		
	// 	t_segmento * segmentoPosible = malloc(sizeof(t_segmento));
	// 	segmentoPosible = list_find(copiaSegmentosLibres, cabenTareasEnElSegmento); 
		
	// 	if(segmentoPosible != NULL){
	// 		t_segmento * copiaSegmentoTareas = malloc(sizeof(t_segmento)); 
	// 		copiaSegmentoTareas -> base = segmentoPosible -> base; 
	// 		copiaSegmentoTareas -> tamanio = tamanioTareas; 
	// 		list_add(copiaSegmentosOcupados, copiaSegmentoTareas); 
	// 	}
	// 	else{
	// 		return -1; 
	// 	}

	// 	for(int i = 0 ; i < cantidadTripulantes; i ++){

	// 		copiaSegmentosLibres = obtenerSegmentosLibres(copiaSegmentosOcupados); 
	// 		segmentoPosible = list_find(copiaSegmentosLibres, cabeTCB); 
	// 		if(segmentoPosible != NULL){
	// 			t_segmento * copiaSegmentoTCB = malloc(sizeof(TCB)); 
	// 			copiaSegmentoTCB -> base = segmentoPosible -> base; 
	// 			copiaSegmentoTCB -> tamanio = sizeof(TCB); 
	// 			list_add(copiaSegmentosOcupados, copiaSegmentoTCB); 
	// 		}
	// 		else{
	// 			return -1; 
	// 		}

	// 	}

	// 	copiaSegmentosLibres = obtenerSegmentosLibres(copiaSegmentosOcupados); 
	// 	segmentoPosible = list_find(copiaSegmentosLibres, cabePCB); 
	// 	if(segmentoPosible != NULL){
	// 		return 1;  

	// 	}
	// 	else{
	// 		return -1; 
	// 	}
	// }

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		int cantidadMemoriaNecesaria = tamanioTareas + SIZEOF_TCB * cantidadTripulantes + SIZEOF_PCB;
		int cantidadFramesDisponibles = framesDisponibles();

		if (divisionRedondeadaParaAriba(cantidadMemoriaNecesaria, tamanioPagina) < cantidadFramesDisponibles) {
			return 1;
		}
		
		return -1;

	}	

} 

uint32_t asignarMemoria(void * contenido){

	// if(strcmp(esquemaMemoria, "PAGINACION") == 0){
	// 	asignarMemoriaPaginacion(contenido); 
	// }

	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0){
		asignarMemoriaSegmentacion(contenido); 
	}
}

uint32_t asignarMemoriaTareas(char * tareas){

	// if(strcmp(esquemaMemoria, "PAGINACION") == 0){
	// 	asignarMemoriaTareasPaginacion(tareas); 
	// }

	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0){
		asignarMemoriaTareasSegmentacion(tareas); 
	}
}

uint32_t asignarMemoriaSegmentacion(void * contenido){

}

uint32_t asignarMemoriaTareasSegmentacion(char * contenido){
	
}

// -------------------- PAGINACION ------------------------------

void iniciarFrames(){
	printf("--------------------------------------------------------------------- \n");
	printf("Iniciando Frames... \n\n");
	int cantidadFrames = 0;

	for(int desplazamiento = 0; desplazamiento< tamanioMemoria; desplazamiento += tamanioPagina){
		t_frame * frame = malloc(sizeof(t_frame));
		frame->inicio = desplazamiento;
		frame->ocupado = 0;

		list_add(listaFrames, frame);
		cantidadFrames ++;
	}

	  	printf("--------------------------------------------------------------------- \n");
		printf("Disponibilidad de frames: \n");
		int x = 0;
		void estaOcupado(t_frame * frame) {
			printf("el frame %d esta ocupado: %d \n", x, frame->ocupado);
			x++;
		};
		list_iterate(listaFrames, estaOcupado);

	return;
}

int framesDisponibles() {
	int libre = 0;
	void estaLibre(t_frame * frame) {
    	if (frame->ocupado == 0){ libre++; }
  	}
	list_iterate(listaFrames, estaLibre);
	return libre;
}

uint32_t buscarFrame() {

  bool estaLibre(t_frame * frame) {
    return frame->ocupado == 0;
  }

  t_frame * frameLibre = malloc(sizeof(t_frame));
  frameLibre = list_find(listaFrames, estaLibre);
  uint32_t direccionFrame = frameLibre->inicio;
  free(frameLibre);

  return direccionFrame;
}

void llenarFramesConPatota(t_list * tablaDePaginas, void * streamDePatota, int cantidadFrames, int cantidadTCBs, int longitudTareas) {

	int i = 0, j = 0;

	uint32_t direcProximoFrame = buscarFrame();
	for (i = 0; i < cantidadFrames; i++){
		printf("direc prox frame a escribir %d \n", direcProximoFrame);

		for (j = 0; j < tamanioPagina; j++) {
			memcpy( memoriaPrincipal + (direcProximoFrame + j) , (char *)streamDePatota + (j + i*tamanioPagina), 1); //copio byte a byte (ojala que ande)
		}

		uint32_t numeroDeFrame = direcProximoFrame / tamanioPagina;  // no hace falta redondear porque la division de int redondea pra abajo
		t_frame * frameOcupado = malloc(sizeof(t_frame));
		frameOcupado->inicio = direcProximoFrame;
		frameOcupado->ocupado = 1;
		t_frame * frameParaLiberar = list_replace(listaFrames, numeroDeFrame, frameOcupado);  // ver si se puede usar replace and destroy para liberar memoria
		//free(frameParaLiberar);

		t_pagina * pagina = malloc(sizeof(t_pagina));
		pagina->numeroFrame = numeroDeFrame;
		pagina->numeroPagina = (uint32_t) i;
		list_add(tablaDePaginas, pagina);

		direcProximoFrame = buscarFrame();
	}

	list_add(listaTablasDePaginas, tablaDePaginas);

}

int divisionRedondeadaParaAriba(int x, int y) {
	return (x -1)/y +1;
}
