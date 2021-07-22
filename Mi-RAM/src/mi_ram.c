#include "mi_ram.h"




int main(int argc, char ** argv){
	navePrincipal = nivel_crear("Nave Principal");
	loggerMiram = log_create("miram.log", "mi_ram.c", 0, LOG_LEVEL_INFO);
	leerConfig();

	memoriaPrincipal = malloc(tamanioMemoria);
	memset(memoriaPrincipal, 0, tamanioMemoria);
	iniciarMemoria();
	
	pthread_t servidor;
	pthread_t senial1;
	pthread_t senial2;
	pthread_create(&servidor, NULL, servidorPrincipal, puertoMemoria);
	pthread_create(&senial1, NULL, hiloSIGUSR1, NULL); 
	pthread_create(&senial2, NULL, hiloSIGUSR2, NULL);

	

	// pthread_t mapa;
	// pthread_create(&mapa, NULL, iniciarMapa, NULL);
	// pthread_join(mapa, NULL);
	// nivel_destruir(navePrincipal); 
	// nivel_gui_terminar();
	
	//pthread_t mapa;
	//pthread_create(&mapa, NULL, iniciarMapa, NULL);
	//pthread_join(mapa, NULL);

	// nivel_destruir(navePrincipal);
	// nivel_gui_terminar();
		
	pthread_join(servidor, NULL);
	
		(memoriaPrincipal);

	return 0; 
}

