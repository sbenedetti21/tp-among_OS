#include "mi_ram.h"


 
NIVEL* navePrincipal;


int main(int argc, char ** argv){

	loggerMiram = log_create("miram.log", "mi_ram.c", 0, LOG_LEVEL_INFO);
	leerConfig();
	sem_init(&mutexProximoPID, 0, 1);

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
		char* tareas = malloc(tamanioTareas + 2);
		memcpy(tareas, stream, tamanioTareas);
		stream += tamanioTareas;

		char pipe = '|';
		memcpy(tareas + tamanioTareas, &pipe, 1);	
		tamanioTareas++;

		log_info(loggerMiram, "%s", tareas);
		printf("%s \n", tareas); // tira un invalid read valgrind o unitialised values

		//Deserializar CantidadDeTCBs
		int cantidadTCBs = 0;
		memcpy(&cantidadTCBs, stream, sizeof(int));
		stream += sizeof(int); //lo que sigue en el stream son los tcbs

		log_info(loggerMiram, "cantidad tcbs recibidos: %d \n", cantidadTCBs);

		//Nos aseguramos de que hay espacio para recibir la patota 
		int hayLugar = buscarEspacioNecesario(tamanioTareas, cantidadTCBs); 
		

		log_info(loggerMiram, "hay lugar disponible (1 -> si, -1 -> no): %d \n", hayLugar);

		if(hayLugar){

			if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
				int memoriaNecesaria = SIZEOF_PCB + tamanioTareas + SIZEOF_TCB * cantidadTCBs + 8;
				int framesNecesarios = divisionRedondeadaParaArriba(memoriaNecesaria, tamanioPagina);
				void * streamPatota = malloc(memoriaNecesaria);

				t_list * tablaDePaginas = list_create();

				referenciaTablaPatota * referenciaPatota = malloc(sizeof(referenciaTablaPatota));

	
				PCB * pcb = crearPCB();

				int pid = pcb->pid;

				int offset = 0;
				memcpy(streamPatota + offset, &(pcb->pid), sizeof(uint32_t) );
				offset += sizeof(uint32_t);
				memcpy(streamPatota + offset, &(pcb->tareas), sizeof(uint32_t) );
				offset += sizeof(uint32_t);
				memcpy(streamPatota + offset, tareas, tamanioTareas);
				offset += tamanioTareas;


				uint32_t proximaInstruccion = 0, direccionPCB = 0;
				uint32_t idTripulante[cantidadTCBs];
				for (int i = 0; i < cantidadTCBs; i++) {

					memcpy(streamPatota + offset, stream, SIZEOF_TCB);
					stream += SIZEOF_TCB; offset += (SIZEOF_TCB - 8);
					//memcpy(streamPatota + offset, &proximaInstruccion, sizeof(uint32_t));
					memset(streamPatota + offset, 0, sizeof(uint32_t));
					offset += sizeof(uint32_t);
					//memcpy(streamPatota + offset, &direccionPCB, sizeof(uint32_t));
					memset(streamPatota + offset, 0, sizeof(uint32_t));
					offset += sizeof(uint32_t);
				};
			
				

				llenarFramesConPatota(tablaDePaginas, streamPatota, framesNecesarios, cantidadTCBs, tamanioTareas, memoriaNecesaria);

				mem_hexdump(memoriaPrincipal, 2048);

				// void mostrarCosas(char * key, void * value) {
				// 	printf("%c %d \n", key, *(int*)value);
				// }
				// dictionary_iterator(diccionarioTripulantes, mostrarCosas);
				// mostrarMemoriaInt(20, 17, 1);
				// mostrarMemoriaChar(21, 1, 1);
				// mostrarMemoriaInt(21, 2, 4);

				// mostrarMemoriaInt(21, 18, 1);
				// mostrarMemoriaChar(22, 2, 1);
				// mostrarMemoriaInt(22, 3, 4);
				free(streamPatota);
			}

			if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

				t_list * tablaSegmentos = list_create();
				PCB * pcb = crearPCB();
				 
				printf("ID DE LA PATOTA %d \n", pcb->pid);
				uint32_t direccionTareas = asignarMemoriaSegmentacionTareas(tareas, tamanioTareas, tablaSegmentos); 
				pcb ->tareas = direccionTareas; 

				void * streamPCB = malloc(SIZEOF_PCB);
				memcpy(streamPCB, &(pcb->pid), sizeof(uint32_t)); 
				memcpy(streamPCB + sizeof(uint32_t), &(pcb->tareas), sizeof(uint32_t)); 
				uint32_t direccionPCB = asignarMemoriaSegmentacionPCB(streamPCB, tablaSegmentos); 

				for(int i = 0 ; i < cantidadTCBs ;  i++ ){
					void * tripulante = malloc(SIZEOF_TCB);
					int offset = 0; 
					//Deserializamos los campos que tenemos en el buffer
					memcpy(tripulante + offset, stream, sizeof(uint32_t));
					stream += sizeof(uint32_t);
					offset = offset + sizeof(uint32_t);

					memcpy(tripulante + offset, stream, sizeof(uint32_t));
					stream += sizeof(uint32_t);
					offset = offset + sizeof(uint32_t); 

					memcpy(tripulante + offset, stream, sizeof(uint32_t));
					stream += sizeof(uint32_t);
					offset = offset + sizeof(uint32_t); 

					memcpy(tripulante + offset, stream, sizeof(char));
					stream += sizeof(char);
					offset = offset + sizeof(char); 

					memcpy(tripulante + offset, &direccionTareas, sizeof(uint32_t)); 
					offset = offset + sizeof(uint32_t);

					memcpy(tripulante + offset, &direccionPCB, sizeof(uint32_t));

					
					uint32_t direccionLogica = asignarMemoriaSegmentacionTCB(tripulante, tablaSegmentos); 
					//log_info(loggerMiram, "Asigno al tripulante %d la dirección logica %d \n", tripulante->tid, direccionLogica);
					
			
				}

			referenciaTablaPatota * referencia = malloc(sizeof(referenciaTablaPatota)); 
			referencia ->pid = pcb -> pid; 
			referencia->tablaPatota = tablaSegmentos;

			sem_wait(&mutexTablaDeTablas);
			list_add(tablaDeTablasSegmentos, referencia); 
			sem_post(&mutexTablaDeTablas);

				int proximaTarea = obtenerProximaTarea(direccionTareas);
				int proximaTarea1 = obtenerProximaTarea(proximaTarea);

				PCB * pcbObtenido = malloc(SIZEOF_PCB);

				memcpy(pcbObtenido, memoriaPrincipal + direccionPCB, SIZEOF_PCB); 

				printf("Me trae el PCB id %d cuyas tareas están en %d \n", pcbObtenido ->pid, pcbObtenido->tareas); 
				printf("Direccion de las tareas %d \n", direccionTareas);
				imprimirSegmentosLibres();
				/*ENCONTRAR UNA TABLA DE PATOTA POR SU REFERENCIA EN LA TABLA DE TABLAS DE SEGMENTO */
				bool coincidePatota(referenciaTablaPatota * referencia){
					return (referencia->pid == pcb -> pid);
				}

			referenciaTablaPatota * referenciaObtenida = list_find(tablaDeTablasSegmentos, coincidePatota); 

			t_list * tablaObtenida = referenciaObtenida->tablaPatota; 
			t_segmento * segmentoObtenido = list_get(tablaObtenida, 1); 

			printf("La direccion logica de las tareas es %d \n EL tamaño de las tareas %d \n", segmentoObtenido ->base, segmentoObtenido ->tamanio);



				
			}



			//mem_hexdump(memoriaPrincipal, 2048);

		}
		

		break;

	case PEDIR_TAREA: ;

		/*
		uint32_t idTripulante = 0;
		memcpy(&idTripulante, paquete->buffer->stream, sizeof(uint32_t));
		//buscarTripulante
		
		char * tarea = obtenerProximaTarea(); //modificar puntero instruccion en tripulante
		
		// meter en un paquete
		int header;
		int longitudStreamTarea;
		void * streamTarea;

		send(socketCliente, )
		 */ 

		break;
	
	default:	



		break;
	}

	// free(paquete->buffer->stream);
	// free(paquete->buffer);
	// free(paquete);
	


}



int obtenerProximaTarea(int direccionLogica){
	char * tareaObtenida = malloc(40); 
	char caracterComparacion = 'a'; 
	int desplazamiento = 0; 
	
	
	while(caracterComparacion != '\n' && caracterComparacion != '|'){

	memcpy(tareaObtenida + desplazamiento, memoriaPrincipal+direccionLogica+desplazamiento, 1 ); 
	
	memcpy(&caracterComparacion, memoriaPrincipal + direccionLogica + desplazamiento + 1, 1); 
	desplazamiento ++; 
	
	}
	direccionLogica = direccionLogica + desplazamiento;

	printf("La proxima tarea es %s \n \n", tareaObtenida); 
	log_info(loggerMiram,"Proxima tarea: %s", tareaObtenida);
	printf("Caracteres leidos %d \n ", desplazamiento);

	//actualizarProximaTarea(tcb, direccionLogica); 
	return direccionLogica;
} 


TCB * deserializar_TCB(void * stream){ 
	TCB * tripulante = malloc(SIZEOF_TCB);

	
	//Deserializamos los campos que tenemos en el buffer
	memcpy(&(tripulante->tid), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	printf("tripulante id deserializaicon %d\n", tripulante->tid);

	memcpy(&(tripulante->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tripulante->estado), stream, sizeof(char));
	stream += sizeof(char);

	return tripulante;
}

PCB * crearPCB(){

	PCB * patota = malloc(SIZEOF_PCB);
	patota->pid = proximoPID;
	patota->tareas = SIZEOF_PCB;
	
	sem_wait(&mutexProximoPID); 
	proximoPID++;
	sem_post(&mutexProximoPID); 


	
	return patota; 
}





// ------------ASIGNAR MEMORIA-------------

void iniciarMemoria() {
	
	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		tablaSegmentosGlobal = list_create(); 
		tablaDeTablasSegmentos = list_create();
		sem_init(&mutexTablaGlobal, 0, 1);
		sem_init(&mutexTablaDeTablas, 0, 1);
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		listaFrames = list_create();
		listaTablasDePaginas = list_create();
		pthread_mutex_init(&mutexMemoriaPrincipal, NULL);
		pthread_mutex_init(&mutexListaTablas, NULL);
		//diccionarioTripulantes = dictionary_create(); // mirar si el tamaño alcanza
		iniciarFrames();	
	}
}