void leerConfig(){

	t_config * config = config_create("./cfg/miram.config");
	esquemaMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA"); 
	algoritmoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	criterioSeleccion = config_get_string_value(config, "CRITERIO_SELECCION");
	tamanioMemoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	puertoMemoria = config_get_string_value(config, "PUERTO");
	tamanioPagina = config_get_int_value(config, "TAMANIO_PAGINA");
	path_SWAP = config_get_string_value(config, "PATH_SWAP");
	tamanioSwap = config_get_int_value(config, "TAMANIO_SWAP");

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

	if(! BUFFER_RECV){ log_error(loggerMiram,"No se pudo recibir el buffer");} else {
		log_info(loggerMiram, "Cantidad de bytes recibidos en el buffer: %d", BUFFER_RECV);
	}
	

	void* stream = malloc(paquete->buffer->size);
	stream = paquete->buffer->stream;

	uint32_t idPatota = 0;

	switch (paquete->header)
	{
	case INICIAR_PATOTA: ; 
		
		// Deserializamos tareas
		int tamanioTareas;
		memcpy(&tamanioTareas, stream, sizeof(int));
		stream += sizeof(int);
		char* tareas = malloc(tamanioTareas + 2); //mirar si no es +1
		memcpy(tareas, stream, tamanioTareas);
		stream += tamanioTareas;

		char pipe = '|';
		memcpy(tareas + tamanioTareas, &pipe, 1);	
		tamanioTareas++;
		

		log_info(loggerMiram, "%s", tareas);

		//deserializar pid 
		idPatota = 0; 
		memcpy(&idPatota, stream, sizeof(uint32_t)); 
		stream += sizeof(uint32_t);

		//Deserializar CantidadDeTCBs
		int cantidadTCBs = 0;
		memcpy(&cantidadTCBs, stream, sizeof(int));
		stream += sizeof(int); //lo que sigue en el stream son los tcbs

		log_info(loggerMiram, "cantidad tcbs recibidos: %d \n", cantidadTCBs);

		//Nos aseguramos de que hay espacio para recibir la patota 
		int hayLugar = buscarEspacioNecesario(tamanioTareas, cantidadTCBs); 
		

		log_info(loggerMiram, "hay lugar disponible (1 -> si, -1 -> no): %d \n", hayLugar);

		if(hayLugar == -1){
			log_info(loggerMiram, "Rechazo patota por falta de espacio en memoria");
					
		}
			
				
		if(hayLugar == 1){
			log_info(loggerMiram, "agrego patota");
				

			if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
				int memoriaNecesaria = SIZEOF_PCB + tamanioTareas + SIZEOF_TCB * cantidadTCBs; // +8
				int framesNecesarios = divisionRedondeadaParaArriba(memoriaNecesaria, tamanioPagina);
				void * streamPatota = malloc(memoriaNecesaria);
				memset(streamPatota, 0, memoriaNecesaria);
				
				t_list * tablaDePaginas = list_create();
								
	
				PCB * pcb = crearPCB(idPatota);

				int pid = pcb->pid;
				
				int offset = 0;
				memcpy(streamPatota + offset, &(pcb->pid), sizeof(uint32_t) );
				offset += sizeof(uint32_t);
				memcpy(streamPatota + offset, &(pcb->tareas), sizeof(uint32_t) );
				offset += sizeof(uint32_t);
				memcpy(streamPatota + offset, tareas, tamanioTareas);
				offset += tamanioTareas;



				uint32_t proximaInstruccion = 8, direccionPCB = 0;
				for (int i = 0; i < cantidadTCBs; i++) {
					// tripu->idPatota = pid;
					// tripu->longitudTareas = tamanioTareas;
					int tid = 0;
					memcpy(&tid, stream, 4);
					uint32_t x = 0;
					uint32_t y = 0;
				
					memcpy(&x, stream + 4, 4);
					memcpy(&y, stream + 8, 4);
					agregarTripulanteAlMapa(tid,x,y);
				// Calculo de pagina y offset de los tripulantes

					t_tripulantePaginacion * tripu = malloc(sizeof(t_tripulantePaginacion)); 
					tripu->tid = tid;
					tripu->pid = idPatota;
					tripu->nroPagina = (SIZEOF_PCB + tamanioTareas + SIZEOF_TCB * i) / tamanioPagina;
					tripu->offset = (SIZEOF_PCB + tamanioTareas + SIZEOF_TCB * i) % tamanioPagina;
					tripu->cantidadDePaginas = ((tripu->offset + SIZEOF_TCB) / tamanioPagina) + 1;

					log_info(loggerMiram, "Tripulante %d, Patota %d, pag %d, offset %d, cantidadPaginas %d", tripu->tid, tripu->pid, tripu->nroPagina, tripu->offset, tripu->cantidadDePaginas);

					list_add(listaTripulantes, tripu);
				//

					memcpy(streamPatota + offset, stream, (SIZEOF_TCB - 8));
					stream += (SIZEOF_TCB - 8); offset += (SIZEOF_TCB - 8);
					memcpy(streamPatota + offset, &proximaInstruccion, sizeof(uint32_t));
					offset += sizeof(uint32_t);
					memcpy(streamPatota + offset, &direccionPCB, sizeof(uint32_t));
					offset += sizeof(uint32_t);
				};
				log_info(loggerMiram, "streamPatota a ser alojado en memoria %s", mem_hexstring(streamPatota, memoriaNecesaria));

				//mem_hexdump(streamPatota, memoriaNecesaria);
				
				llenarFramesConPatota(tablaDePaginas, streamPatota, framesNecesarios, cantidadTCBs, tamanioTareas, memoriaNecesaria, pid);

				mem_hexdump(memoriaPrincipal, 300);

				//dumpDeMemoriaPaginacion();
				
			}

			if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

				t_list * tablaSegmentos = list_create();
				referenciaTablaPatota * referencia = malloc(sizeof(referenciaTablaPatota)); 
				
				PCB * pcb = crearPCB(idPatota);
				void * streamPCB = malloc(SIZEOF_PCB);
				memcpy(streamPCB, &(pcb->pid), sizeof(uint32_t)); 
				memset(streamPCB + sizeof(uint32_t), 0, sizeof(uint32_t)); 
				 
				//printf("ID DE LA PATOTA %d \n", pcb->pid);
				uint32_t direccionPCB = asignarMemoriaSegmentacionPCB(streamPCB, tablaSegmentos); 
				
				uint32_t direccionTareas = asignarMemoriaSegmentacionTareas(tareas, tamanioTareas, tablaSegmentos); 
				pcb ->tareas = direccionTareas; 
				memcpy(memoriaPrincipal + direccionPCB + sizeof(uint32_t), &direccionTareas, 4); 
				

				for(int i = 0 ; i < cantidadTCBs ;  i++ ){
					void * tripulante = malloc(SIZEOF_TCB);
					int offset = 0; 
					int tripulanteID = 0, posx = 0, posy = 0; 
					//Deserializamos los campos que tenemos en el buffer
					memcpy(tripulante + offset, stream, sizeof(uint32_t));
					memcpy(&tripulanteID, stream, sizeof(uint32_t));
					stream += sizeof(uint32_t);
					offset = offset + sizeof(uint32_t);

					memcpy(tripulante + offset, stream, sizeof(uint32_t));
					memcpy(&posx, stream, sizeof(uint32_t));
					stream += sizeof(uint32_t);
					offset = offset + sizeof(uint32_t); 

					memcpy(tripulante + offset, stream, sizeof(uint32_t));
					memcpy(&posy, stream, sizeof(uint32_t));
					stream += sizeof(uint32_t);
					offset = offset + sizeof(uint32_t); 

					memcpy(tripulante + offset, stream, sizeof(char));
					stream += sizeof(char);
					offset = offset + sizeof(char); 

					memcpy(tripulante + offset, &direccionTareas, sizeof(uint32_t)); 
					offset = offset + sizeof(uint32_t);

					memcpy(tripulante + offset, &direccionPCB, sizeof(uint32_t));

					
					

					uint32_t direccionLogica = asignarMemoriaSegmentacionTCB(tripulante, tripulanteID, tablaSegmentos); 
					
					referenciaTripulante * referenciaTripulante = malloc(2 * sizeof(uint32_t)); 

					memcpy(&(referenciaTripulante->tid), &tripulanteID, sizeof(uint32_t));
					memcpy(&(referenciaTripulante->pid), &(pcb->pid), sizeof(uint32_t));
					
					
					sem_wait(&mutexTripulantesPatotas);
					list_add(tripulantesPatotas, referenciaTripulante);
					sem_post(&mutexTripulantesPatotas);

				agregarTripulanteAlMapa(tripulanteID, posx, posy);
			
				}

			referencia ->pid = pcb -> pid; 
			referencia->tablaPatota = tablaSegmentos;

			sem_wait(&mutexTablaDeTablas);
			list_add(tablaDeTablasSegmentos, referencia); 
			sem_post(&mutexTablaDeTablas);
			
			mem_hexdump(memoriaPrincipal, 300);
			
			}
			

			
		
			


		}
		

		break;

	case PEDIR_TAREA: ;

	//enviar proxima tarea (mensaje de miram)

		// void* stream = malloc(paquete->buffer->size);
		// stream = paquete->buffer->stream;
		uint32_t tid = 0; 
		memcpy(&tid, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		// en tid ya tenes el tid del tripulante que te lo pidio

		uint32_t pid = 0; 
		memcpy(&pid, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);

		log_info(loggerMiram, "DATOS TRIPU QUE PIDIO TAREA, PID: %d, TID: %d", pid, tid);
	  
	 
		char * stringTarea = malloc(40); //este es el string de tareas que despues tenes que cambiar por el que uses
		stringTarea = obtenerProximaTarea(pid, tid);
		int tamanioTarea = strlen(stringTarea) + 1;

		log_info(loggerMiram, "La tarea a enviar es: %s", stringTarea);

		if(strcmp(stringTarea, "TRIPULANTE_ELIMINADO") == 0){ break;}
	

		if(strcmp(stringTarea, "NO_HAY_TAREA") == 0){ //Esto cambialo cuando sepas si hay tarea o no

			t_buffer* buffer = malloc(sizeof(t_buffer)); // se puede mandar un buffer vacio????????????
			buffer-> size = sizeof(int);
			void * streamEnvio = malloc(buffer->size);

			int offset = 0;

			memcpy(streamEnvio+offset, &tamanioTarea , sizeof(int));
			offset += sizeof(int);

			// memcpy(streamEnvio+offset, stringTarea, tamanioTarea);

			 buffer-> stream = streamEnvio; 

			
			
			mandarPaqueteSerializado(buffer, socketCliente, NO_HAY_TAREA);
			
			

		}
		else{	
			t_buffer* buffer = malloc(sizeof(t_buffer));
			buffer-> size = sizeof(int) + tamanioTarea;
			void * streamEnvio = malloc(buffer->size);

			int offset = 0;

			memcpy(streamEnvio+offset, &tamanioTarea , sizeof(int));
			offset += sizeof(int);

			memcpy(streamEnvio+offset, stringTarea, tamanioTarea);

			buffer-> stream = streamEnvio;

			mandarPaqueteSerializado(buffer, socketCliente, HAY_TAREA);
			
		}

		break;  

	case ACTUALIZAR_POS: ;

		//log_info(loggerMiram, mem_hexstring(stream, sizeof(uint32_t) * 4));
 
		uint32_t tripulanteid = 0, patotaid = 0, posx = 0, posy = 0;
		int offset = 0;
		memcpy(&tripulanteid, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&patotaid, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&posx, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&posy, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		log_info(loggerMiram,"Tripulante %d se movio hacia %d|%d",tripulanteid,posx,posy);

		moverTripulanteEnMapa(tripulanteid,posx,posy);

		actualizarPosicionTripulante(patotaid, tripulanteid, posx, posy);

	break; 

	case ACTUALIZAR_ESTADO: ;
		 uint32_t trip = 0, pat = 0;
		char estadoNuevo; 
		
		memcpy(&trip, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&pat, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&estadoNuevo, stream+offset, sizeof(char));
		
		actualizarEstadoTripulante(pat, trip, estadoNuevo); 
		mem_hexdump(memoriaPrincipal, 300);
	
	break; 

	case EXPULSAR_TRIPULANTE: ;
		 uint32_t tripid = 0, patid = 0;
		
		
		memcpy(&tripid, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&patid, stream+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		eliminarTripulante(patid, tripid);
		mem_hexdump(memoriaPrincipal, 300);

	break ;

	
	default:	



		break;
	}


}

char * obtenerProximaTarea(uint32_t idPatota, uint32_t tid) {
	
	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		uint32_t direccionTCB = obtenerDireccionTripulante(idPatota, tid);
		if(direccionTCB == tamanioMemoria + 1){
			return  "TRIPULANTE_ELIMINADO";
		} 
		uint32_t direccionTarea = obtenerDireccionProximaTarea(direccionTCB);
		return obtenerProximaTareaSegmentacion(direccionTarea,  direccionTCB);
	
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {

		bool coincideIDPatota(referenciaTablaPaginas * tablaPagina) {
			return tablaPagina->pid == idPatota;
		}
		
		pthread_mutex_lock(&mutexListaTablas);
		referenciaTablaPaginas * tablaDeLaPatota = list_find(listaTablasDePaginas, coincideIDPatota);
		pthread_mutex_unlock(&mutexListaTablas);

		if (tablaDeLaPatota == NULL){
			log_error(loggerMiram, "No se encontro la tabla de paginas de la patota %d", idPatota);
			return "NO_ENCONTRE_TABLA";
		} else {
			return obtenerProximaTareaPaginacion(tablaDeLaPatota, tid);
		}
		
	}

}


uint32_t obtenerDireccionTripulante(uint32_t idPatota, uint32_t tripulanteID){

	
	bool coincidePID(referenciaTablaPatota * unaReferencia){
		return (unaReferencia->pid == idPatota); 
	}
	bool coincideTID(t_segmento * segmento){
		return (segmento->tid == tripulanteID); 
	}
	
	sem_wait(&mutexTablaDeTablas); 
	referenciaTablaPatota * referencia = list_find(tablaDeTablasSegmentos, coincidePID); 
	sem_post(&mutexTablaDeTablas);
	if(referencia == NULL){
		sleep(10); 
		sem_wait(&mutexTablaDeTablas); 
		referencia = list_find(tablaDeTablasSegmentos, coincidePID); 
		sem_post(&mutexTablaDeTablas);
	}
	t_list * tablaPatota = referencia -> tablaPatota;  
	
	log_info(loggerMiram, "despues de la referencia");
	t_segmento * segmentoTripulante = list_find(tablaPatota, coincideTID); 

	if(segmentoTripulante == NULL){
		
		log_info(loggerMiram,"Tripulante %d eliminado", tripulanteID);
		return tamanioMemoria + 1;
	}

	return (segmentoTripulante->base);

}

uint32_t obtenerDireccionProximaTarea(uint32_t direccionTCB){
	
	uint32_t direccionTarea; 
	memcpy(&direccionTarea, memoriaPrincipal + direccionTCB + 3 * sizeof(uint32_t) + sizeof(char), sizeof(uint32_t));

	return direccionTarea;
}

char * obtenerProximaTareaSegmentacion(uint32_t direccionLogicaTarea, uint32_t direccionTCB){
	char * tareaObtenida = malloc(40); 
	char caracterComparacion = 'a'; 
	int desplazamiento = 0; 
	char termino = '\0';
	if(direccionLogicaTarea == (tamanioMemoria + 1)){
		
		return "NO_HAY_TAREA"; 
	}
	
	while(caracterComparacion != '\n' && caracterComparacion != '|'){

	memcpy(tareaObtenida + desplazamiento, memoriaPrincipal+direccionLogicaTarea+desplazamiento, 1 ); 
	
	memcpy(&caracterComparacion, memoriaPrincipal + direccionLogicaTarea + desplazamiento + 1, 1); 
	desplazamiento ++; 
	
	}

	if(caracterComparacion == '\n'){
	int direccionProximaTarea = direccionLogicaTarea + desplazamiento +1;

	actualizarProximaTarea(direccionTCB, direccionProximaTarea);

	}
	if(caracterComparacion == '|'){
		actualizarProximaTarea(direccionTCB, tamanioMemoria + 1); 
		
	}
	memcpy(tareaObtenida + desplazamiento, &termino, 1);
	return tareaObtenida;
	
} 

void actualizarProximaTarea(uint32_t direccionTCB, uint32_t direccionTarea){
		memcpy(memoriaPrincipal + direccionTCB + 3* sizeof(uint32_t) + sizeof(char), &direccionTarea, sizeof(uint32_t));
}

TCB * deserializar_TCB(void * stream){ 
	TCB * tripulante = malloc(SIZEOF_TCB);

	
	//Deserializamos los campos que tenemos en el buffer
	memcpy(&(tripulante->tid), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	//printf("tripulante id deserializaicon %d\n", tripulante->tid);

// 	memcpy(&(tripulante->posicionX), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);

// 	memcpy(&(tripulante->posicionY), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);

	memcpy(&(tripulante->estado), stream, sizeof(char));
	stream += sizeof(char);

	return tripulante;
}

PCB * crearPCB(uint32_t pid){

	PCB * patota = malloc(SIZEOF_PCB);
	patota->pid = pid;
	patota->tareas = SIZEOF_PCB;
	
	
	
	return patota; 
}


void mandarPaqueteSerializado(t_buffer * buffer, int socket, int header){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	 paquete->buffer = malloc(sizeof(buffer->size));

	paquete->header = header;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t) ); //PUSE INT EN VEZ DE UINT_8 PQ NUESTRO HEADER ES UN INT
	int offset2 = 0;

	memcpy(a_enviar + offset2, &(paquete->header), sizeof(int));
	offset2 += sizeof(int);

	memcpy(a_enviar + offset2, &(paquete->buffer->size), sizeof(uint32_t));
	offset2 += sizeof(uint32_t);

	memcpy(a_enviar + offset2, paquete->buffer-> stream, paquete->buffer->size);

	send(socket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);

}


// ------------ASIGNAR MEMORIA-------------

void iniciarMemoria() {
	
	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		tablaSegmentosGlobal = list_create(); 
		tablaDeTablasSegmentos = list_create();
		tripulantesPatotas = list_create();
		
		sem_init(&mutexTablaGlobal, 0, 1);
		sem_init(&mutexTablaDeTablas, 0, 1);
		sem_init(&mutexTripulantesPatotas, 0, 1);
		sem_init(&mutexCompactacion, 0, 1);
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		listaFrames = list_create();
		listaFramesSwap = list_create();
		listaTablasDePaginas = list_create();
		listaTripulantes = list_create();
		pthread_mutex_init(&mutexMemoriaPrincipal, NULL);
		pthread_mutex_init(&mutexListaTablas, NULL);
		pthread_mutex_init(&mutexListaFrames, NULL);
		pthread_mutex_init(&mutexListaFramesSwap, NULL);
		pthread_mutex_init(&mutexTareas, NULL);
		pthread_mutex_init(&mutexContadorLRU, NULL);
		contadorLRU = 0;
		iniciarFrames();
		iniciarSwap();	
	}
}


int buscarEspacioNecesario(int tamanioTareas, int cantidadTripulantes) {

	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

		return buscarEspacioSegmentacion(tamanioTareas, cantidadTripulantes);
		
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		int cantidadMemoriaNecesaria = tamanioTareas + SIZEOF_TCB * cantidadTripulantes + SIZEOF_PCB;
		int cantidadFramesDisponiblesMemoria = framesDisponibles();
		int cantidadFramesNecesarios = divisionRedondeadaParaArriba(cantidadMemoriaNecesaria, tamanioPagina);

		if (cantidadFramesNecesarios <= cantidadFramesDisponiblesMemoria) {
			return 1; // si hay lugar en memoria para todo
		}
		
		int framesNecesariosEnSwap = cantidadFramesNecesarios - cantidadFramesDisponiblesMemoria;
		int framesDisponiblesEnSwap = framesDisponiblesSwap();

		if (framesDisponiblesEnSwap >= framesNecesariosEnSwap) {
			for(int i = 0; i < framesNecesariosEnSwap; i++) {
				llevarPaginaASwap();
			}
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

		pthread_mutex_lock(&mutexListaFrames);
		list_add(listaFrames, frame);
		pthread_mutex_unlock(&mutexListaFrames);
		cantidadFrames ++;
	}
		log_info(loggerMiram, "-------------------------");
		log_info(loggerMiram, "Disponibilidad de frames: ");
		int x = 0;
		void estaOcupado(t_frame * frame) {
			log_info(loggerMiram, "el frame %d esta ocupado: %d ", x, frame->ocupado);
			x++;
		};

		pthread_mutex_lock(&mutexListaFrames);
		list_iterate(listaFrames, estaOcupado);
		pthread_mutex_unlock(&mutexListaFrames);

	return;
}