int buscarEspacioNecesario(int tamanioTareas, int cantidadTripulantes) {

	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

		return buscarEspacioSegmentacion(tamanioTareas, cantidadTripulantes);
		
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		int cantidadMemoriaNecesaria = tamanioTareas + SIZEOF_TCB * cantidadTripulantes + SIZEOF_PCB;
		int cantidadFramesDisponibles = framesDisponibles();

		if (divisionRedondeadaParaArriba(cantidadMemoriaNecesaria, tamanioPagina) < cantidadFramesDisponibles) {
			return 1;
		}
		
		return -1;

	}	

} 

//ver como generalizar el asignar memoria

// -------------------- PAGINACION ------------------------------

void iniciarFrames(){
	log_info(loggerMiram, "Iniciando Frames... ");
	int cantidadFrames = 0;

	for(int desplazamiento = 0; desplazamiento< tamanioMemoria; desplazamiento += tamanioPagina){
		t_frame * frame = malloc(sizeof(t_frame));
		frame->inicio = desplazamiento;
		frame->ocupado = 0;

		list_add(listaFrames, frame);
		cantidadFrames ++;
	}
		log_info(loggerMiram, "-------------------------");
		log_info(loggerMiram, "Disponibilidad de frames: ");
		int x = 0;
		void estaOcupado(t_frame * frame) {
			log_info(loggerMiram, "el frame %d esta ocupado: %d ", x, frame->ocupado);
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
  //free(frameLibre);

  return direccionFrame;
}

void llenarFramesConPatota(t_list * tablaDePaginas, void * streamDePatota, int cantidadFrames, int cantidadTCBs, int longitudTareas, int memoriaAGuardar) {

	int i = 0, j = 0;

	for (i = 0; i < cantidadFrames; i++){
		uint32_t direcProximoFrame = buscarFrame();
		printf("direc prox frame a escribir %d \n", direcProximoFrame);

		pthread_mutex_lock(&mutexMemoriaPrincipal);
		while ((i * tamanioPagina + j) < (memoriaAGuardar) && j < tamanioPagina){
			memcpy( memoriaPrincipal + (direcProximoFrame + j) , streamDePatota + (j + i*tamanioPagina), 1); //copio byte a byte (ojala que ande)	
			j++;
		}
		pthread_mutex_unlock(&mutexMemoriaPrincipal);

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
		j = 0;
	}

	pthread_mutex_lock(&mutexListaTablas);
	list_add(listaTablasDePaginas, tablaDePaginas);
	pthread_mutex_unlock(&mutexListaTablas);
}

int divisionRedondeadaParaArriba(int x, int y) {
	return (x -1)/y +1;
}


//----------------SEGMENTACION

uint32_t asignarMemoriaSegmentacionTCB(void * tripulante, t_list * tablaSegmentos){

		//busco un lugar de memoria (segun algoritmo)
		uint32_t direccionLogica = 0;
		
		direccionLogica = encontrarLugarSegmentacion(SIZEOF_TCB); 
		//le asigno el lugar de memoria encontrado
		memcpy(memoriaPrincipal + direccionLogica, tripulante, sizeof(SIZEOF_TCB)); 
		

		//creo el segmento para la estructura nueva
		t_segmento * segmentoNuevo = malloc(sizeof(t_segmento)); 
		segmentoNuevo -> tamanio = SIZEOF_TCB; 
		segmentoNuevo -> base = direccionLogica; 

		//agrego el segmento a la tabla de segmentos 
		list_add(tablaSegmentos, segmentoNuevo);
		sem_wait(&mutexTablaGlobal);
		list_add(tablaSegmentosGlobal, segmentoNuevo);  
		sem_post(&mutexTablaGlobal);
		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionPCB(void * pcb , t_list * tablaSegmentos){
	//busco un lugar de memoria (segun algoritmo)
		
		int direccionLogica = encontrarLugarSegmentacion(SIZEOF_PCB); 
		//le asigno el lugar de memoria encontrado
		memcpy(memoriaPrincipal + direccionLogica , pcb, SIZEOF_PCB); 
		
		//creo el segmento para la estructura nueva
		t_segmento * segmentoNuevo = malloc(sizeof(t_segmento)); 
		segmentoNuevo -> tamanio = SIZEOF_PCB; 
		segmentoNuevo -> base = direccionLogica; 

		//agrego el segmento a la tabla de segmentos 
		list_add_in_index(tablaSegmentos, 0,segmentoNuevo);
		sem_wait(&mutexTablaGlobal);
		list_add(tablaSegmentosGlobal, segmentoNuevo); 
		sem_post(&mutexTablaGlobal);
		printf("Direccion logica asignada %d \n", direccionLogica);  
		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionTareas(char * tareas, int tamanioTareas, t_list * tablaSegmentos){

	uint32_t direccionLogica = encontrarLugarSegmentacion(tamanioTareas); 
	memcpy(memoriaPrincipal + direccionLogica, tareas, tamanioTareas); 

	t_segmento * segmentoTareas = malloc(tamanioTareas); 
	segmentoTareas->tamanio = tamanioTareas; 
	segmentoTareas ->base = direccionLogica; 
	list_add(tablaSegmentos, segmentoTareas);
	sem_wait(&mutexTablaGlobal);
	list_add(tablaSegmentosGlobal, segmentoTareas);  
	sem_post(&mutexTablaGlobal);
	return direccionLogica;

}

uint32_t encontrarLugarSegmentacion(int tamanioSegmento){
	if(strcmp(algoritmoReemplazo, "FIRST_FIT") == 0){return firstFit(tamanioSegmento); }
	if(strcmp(algoritmoReemplazo, "BEST_FIT") == 0){ return bestFit(tamanioSegmento);}
	//es realmente un algoritmo de reemplazo para segmentacion¿? o algoritmo de busqueda de lugar 
}

uint32_t firstFit(int tamanioContenido){
	int i = 0; 
	t_list * segmentosLibres = obtenerSegmentosLibres(tablaSegmentosGlobal); 
	t_segmento * segmentoLibre = list_get(segmentosLibres, i); 
	
	while((segmentoLibre -> tamanio) < tamanioContenido){
		i++; 
		segmentoLibre = list_get(segmentosLibres, i); 
	}
	printf("La base del primer segmento libre es : %d \n", (segmentoLibre -> base));
	printf("El tamanio del primer segmento libre es %d \n", segmentoLibre->tamanio);

	
	return (segmentoLibre -> base);

}

uint32_t bestFit(int tamanioContenido){

	int i = 0;  
	t_list * segmentosLibres = obtenerSegmentosLibres(tablaSegmentosGlobal); 
	t_segmento * segmentoLibre = malloc(sizeof(t_segmento)); 
	t_list * lugaresPosibles = list_create(); 

	while(i < (list_size(segmentosLibres))){
		segmentoLibre = list_get(segmentosLibres, i); 
		if((segmentoLibre ->tamanio) > tamanioContenido){
			list_add(lugaresPosibles, segmentoLibre); 
		}

		i++;

	}

	list_sort(lugaresPosibles, segmentoMasPequenio); 
	t_segmento * segmentoElegido = list_get(lugaresPosibles, 0); 
	

	return (segmentoElegido -> base);  

}

t_list *  obtenerSegmentosLibres(t_list * tablaSegmentos){
	int i = 0; 
	t_list * segmentosLibres = list_create(); 

	if(list_size(tablaSegmentos) == 0){
		t_segmento * segmento = malloc(sizeof(t_segmento)); 
		segmento-> base = 0 ; 
		segmento-> tamanio = tamanioMemoria; 
		list_add(segmentosLibres, segmento);
		return segmentosLibres;
	}


	list_sort(tablaSegmentos, seEncuentraPrimeroEnMemoria); 
	t_segmento * primerSegmentoOCupado = malloc(sizeof(t_segmento));
	primerSegmentoOCupado = list_get(tablaSegmentos, 0);
	

	if((primerSegmentoOCupado -> base) != 0){
		t_segmento * primerSegmentoLibre = malloc(sizeof(t_segmento)); 
		primerSegmentoLibre -> base = 0; 
		primerSegmentoLibre -> tamanio = (primerSegmentoOCupado -> base)  - (primerSegmentoLibre -> base); 
		list_add(segmentosLibres, primerSegmentoLibre); 
	} 

	while(i < (list_size(tablaSegmentos)-1)){
		t_segmento * segmentoActual = malloc(sizeof(t_segmento)); 
		t_segmento * segmentoLibre = malloc(sizeof(t_segmento));
		t_segmento * proximoSegmento = malloc(sizeof(t_segmento));
		segmentoActual = list_get(tablaSegmentos, i); 
		proximoSegmento = list_get(tablaSegmentos, i +1);
		segmentoLibre->base = segmentoActual->base + segmentoActual-> tamanio ; 
		segmentoLibre-> tamanio = (proximoSegmento -> base) - segmentoLibre -> base;
		if((segmentoLibre -> tamanio) != 0) {
				list_add(segmentosLibres, segmentoLibre); 

		}

		i++;
		
	}

	t_segmento * ultimoSegmentoOcupado = malloc(sizeof(t_segmento));  
	ultimoSegmentoOcupado = list_get(tablaSegmentos, i); 
	int finalUltimoSegmento = ultimoSegmentoOcupado ->base + ultimoSegmentoOcupado -> tamanio; 


	if(finalUltimoSegmento < tamanioMemoria){
		t_segmento * ultimoSegmentoLibre = malloc(sizeof(t_segmento));
		ultimoSegmentoLibre -> base = ultimoSegmentoOcupado ->base + ultimoSegmentoOcupado ->tamanio ; 
		ultimoSegmentoLibre -> tamanio = tamanioMemoria - (ultimoSegmentoLibre -> base); 
		
		list_add(segmentosLibres, ultimoSegmentoLibre);
	}

	return segmentosLibres;

}

void imprimirSegmentosLibres(){
	t_list * segmentosLibres = obtenerSegmentosLibres(tablaSegmentosGlobal);
	for(int i = 0; i < (list_size(segmentosLibres)); i++){
	   t_segmento * segmento = list_get(segmentosLibres, i); 
	   printf("Segmento libre que empieza en %d y termina en %d \n", segmento->base, segmento->base + segmento->tamanio); 
	}

}

int buscarEspacioSegmentacion(int tamanioTareas, int cantidadTripulantes){
t_list * copiaSegmentosOcupados = list_duplicate(tablaSegmentosGlobal); 
t_list * copiaSegmentosLibres = obtenerSegmentosLibres(copiaSegmentosOcupados);

		bool cabenTareasEnElSegmento(t_segmento * segmento){

			if((segmento->tamanio) >= tamanioTareas){
				return true; 
			}
			else{
				return false; 
			}
		}
		
		
		t_segmento * segmentoPosible = malloc(sizeof(t_segmento));
		segmentoPosible = list_find(copiaSegmentosLibres, cabenTareasEnElSegmento); 
		
		if(segmentoPosible != NULL){
			t_segmento * copiaSegmentoTareas = malloc(sizeof(t_segmento)); 
			copiaSegmentoTareas -> base = segmentoPosible -> base; 
			copiaSegmentoTareas -> tamanio = tamanioTareas; 
			list_add(copiaSegmentosOcupados, copiaSegmentoTareas); 
		}
		else{
			return -1; 
		}

		for(int i = 0 ; i < cantidadTripulantes; i ++){

			copiaSegmentosLibres = obtenerSegmentosLibres(copiaSegmentosOcupados); 
			segmentoPosible = list_find(copiaSegmentosLibres, cabeTCB); 
			if(segmentoPosible != NULL){
				t_segmento * copiaSegmentoTCB = malloc(SIZEOF_TCB); 
				copiaSegmentoTCB -> base = segmentoPosible -> base; 
				copiaSegmentoTCB -> tamanio = SIZEOF_TCB;
				list_add(copiaSegmentosOcupados, copiaSegmentoTCB); 
			}
			else{
				return -1; 
			}

		}

		copiaSegmentosLibres = obtenerSegmentosLibres(copiaSegmentosOcupados); 
		segmentoPosible = list_find(copiaSegmentosLibres, cabePCB); 
		if(segmentoPosible != NULL){
			return 1;  

		}
		else{
			return -1; 
		}
	}

bool seEncuentraPrimeroEnMemoria(t_segmento * unSegmento, t_segmento* otroSegmento){

	 if((unSegmento->base) <( otroSegmento->base)){
		 return true; 
	 }
	 else{
		 return false; 
	 }
}

bool segmentoMasPequenio(t_segmento * unSegmento, t_segmento * otroSegmento){

	if((unSegmento ->tamanio < (otroSegmento->tamanio))){
		return true; 
	}

	else{
		return false; 
	}
}

bool cabePCB(t_segmento * segmento){
	if( (segmento->tamanio) >= SIZEOF_PCB){
		return true; 
	}

	else {return false; }
}


bool cabeTCB(t_segmento * segmento){
	if( (segmento->tamanio) >= SIZEOF_TCB){
		return true; 
	}

	else {return false; }
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