int framesDisponibles() {
	int libre = 0;
	void estaLibre(t_frame * frame) {
    	if (frame->ocupado == 0){ libre++; }
  	}
	
	pthread_mutex_lock(&mutexListaFrames);
	list_iterate(listaFrames, estaLibre);
	pthread_mutex_unlock(&mutexListaFrames);  
	return libre;
}

int framesDisponiblesSwap() {
	int libre = 0;
	void estaLibre(t_frame * frame) {
    	if (frame->ocupado == 0){ libre++; }
  	}
	
	pthread_mutex_lock(&mutexListaFramesSwap);
	list_iterate(listaFramesSwap, estaLibre);
	pthread_mutex_unlock(&mutexListaFramesSwap);  
	return libre;
}

t_frame * buscarFrame() {

	bool estaLibre(t_frame * frame) {
		return frame->ocupado == 0;
	}

	t_frame * frameLibre = malloc(sizeof(t_frame));

	pthread_mutex_lock(&mutexListaFrames);
    frameLibre = list_find(listaFrames, estaLibre);
	pthread_mutex_unlock(&mutexListaFrames);
	//free(frameLibre);

	return frameLibre;
}

t_frame * buscarFrameSwap() {

	bool estaLibre(t_frame * frame) {
		return frame->ocupado == 0;
	}

	t_frame * frameLibre = malloc(sizeof(t_frame));

			pthread_mutex_lock(&mutexListaFramesSwap);
	frameLibre = list_find(listaFramesSwap, estaLibre);
			pthread_mutex_unlock(&mutexListaFramesSwap);
	//free(frameLibre);

	return frameLibre;
}

void iniciarSwap() {
	FILE * swap = fopen(path_SWAP, "w");
	// char cero = '0';
	// fwrite(&cero, 1, tamanioSwap, swap);
	log_info(loggerMiram, "Iniciando Frames Swap... ");
	int cantidadFrames = 0;

	for(int desplazamiento = 0; desplazamiento< tamanioSwap; desplazamiento += tamanioPagina){
		t_frame * frame = malloc(sizeof(t_frame));
		frame->inicio = desplazamiento;
		frame->ocupado = 0;

		pthread_mutex_lock(&mutexListaFramesSwap);
		list_add(listaFramesSwap, frame);
		pthread_mutex_unlock(&mutexListaFramesSwap);
		cantidadFrames ++;
	}
	fclose(swap);
}

void traerPaginaAMemoria(t_pagina* pagina) {
	llevarPaginaASwap();

	uint32_t nroFrameSwap = pagina->numeroFrame;

	// leer del archivo de SWAP desde el nroFrame * tamPagina
	// buscar frame libre y asignarle esta pagina nueva
	// copiar a memoria el contenido leido en la direccion del frame
	void * memAux = malloc(tamanioPagina);

	FILE * swap = fopen(path_SWAP, "aw+");
	fseek(swap, nroFrameSwap*tamanioPagina, SEEK_SET);
	fread(memAux, tamanioPagina, 1, swap);

	// buscar el frame en la lista de frames swapp y poner que esta libre  --> TODO
	pthread_mutex_lock(&mutexListaFramesSwap);
	t_frame * frameSwap = list_get(listaFramesSwap, nroFrameSwap);
	frameSwap->ocupado = 0;
	pthread_mutex_unlock(&mutexListaFramesSwap);
	//

	t_frame * frameLibre = buscarFrame();

	//pthread_mutex_lock(&mutexMemoriaPrincipal);
	memcpy(memoriaPrincipal + frameLibre->inicio, memAux, tamanioPagina);
	//pthread_mutex_unlock(&mutexMemoriaPrincipal);
	frameLibre->ocupado = 1;
	frameLibre->pagina = pagina;
	frameLibre->pagina->bitDeValidez = 1;
	frameLibre->pagina->numeroFrame = frameLibre->inicio / tamanioPagina;
	frameLibre->pagina->bitDeUso = 1;
	pthread_mutex_lock(&mutexContadorLRU);
	frameLibre->pagina->ultimaReferencia = contadorLRU;
	contadorLRU++;
	pthread_mutex_unlock(&mutexContadorLRU); 
	int index = frameLibre->inicio / tamanioPagina;
	pthread_mutex_lock(&mutexListaFrames);
	list_replace(listaFrames, index, frameLibre);
	pthread_mutex_unlock(&mutexListaFrames);

	log_info(loggerMiram, "Se trae la pagina %d, del proceso %d a MEMORIA", frameLibre->pagina->numeroPagina, frameLibre->pagina->pid);
}

void llevarPaginaASwap() {

	t_frame * frameVictima = seleccionarVictima();
	frameVictima->pagina->bitDeValidez = 0;  // Cambia el valor de la pagina real???
	
	void * memAux = malloc(tamanioPagina);

	//pthread_mutex_lock(&mutexMemoriaPrincipal);
	memcpy(memAux, memoriaPrincipal + frameVictima->inicio, tamanioPagina);
	//pthread_mutex_unlock(&mutexMemoriaPrincipal);

	frameVictima->ocupado = 0;
	int index = frameVictima->inicio / tamanioPagina;
	pthread_mutex_lock(&mutexListaFrames);
	list_replace(listaFrames, index, frameVictima);
	pthread_mutex_unlock(&mutexListaFrames);
	t_frame * frameSwap = buscarFrameSwap();
	frameSwap->ocupado = 1;
	frameSwap->pagina = frameVictima->pagina;
	pthread_mutex_lock(&mutexContadorLRU);
	frameSwap->pagina->ultimaReferencia = contadorLRU;
	contadorLRU++;
	pthread_mutex_unlock(&mutexContadorLRU);

	log_info(loggerMiram, "Se lleva la pagina %d, del proceso %d a SWAP", frameVictima->pagina->numeroPagina, frameVictima->pagina->pid);

	FILE * swap = fopen(path_SWAP, "a+");
	fseek(swap, frameSwap->inicio, SEEK_SET);
	fwrite(memAux, tamanioPagina, 1, swap);
	fclose(swap);
	free(memAux);
}

t_frame * seleccionarVictima() {

	t_frame * victima = malloc(sizeof(t_frame));

	if (strcmp(algoritmoReemplazo, "LRU") == 0) {
		pthread_mutex_lock(&mutexListaFrames);
		t_list * listaAux = list_duplicate(listaFrames);
		pthread_mutex_unlock(&mutexListaFrames);

		bool ultReferencia(t_frame * unFrame, t_frame * otroFrame) {
			if (!otroFrame->ocupado) {
				return true;
			}
			if (!unFrame->ocupado) {
				return false;
			}
			return unFrame->pagina->ultimaReferencia <= otroFrame->pagina->ultimaReferencia;
		}

		list_sort(listaAux, ultReferencia);
		victima = list_get(listaAux, 0);
	}

	if (strcmp(algoritmoReemplazo, "CLOCK") == 0) {
		
	}

	return victima;
}

void dumpDeMemoriaPaginacion() {
	
	char * fecha = temporal_get_string_time("%d-%m-%y_%H:%M:%S"); 
	char * nombreArchivo = string_from_format("dumpMemoria_%s.dmp", fecha); 

	FILE * dump = fopen(nombreArchivo, "w+"); 
	
	pthread_mutex_lock(&mutexListaFrames);
	for(int i = 0; i < list_size(listaFrames); i++){

		t_frame * frameActual = list_get(listaFrames, i); 
		char * estado = malloc(10); 
		char * pagina;
		char * proceso;
		if(frameActual->ocupado){

			estado = "Ocupado\0"; 
			
			pagina = string_from_format("%2d\0", frameActual->pagina->numeroPagina);

			proceso = string_from_format("%2d\0", frameActual->pagina->pid); 
			char * frame = string_from_format("Marco: %2d   Estado: %s Proceso: %s Pagina: %s \n", i, estado, proceso, pagina); 
			
			fwrite(frame, strlen(frame), 1, dump);	

			
		}
		else{
			estado = "Libre\0"; 

			pagina = "-\0";
			proceso = "-\0";
			char * frame = string_from_format("Marco: %2d   Estado: %s Proceso: %s Pagina: %s \n", i, estado, proceso, pagina); 
			fwrite(frame, strlen(frame), 1, dump);	
			
		}
		
		
		 

	}
	pthread_mutex_unlock(&mutexListaFrames);

	fclose(dump);
}

void llenarFramesConPatota(t_list* listaDePaginas, void * streamDePatota, int cantidadFrames, int cantidadTCBs, int longitudTareas, int memoriaAGuardar, uint32_t pid) {

	int i = 0, j = 0;

	for (i = 0; i < cantidadFrames; i++){ 
		t_frame * frameLibre = buscarFrame();
		uint32_t direcProximoFrame = frameLibre->inicio;
		log_info(loggerMiram,"direc prox frame a escribir %d \n", direcProximoFrame);

		pthread_mutex_lock(&mutexMemoriaPrincipal);
		while ((i * tamanioPagina + j) < (memoriaAGuardar) && j < tamanioPagina){
			memcpy( memoriaPrincipal + (direcProximoFrame + j) , streamDePatota + (j + i*tamanioPagina), 1); //copio byte a byte (ojala que ande)	
			j++;
		}
		pthread_mutex_unlock(&mutexMemoriaPrincipal);

		uint32_t numeroDeFrame = direcProximoFrame / tamanioPagina;  // no hace falta redondear porque la division de int redondea pra abajo

		t_pagina * pagina = malloc(sizeof(t_pagina));
		pagina->numeroFrame = numeroDeFrame;
		pagina->numeroPagina = (uint32_t) i;
		pagina->pid = pid;
		pagina->bitDeValidez = 1;
		pagina->bitDeUso = 1;
		pthread_mutex_lock(&mutexContadorLRU);
		pagina->ultimaReferencia = contadorLRU;
		contadorLRU ++;
		pthread_mutex_unlock(&mutexContadorLRU);
		list_add(listaDePaginas, pagina);
		
		t_frame * frameOcupado = malloc(sizeof(t_frame));
		frameOcupado->inicio = direcProximoFrame;
		frameOcupado->ocupado = 1;
		frameOcupado->pagina = pagina; 
		j=0; // NO BOIRRAR
		pthread_mutex_lock(&mutexListaFrames);
		t_frame * frameParaLiberar = list_replace(listaFrames, numeroDeFrame, frameOcupado);  // ver si se puede usar replace and destroy para liberar memoria
		pthread_mutex_unlock(&mutexListaFrames);
		//free(frameParaLiberar);
	}

	referenciaTablaPaginas * tablaDePaginas = malloc(sizeof(referenciaTablaPaginas));
	tablaDePaginas->longitudTareas = longitudTareas;
	tablaDePaginas->pid = pid;
	tablaDePaginas->listaPaginas = listaDePaginas;

	pthread_mutex_lock(&mutexListaTablas);
	list_add(listaTablasDePaginas, tablaDePaginas);
	pthread_mutex_unlock(&mutexListaTablas);
}

int divisionRedondeadaParaArriba(int x, int y) {
	return (x -1)/y +1;
}


void * obtenerStreamTripulante(referenciaTablaPaginas * referenciaTabla, uint32_t tid){
	
	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}

	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID); 
	
	uint32_t offset = referenciaTripulante->offset; 
	uint32_t cantPaginas = referenciaTripulante-> cantidadDePaginas; 
	uint32_t pagina = referenciaTripulante ->nroPagina; 
	void * streamTripulante = malloc(SIZEOF_TCB);
	int bytesCopiados = 0;
	
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	for(int paginaUsada = 0 ; paginaUsada < cantPaginas; paginaUsada++){
		uint32_t direccionFrame = obtenerDireccionFrame(referenciaTabla, pagina); 

		for(int desplazamiento = 0; desplazamiento + offset < tamanioPagina && bytesCopiados < SIZEOF_TCB; desplazamiento ++){
			memcpy(streamTripulante + bytesCopiados, memoriaPrincipal + direccionFrame + desplazamiento + offset, 1); 
			bytesCopiados++;
		}

		offset = 0; 
		pagina++; 
	}
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
	mem_hexdump(streamTripulante, SIZEOF_TCB);
	return streamTripulante; 
}

uint32_t obtenerDireccionFrame(referenciaTablaPaginas * referenciaTabla, uint32_t nroPagina){

	uint32_t direcFrame = 0; 

	bool coincideNro(t_pagina * pagina){
		return (pagina->numeroPagina == nroPagina);
	}

	t_list * tablaPaginas = referenciaTabla->listaPaginas; 
	t_pagina * pagina = list_find(tablaPaginas, coincideNro); 
	if (!pagina->bitDeValidez) {
		traerPaginaAMemoria(pagina);
	}
	direcFrame = pagina->numeroFrame * tamanioPagina; 

	return direcFrame; 
}

void actualizarEstadoPaginacion(uint32_t tid, uint32_t pid, char estadoNuevo){

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}

	bool coincidePID(referenciaTablaPaginas * referenciaTabla){
		return (referenciaTabla->pid == pid); 
	}

	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID); 
	pthread_mutex_lock(&mutexListaTablas);
	referenciaTablaPaginas * referenciaTabla = list_find(listaTablasDePaginas, coincidePID); 
	pthread_mutex_unlock(&mutexListaTablas);
	t_list * tablaPaginas = referenciaTabla->listaPaginas; 
	uint32_t primeraPagina = referenciaTripulante->nroPagina; 
	uint32_t offset = referenciaTripulante->offset; 
	uint32_t bytesPrevios = sizeof(uint32_t)*3; 
	uint32_t cantidadPaginas = referenciaTripulante->cantidadDePaginas; 

	if((tamanioPagina - offset) > (bytesPrevios + 1)){


		uint32_t frame = obtenerDireccionFrame(referenciaTabla, primeraPagina); 
		//habria que hacer el memcopy en direccion frame + offset + bytesPrevios 
		pthread_mutex_lock(&mutexMemoriaPrincipal);
		memcpy(memoriaPrincipal + frame + offset + bytesPrevios, &estadoNuevo, sizeof(char));
		pthread_mutex_unlock(&mutexMemoriaPrincipal);
	}

	else{

		for(int nroPagina = 0; nroPagina < cantidadPaginas; nroPagina ++){
		//bytes que necesitaria recorrer para llegar al estado, es decir los que necesitaria que entren en la pagina para llegar
		uint32_t bytesProximaPagina = SIZEOF_TCB -(tamanioPagina - offset)- 2*sizeof(uint32_t)- nroPagina * tamanioPagina; 
		if(bytesProximaPagina < tamanioPagina){
			uint32_t pagina = nroPagina + primeraPagina + 1; 
			uint32_t frame = obtenerDireccionFrame(referenciaTabla, pagina); 
			uint32_t desplazamiento = bytesProximaPagina - 1;
			pthread_mutex_lock(&mutexMemoriaPrincipal);
			memcpy(memoriaPrincipal + frame + desplazamiento, &estadoNuevo, sizeof(char));
			pthread_mutex_unlock(&mutexMemoriaPrincipal);
		}
		}
	}
	
}

void actualizarPosicionTripulante(uint32_t pid, uint32_t tid, uint32_t posx, uint32_t posy){

	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0 ){
		actualizarPosicionTripulanteSegmentacion(pid, tid, posx, posy); 
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		actualizarPosicionPaginacion(pid,  tid, posx, posy);
	}
}

void actualizarPosicionPaginacion(uint32_t pid, uint32_t tid, uint32_t posx, uint32_t posy) {

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}

	bool coincidePID(referenciaTablaPaginas * referenciaTabla){
		return (referenciaTabla->pid == pid); 
	}

	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID); 
	pthread_mutex_lock(&mutexListaTablas);
	referenciaTablaPaginas * referenciaTabla = list_find(listaTablasDePaginas, coincidePID); 
	pthread_mutex_unlock(&mutexListaTablas);
	t_list * tablaPaginas = referenciaTabla->listaPaginas; 
	uint32_t primeraPagina = referenciaTripulante->nroPagina; 
	uint32_t offset = referenciaTripulante->offset; 
	uint32_t bytesPrevios = sizeof(uint32_t);
	uint32_t cantidadPaginas = referenciaTripulante->cantidadDePaginas;

	if((tamanioPagina - offset) > (bytesPrevios + sizeof(uint32_t)*2)){
		uint32_t frame = obtenerDireccionFrame(referenciaTabla, primeraPagina); 
		//habria que hacer el memcopy en direccion frame + offset + bytesPrevios 
		pthread_mutex_lock(&mutexMemoriaPrincipal);
		memcpy(memoriaPrincipal + frame + offset + bytesPrevios, &posx, sizeof(uint32_t));
		memcpy(memoriaPrincipal + frame + offset + bytesPrevios + 4, &posy, sizeof(uint32_t));
		pthread_mutex_unlock(&mutexMemoriaPrincipal);
	} else {
		int i = 0,j = 0;
		void * memAux = malloc(8);
		memcpy(memAux, &posx, 4);
		memcpy(memAux + 4, &posy, 4);
		while (j < 8) {
			uint32_t frame = obtenerDireccionFrame(referenciaTabla, primeraPagina);
			while (offset + 4 + j < tamanioPagina && j < 8){
				memcpy(memoriaPrincipal + frame + offset + 4 + j, memAux + j, 1);
				j++;
			}
			offset = 0;
			i++;
		}
	}

}

void eliminarTripulantePaginacion(uint32_t tid, uint32_t pid){

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}
		bool coincidePID(referenciaTablaPaginas * referenciaTabla){
		return (referenciaTabla->pid == pid); 
	}
	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID); 
	referenciaTablaPaginas * referenciaTabla = list_find(listaTablasDePaginas, coincidePID); 
	t_list * tablaPaginas = referenciaTabla->listaPaginas; 
	uint32_t primeraPagina = referenciaTripulante->nroPagina; 
	uint32_t offset = referenciaTripulante->offset; 
	uint32_t cantidadPaginas = referenciaTripulante->cantidadDePaginas; 
	
	uint32_t paginasLlenas = cantidadPaginas - 2;

	//para la primera pagina que ocupa 
	

	for(int i = 0; i < paginasLlenas; i ++){

		bool coincideNro(t_pagina * pagina){
			return (pagina->numeroPagina == (primeraPagina + i + 1)); 
		}

		t_pagina * paginaActual = list_find(tablaPaginas, coincideNro); 
		paginaActual->bitDeValidez = 0; 
		uint32_t frame = paginaActual->numeroFrame;
		t_frame * unFrame =  list_get(listaFrames, frame); 
		unFrame->ocupado = 0; 
		
	}

}


uint32_t obtenerDireccionProximaTareaPaginacion(void * streamTripulante){
	uint32_t direccion = 0; 
	memcpy(&direccion, streamTripulante + sizeof(uint32_t)*3 + sizeof(char), sizeof(uint32_t)); 
	return direccion;
}

char * obtenerProximaTareaPaginacion(referenciaTablaPaginas * referenciaTabla, uint32_t tid) {
	void * streamTripulante = malloc(SIZEOF_TCB);
	streamTripulante = obtenerStreamTripulante(referenciaTabla, tid);
	int proximaTarea = 0;
	proximaTarea = obtenerDireccionProximaTareaPaginacion(streamTripulante);
	if ((proximaTarea) == 0) {
		return "NO_HAY_TAREA";
	}

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}

	t_tripulantePaginacion * referenciaTripu = list_find(listaTripulantes, coincideID);

	/// Encontrar los frames de las tareas
	int i = 0, j = 0;

	bool coincideIDPatota(referenciaTablaPaginas * tablaPagina) {
		return tablaPagina->pid == referenciaTripu->pid;
	}
		
	referenciaTablaPaginas * tablaDeLaPatota = list_find(listaTablasDePaginas, coincideIDPatota);
	int cantidadDePaginas = divisionRedondeadaParaArriba((tablaDeLaPatota->longitudTareas + SIZEOF_PCB) , tamanioPagina);
	int *frames = malloc(cantidadDePaginas * 4);

	for(int z = 0; z < cantidadDePaginas; z++) {
		t_pagina * pagina = list_get(tablaDeLaPatota->listaPaginas, z);
		if (!pagina->bitDeValidez) {
			traerPaginaAMemoria(pagina);
		}
		frames[i] = pagina->numeroFrame;
		i++;
	}

	void * streamTareas = malloc(cantidadDePaginas * tamanioPagina);

	pthread_mutex_lock(&mutexMemoriaPrincipal);	
	for(j = 0; j < i; j++) {
		memcpy(streamTareas + j * tamanioPagina, memoriaPrincipal + tamanioPagina * frames[j], tamanioPagina);
	}
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
	///
	char * tarea = malloc(40); //como se cuanto ocupan las tareas
	char c = 'a';
	j = 0;i = 0;

	memcpy(&c, streamTareas + proximaTarea, 1);
	while(c != '|' && c != '\n') {
		memcpy(tarea + j, streamTareas + i + proximaTarea, 1);
		i++;
		memcpy(&c, streamTareas + i + proximaTarea, 1);
		j++;
	}

	if(c == '\n') {
		proximaTarea = i + proximaTarea + 1;
	}

	if(c == '|') {
		proximaTarea = 0;
	}

	printf("prox tarea: %d", proximaTarea);
	//realloc(tarea, j + 1);
	char barraCero = '\0';
	memcpy(tarea + j, &barraCero, 1);

	actualizarPunteroTarea(referenciaTripu, referenciaTabla->listaPaginas, proximaTarea);

	return tarea;
}

void actualizarPunteroTarea(t_tripulantePaginacion * unTripu, t_list * tablaDePaginas, int direcProximaTarea) {

	int * frames = malloc(unTripu->cantidadDePaginas * sizeof(*frames)); 
	int i = 0, z = 0, nroPrimerPagina = 0, j = 0;

	void almacenarFrames(t_pagina * pagina) { 
		if((unTripu->nroPagina == pagina->numeroPagina)) {
			if (!pagina->bitDeValidez) {
				traerPaginaAMemoria(pagina);
			}
			frames[i] = pagina->numeroFrame;
			i++;
			nroPrimerPagina = pagina->numeroPagina;
		}
	}

	list_iterate(tablaDePaginas, almacenarFrames);

	while (unTripu->cantidadDePaginas - 1 - z) {
		t_pagina * pag = malloc(sizeof(t_pagina));
		pag = list_get(tablaDePaginas, nroPrimerPagina + 1 + z);
		frames[i] = pag->numeroFrame;
		i++;
		z++;
	}

	i = 0;

	pthread_mutex_lock(&mutexMemoriaPrincipal);
	if (unTripu->offset + 13 + 4 < tamanioPagina) {
		memcpy(memoriaPrincipal + tamanioPagina * frames[i] + unTripu->offset + 13, &direcProximaTarea, 4);
	} else {
		void * memAux = malloc(4);
		memcpy(memAux, &direcProximaTarea, 4);
		while (j < 4) {
			while (unTripu->offset + 13 + j < tamanioPagina){
				memcpy(memoriaPrincipal + tamanioPagina * frames[i] + unTripu->offset + 13 + j, memAux + j, 1);
				j++;
			}
			i++;
		}
	}
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
}

//----------------SEGMENTACION

uint32_t asignarMemoriaSegmentacionTCB(void * tripulante, int tripulanteID, t_list * tablaSegmentos){

		//busco un lugar de memoria (segun algoritmo)
		uint32_t direccionLogica = 0;
		direccionLogica = encontrarLugarSegmentacion(SIZEOF_TCB); 
		//le asigno el lugar de memoria encontrado
		memcpy(memoriaPrincipal + direccionLogica, tripulante, SIZEOF_TCB); 
		   

		//creo el segmento para la estructura nueva
		t_segmento * segmentoNuevo = malloc(sizeof(t_segmento)); 
		segmentoNuevo ->tid = tripulanteID;
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
		segmentoNuevo->tid = -1; 
		segmentoNuevo -> tamanio = SIZEOF_PCB; 
		segmentoNuevo -> base = direccionLogica; 

		//agrego el segmento a la tabla de segmentos 
		list_add_in_index(tablaSegmentos, 0,segmentoNuevo);
		sem_wait(&mutexTablaGlobal);
		list_add(tablaSegmentosGlobal, segmentoNuevo); 
		sem_post(&mutexTablaGlobal);
		//printf("Direccion logica asignada %d \n", direccionLogica);   
		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionTareas(char * tareas, int tamanioTareas, t_list * tablaSegmentos){

	uint32_t direccionLogica = encontrarLugarSegmentacion(tamanioTareas); 
	memcpy(memoriaPrincipal + direccionLogica, tareas, tamanioTareas); 
	

	t_segmento * segmentoTareas = malloc(sizeof(t_segmento)); 
	segmentoTareas -> tid = -1; 
	segmentoTareas->tamanio = tamanioTareas; 
	segmentoTareas ->base = direccionLogica; 
	list_add(tablaSegmentos, segmentoTareas);
	sem_wait(&mutexTablaGlobal);
	list_add(tablaSegmentosGlobal, segmentoTareas);  
	sem_post(&mutexTablaGlobal);
	return direccionLogica;

}

uint32_t encontrarLugarSegmentacion(int tamanioSegmento){
	if(strcmp(criterioSeleccion, "FIRST_FIT") == 0){return firstFit(tamanioSegmento); }
	if(strcmp(criterioSeleccion, "BEST_FIT") == 0){ return bestFit(tamanioSegmento);}
	}

uint32_t firstFit(int tamanioContenido){
	int i = 0; 
	t_list * segmentosLibres = obtenerSegmentosLibres(tablaSegmentosGlobal); 
	t_segmento * segmentoLibre = list_get(segmentosLibres, i); 
	
	bool cabeElContenido(t_segmento * segmento){

			if((segmento->tamanio) >= tamanioContenido){
				return true; 
			}
			else{
				return false; 
			}
		}

	if(!(list_any_satisfy(segmentosLibres, cabeElContenido))){
		log_info(loggerMiram, "Compactando memoria");
		sem_wait(&mutexCompactacion); 
		compactarMemoria();
		sem_post(&mutexCompactacion);
	}
	

	while((segmentoLibre -> tamanio) < tamanioContenido){
		i++; 
		segmentoLibre = list_get(segmentosLibres, i); 
	}
	//printf("La base del primer segmento libre es : %d \n", (segmentoLibre -> base));
	//printf("El tamanio del primer segmento libre es %d \n", segmentoLibre->tamanio);

	
	return (segmentoLibre -> base);

}

uint32_t bestFit(int tamanioContenido){

	int i = 0;  
	t_list * segmentosLibres = obtenerSegmentosLibres(tablaSegmentosGlobal); 
	t_segmento * segmentoLibre = malloc(sizeof(t_segmento)); 
	t_list * lugaresPosibles = list_create(); 

	bool cabeElContenido(t_segmento * segmento){

			if((segmento->tamanio) >= tamanioContenido){
				return true; 
			}
			else{
				return false; 
			}
		}

	if(!(list_any_satisfy(segmentosLibres, cabeElContenido))){
		log_info(loggerMiram, "Compactando memoria"); 
		sem_wait(&mutexCompactacion); 
		compactarMemoria();
		sem_post(&mutexCompactacion);
	}

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

void actualizarPosicionTripulanteSegmentacion(uint32_t idPatota, uint32_t idTripulante, uint32_t nuevaPosx, uint32_t nuevaPosy){
	uint32_t direccionTripulante = obtenerDireccionTripulante(idPatota, idTripulante); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t), &nuevaPosx, sizeof(uint32_t)); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t)*2, &nuevaPosy, sizeof(uint32_t));
}

void eliminarTripulante(uint32_t patid, uint32_t tripid) {
	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		eliminarTripulanteSegmentacion(patid, tripid);
	}	
	if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
		eliminarTripulantePaginacion(patid, tripid);
	}
}

void eliminarTripulanteSegmentacion(uint32_t pid, uint32_t tid){
	uint32_t direccionTripulante = obtenerDireccionTripulante(pid, tid); 
	memset(memoriaPrincipal + direccionTripulante, 0, SIZEOF_TCB); 
	bool coincidePID(referenciaTablaPatota * unaReferencia){
		return (unaReferencia->pid == pid); 
	}

	bool coincideTID(t_segmento * segmento){
		return (segmento->tid == tid); 
	}

	sem_wait(&mutexTablaDeTablas);
	referenciaTablaPatota * referencia = list_find(tablaDeTablasSegmentos, coincidePID); 
	t_list * tablaPatota = referencia->tablaPatota; 
	sem_post(&mutexTablaDeTablas);

	list_remove_by_condition(tablaPatota, coincideTID); 
	list_remove_by_condition(tablaSegmentosGlobal, coincideTID); 
	
	esElUltimoTripulante(tablaPatota, pid); 

}

void esElUltimoTripulante(t_list * tabla, uint32_t pid){


	t_segmento * segmentoPID = list_get(tabla, 0);
	t_segmento * segmentoTareas = list_get(tabla,1); 
	uint32_t tamanioTareas = segmentoTareas->tamanio; 

	bool coincideBasePID(t_segmento * segmento){
		return (segmento->base == segmentoPID->base); 
	}
	bool coincideBaseTareas(t_segmento * segmento){
		return (segmento->base == segmentoTareas->base); 
	}

	bool coincidePID(referenciaTablaPatota * referencia){
		return (referencia->pid == pid); 
	}

	//significa que quedan los segmentos de tareas y del pcb 
	if(list_size(tabla) == 2){

		list_remove_by_condition(tablaSegmentosGlobal, coincideBasePID);
		memset(memoriaPrincipal + (segmentoPID ->base), 0, SIZEOF_PCB);  
		list_remove_by_condition(tablaSegmentosGlobal, coincideBaseTareas); 
		memset(memoriaPrincipal + (segmentoTareas->base), 0, tamanioTareas); 
		
		sem_wait(&mutexTablaDeTablas); 
		list_remove_by_condition(tablaDeTablasSegmentos, coincidePID); 
		sem_post(&mutexTablaDeTablas);
		list_destroy(tabla);

		

	}

	else{
		//nada
	}
}

void actualizarEstadoTripulante(uint32_t pid, uint32_t tid, char estadoNuevo) {
	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		actualizarEstadoSegmentacion(pid, tid, estadoNuevo);
	}	
	if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
		actualizarEstadoPaginacion(pid, tid, estadoNuevo);
	}
}

void actualizarEstadoSegmentacion(uint32_t pid, uint32_t tid, char estadoNuevo){
	uint32_t direccionTripulante = obtenerDireccionTripulante(pid, tid); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t)*3, &estadoNuevo, sizeof(char)); 
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
	int espacioLibre = 0; 
	for(int i = 0; i < (list_size(segmentosLibres)); i++){
	   t_segmento * segmento = list_get(segmentosLibres, i); 
	   espacioLibre = espacioLibre + segmento->tamanio;
	   printf("Segmento libre que empieza en %d y termina en %d \n", segmento->base, segmento->base + segmento->tamanio); 
	}
	printf("BYtes libres: %d \n", espacioLibre);

}

int buscarEspacioSegmentacion(int tamanioTareas, int cantidadTripulantes){
	t_list * segmentosLibres = obtenerSegmentosLibres(tablaSegmentosGlobal); 
	uint32_t bytesLibres = 0; 
	uint32_t espacioNecesario = tamanioTareas + cantidadTripulantes * SIZEOF_TCB + SIZEOF_PCB; 

	for(int i = 0 ; i< list_size(segmentosLibres); i++){
		t_segmento * segmentoLibre = list_get(segmentosLibres, i); 
		bytesLibres = bytesLibres + (segmentoLibre->tamanio); 
	}

	if (espacioNecesario <= bytesLibres){
		return 1;
	}
	else{
		return -1; 
	}

	
}
 /*
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
	 */

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



void compactarMemoria(){

	list_sort(tablaSegmentosGlobal, seEncuentraPrimeroEnMemoria);

	//para el primer segmento 
	t_segmento * primerSegmentoOcupado = list_get(tablaSegmentosGlobal, 0); 
	if(primerSegmentoOcupado->base != 0) {
		memcpy(memoriaPrincipal, memoriaPrincipal + primerSegmentoOcupado->base, primerSegmentoOcupado->tamanio); 
		primerSegmentoOcupado -> base = 0; 
		list_replace(tablaSegmentosGlobal, 0, primerSegmentoOcupado);

	}
	for(int i = 0; i < (list_size(tablaSegmentosGlobal)-1); i++){
		t_segmento * segmentoActual = list_get(tablaSegmentosGlobal, i); 
		int finSegmento = segmentoActual->base + segmentoActual->tamanio;
		t_segmento * proximoSegmento = list_get(tablaSegmentosGlobal, i + 1); 
		memcpy(memoriaPrincipal + finSegmento, memoriaPrincipal + proximoSegmento->base, proximoSegmento->tamanio); 
		proximoSegmento->base = finSegmento; 
		list_replace(tablaSegmentosGlobal, i+1, proximoSegmento);
		
	}

	t_list * segmentoLibre = obtenerSegmentosLibres(tablaSegmentosGlobal); 
	t_segmento * espacioLibre = list_get(segmentoLibre, 0); 
	memset(memoriaPrincipal + (espacioLibre->base), 0,espacioLibre->tamanio);

}
/* 
void actualizarPosicionTripulanteSegmentacion(uint32_t idPatota, uint32_t idTripulante, uint32_t nuevaPosx, uint32_t nuevoPosy){

}*/


//DUMP DE MEMORIA
void imprimirSegmentos(){

	 char * fecha = temporal_get_string_time("%d-%m-%y_%H:%M:%S"); 
	 char * nombreArchivo = string_from_format("dumpMemoria_%s.dmp", fecha);
	
	FILE* dump = fopen(nombreArchivo, "w+"); 
	
	fwrite("DUMP DE MEMORIA \n", 17, 1, dump); 
	
	for(int i = 0; i< list_size(tablaDeTablasSegmentos); i++){

		referenciaTablaPatota * referencia = list_get(tablaDeTablasSegmentos, i); 
		t_list * tabla = referencia->tablaPatota; 
		int proceso = referencia->pid;
		char pid = proceso + '0'; 
		for(int x = 0; x < list_size(tabla); x++){
			t_segmento * unSegmento = list_get(tabla, x);
			char * segmento = string_from_format("Proceso: %2d    Segmento: %2d Inicio: %3d Tamaño: %3d \n", proceso, x, unSegmento->base, unSegmento->tamanio); 
			fwrite(segmento, strlen(segmento), 1, dump); 
		}
	}
	fclose(dump);		
			

		
	}
	
		
	


  

void sig_handler(uint32_t senial){

	if(senial == SIGUSR1){
		imprimirSegmentos(); 
		 
		 
	}
	else{
		if(senial == SIGUSR2){
		sem_wait(&mutexCompactacion); 
		compactarMemoria(); 
		sem_post(&mutexCompactacion);
		}
		else{
			log_error(loggerMiram, "No se reconoce la señal enviada");
		}
	}
	
	
}



void * hiloSIGUSR1(){
	signal(SIGUSR1, sig_handler); 

	return NULL; 
}

void * hiloSIGUSR2(){
	signal(SIGUSR2, sig_handler); 

	return NULL ;
}

// ------------------------------------------------------ MAPA ----------------------------------------------

void iniciarMapa() {
	sem_init(&semaforoMoverTripulante,0,1);
	sem_init(&semaforoTerminarMapa,0,0);
	nivel_gui_inicializar();
	sem_wait(&semaforoTerminarMapa);
}

void agregarTripulanteAlMapa(uint32_t tid, uint32_t x, uint32_t y) { 
	char id = idMapa(tid); //aca el tid es un uint  
    personaje_crear(navePrincipal, id, x, y);
	nivel_gui_dibujar(navePrincipal);
}

void moverTripulanteEnMapa(uint32_t tid, uint32_t x, uint32_t y){
	char id = idMapa(tid);
	sem_wait(&semaforoMoverTripulante);
	item_mover(navePrincipal, id, x, y);
	nivel_gui_dibujar(navePrincipal);
	sem_post(&semaforoMoverTripulante);
}

void expulsarTripulanteDelMapa(uint32_t tid) {
	char id = idMapa(tid);
	item_borrar(navePrincipal, id);
	nivel_gui_dibujar(navePrincipal);
}

char idMapa(uint32_t tid){

	char id = 0;

	id = tid + 65;

	if (id > 90) {
		id += 6;
	}
	return id;

}
