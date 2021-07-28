#include "mi_ram.h"


int main(int argc, char ** argv){

	//navePrincipal = nivel_crear("Nave Principal");
	loggerMiram = log_create("miram.log", "mi_ram.c", 0, LOG_LEVEL_INFO);
	loggerSegmentacion = log_create("segmentacion.log", "mi_ram.c", 0, LOG_LEVEL_INFO);
	leerConfig();

	log_info(loggerMiram, "---------------------------INICIANDO MI-RAM-HQ-------------------------");
	memoriaPrincipal = malloc(tamanioMemoria);
	memset(memoriaPrincipal, 0, tamanioMemoria);
	iniciarMemoria();
	

	pthread_t servidor;
	pthread_t senial1;
	pthread_t senial2;
	pthread_create(&servidor, NULL, servidorPrincipal, puertoMemoria);
	pthread_create(&senial1, NULL, hiloSIGUSR1, NULL); 
	pthread_create(&senial2, NULL, hiloSIGUSR2, NULL);

	//  pthread_t mapa;
	//  pthread_create(&mapa, NULL, iniciarMapa, NULL);
	//  pthread_join(mapa, NULL);
	//  nivel_destruir(navePrincipal); 
	//  nivel_gui_terminar();

	 	
	pthread_join(servidor, NULL);
	pthread_join(senial1, NULL);
	pthread_join(senial2, NULL);
	
	
	free(memoriaPrincipal); 

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
	pathSwap = config_get_string_value(config, "PATH_SWAP");
	tamanioSwap = config_get_int_value(config, "TAMANIO_SWAP");

}
void servidorPrincipal() {
	int listeningSocket = crear_conexionServer(puertoMemoria);

	

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	 


	while(1){
		pthread_t * receptorDiscordiador = malloc(sizeof(pthread_t));
		int * socketCliente = malloc(sizeof(int)); 
		*socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
		if(socketCliente == -1){printf("Error en la conexión"); log_info(loggerMiram, "error en la conexion con Discordiador");}
		else {
			//log_info(loggerMiram, "Conexión establecida con Discordiador");
			// void * socket = malloc(sizeof(int)); 
			// memcpy(socket, &socketCliente, 4); 
			// log_info(loggerMiram2, "Direccion de memoria %x", &socket); 
			// log_info(loggerMiram2, "Socket cliente %x", &socketCliente);
			int creacionHilo = pthread_create(receptorDiscordiador, NULL, atenderDiscordiador, socketCliente);
			if(creacionHilo != 0){
				log_info(loggerMiram, "Error en la creacion del hilo. Resultado de la creacion: %d", creacionHilo);
			}
			//log_info(loggerMiram2, "Resultado creacion hilo %d", creacionHilo);
			pthread_detach((*receptorDiscordiador));
			//log_info(loggerMiram2, "Contador de hilos: %d", contadorHilos);
			 
			
		}

		//free(receptorDiscordiador); 
		     
		
	}
	
	
	close(listeningSocket);
}

void * atenderDiscordiador(void * socket){
	
	int socketCliente = 0; 
	memcpy(&socketCliente, socket, 4);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	
	int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);
	
	if(headerRECV) { /*log_info(loggerMiram, "Recibi header: %d\n", paquete->header) */} else{ log_error(loggerMiram, "No se pudo recibir el header");}
	
	 
	int statusTamanioBuffer = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);
	

	paquete->buffer->stream = malloc(paquete->buffer->size);

	
	int BUFFER_RECV = recv(socketCliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream
	
	// ACA
	void * stream = paquete->buffer->stream; // ESTE STREAM ESTA BIEN


	uint32_t idPatota = 0;

	if(compactacion){
		sem_wait(&semaforoCompactacion); 
	}       

	switch (paquete->header)
	{
	case INICIAR_PATOTA: ; 

	log_info(loggerMiram, "Header 0 recibido: INICIAR PATOTA"); 
		
		// Deserializamos tareas
		int tamanioTareas;
		memcpy(&tamanioTareas, stream, sizeof(int));
		stream += sizeof(int);
		char* tareas = malloc(tamanioTareas + 1); //mirar si no es +1
		memcpy(tareas, stream, tamanioTareas);
		stream += tamanioTareas;

		char pipe = '|';
		memcpy(tareas + tamanioTareas, &pipe, 1);	
		tamanioTareas++;
		

		//deserializar pid 
		idPatota = 0; 
		memcpy(&idPatota, stream, sizeof(uint32_t)); 
		stream += sizeof(uint32_t);

		//Deserializar CantidadDeTCBs
		int cantidadTCBs = 0;
		memcpy(&cantidadTCBs, stream, sizeof(int));
		stream += sizeof(int); //lo que sigue en el stream son los tcbs
		log_info(loggerMiram, "PID: %d", idPatota); 
		log_info(loggerMiram, "Tareas recibidas: %s Tamanio de las tareas: %d", tareas, tamanioTareas); 
		log_info(loggerMiram, "Cantidad de tripulantes: %d \n", cantidadTCBs);

		//Nos aseguramos de que hay espacio para recibir la patota 
		
		int hayLugar = buscarEspacioNecesario(tamanioTareas, cantidadTCBs);

		if(hayLugar == -1){
			log_info(loggerMiram, "Rechazo patota por falta de espacio en memoria.");
					
		}
			
				
		if(hayLugar == 1){
			log_info(loggerMiram, "Espacio necesario existente. Agrego patota a memoria...");

			if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
				
				t_list * tablaSegmentos = list_create();
				referenciaTablaPatota * referencia = malloc(sizeof(referenciaTablaPatota)); 
				referencia->tablaPatota = tablaSegmentos;
			
				PCB * pcb = crearPCB(idPatota);
				referencia->pid = pcb->pid;
				void * streamPCB = malloc(SIZEOF_PCB);
				memcpy(streamPCB, &(pcb->pid), sizeof(uint32_t)); 
				memset(streamPCB + sizeof(uint32_t), 0, sizeof(uint32_t)); 
				
				 
				//printf("ID DE LA PATOTA %d \n", pcb->pid);
				uint32_t direccionPCB = asignarMemoriaSegmentacionPCB(streamPCB, tablaSegmentos);
				log_info(loggerMiram, "PCB de la patota %d agregado a memoria", pcb->pid);  
				list_add(listaReferenciasPatotaSegmentacion, referencia);

				
				
				uint32_t direccionTareas = asignarMemoriaSegmentacionTareas(tareas, tamanioTareas, tablaSegmentos, (pcb->pid)); 
				log_info(loggerMiram, "Tareas de la patota %d agregadas a memoria", pcb->pid); 
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

					
					

					uint32_t direccionLogica = asignarMemoriaSegmentacionTCB(tripulante, tripulanteID, tablaSegmentos, (pcb->pid)); 
					log_info(loggerMiram, "Tripulante agregado a memoria. TID: %d", tripulanteID);
					
					

				//agregarTripulanteAlMapa(tripulanteID, posx, posy);
				
			
				}


			int headerAEnviar = PATOTA_CREADA; 
			send(socketCliente, &headerAEnviar, sizeof(int), 0); 

			
			
			}
			
			
		
			


		}
		

		break;

	case PEDIR_TAREA: ;
		log_info(loggerMiram, "Header 1 recibido: PEDIR TAREA"); 
	

		// void* stream = malloc(paquete->buffer->size);
		// stream = paquete->buffer->stream;
		uint32_t tid = 0; 
		memcpy(&tid, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		// en tid ya tenes el tid del tripulante que te lo pidio

		uint32_t pid = 0; 
		memcpy(&pid, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);

		log_info(loggerMiram, "Tripulante %d (perteneciente a la patota %d) pidio una tarea.", tid, pid); 

	 	
		char * stringTarea = malloc(40); //este es el string de tareas que despues tenes que cambiar por el que uses
		uint32_t direccionTCB = obtenerDireccionTripulanteSegmentacion(pid, tid); 
		 
		uint32_t direccionTarea; 
		
		memcpy(&direccionTarea, memoriaPrincipal + direccionTCB + sizeof(uint32_t)*3 + sizeof(char), sizeof(uint32_t));
		
		stringTarea = obtenerProximaTareaSegmentacion(direccionTarea,direccionTCB); 
		
		int tamanioTarea = strlen(stringTarea) + 1;

		

		if(strcmp(stringTarea, "TRIPULANTE_ELIMINADO") == 0){ break;}
	

		if(strcmp(stringTarea, "NO_HAY_TAREA") == 0){ //Esto cambialo cuando sepas si hay tarea o no

			log_info(loggerMiram, "La tarea a enviar al tripulante %d es: %s", tid, stringTarea);
			
			t_buffer* buffer = malloc(sizeof(t_buffer)); // se puede mandar un buffer vacio????????????
			buffer-> size = sizeof(int);
			void * streamEnvio = malloc(buffer->size);

			int offset = 0;

			memcpy(streamEnvio+offset, &tamanioTarea , sizeof(int));
			offset += sizeof(int);

			// memcpy(streamEnvio+offset, stringTarea, tamanioTarea);

			 buffer-> stream = streamEnvio; 

			 
			
			mandarPaqueteSerializado(buffer, socketCliente, NO_HAY_TAREA);

			//mem_hexdump(memoriaPrincipal, tamanioMemoria);
			sleep(2);
			log_info(loggerMiram, "Tripulante: %d termino sus tareas", tid); 
			eliminarTripulanteSegmentacion(pid, tid);
			//expulsarTripulanteDelMapa(tid);
			
			  

		}
		else{	
			log_info(loggerMiram, "La tarea a enviar al tripulante %d es: %s", tid, stringTarea);
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

		log_info(loggerMiram, "Header 4 recibido: ACTUALIZAR POSICION"); 
	
		
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

		log_info(loggerMiram,"El tripulante %d se movio hacia %d|%d",tripulanteid,posx,posy);

		//moverTripulanteEnMapa(tripulanteid,posx,posy);

		actualizarPosicionTripulanteSegmentacion(patotaid, tripulanteid, posx, posy);

	break; 

	case ACTUALIZAR_ESTADO: ;

		log_info(loggerMiram, "Header 5 recibido: ACTUALIZAR ESTADO"); 
		uint32_t trip = 0, pat = 0;
		char estadoNuevo = 'a';
		
		memcpy(&trip, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		memcpy(&pat, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		memcpy(&estadoNuevo, stream, sizeof(char)); 
		actualizarEstadoTripulanteSegmentacion(pat, trip, estadoNuevo); 
		log_info(loggerMiram, "Tripulante %d actualizo cambio su estado a: %c", trip, estadoNuevo); 
		//mem_hexdump(memoriaPrincipal, tamanioMemoria);
	
	break; 
 
	case EXPULSAR_TRIPULANTE: ;

		log_info(loggerMiram, "Header 6 recibido: EXPULSAR TRIPULANTE"); 
		
		uint32_t tripid = 0, patid = 0;
		
		memcpy(&tripid, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		memcpy(&patid, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		eliminarTripulanteSegmentacion(patid, tripid);
		//expulsarTripulanteDelMapa(tripid);
		log_info(loggerMiram, "El tripulante %d fue expulsado", tripid); 

	break ; 


	default: ;



		break;
	}

	close(socketCliente);
	return NULL;

}


//GENERALES

void iniciarMemoria(){
	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0){
		tablaSegmentosGlobal = list_create(); 
		listaReferenciasPatotaSegmentacion = list_create();
		t_segmento * segmento = malloc(sizeof(t_segmento)); 
		segmento -> base = 0; 
		segmento -> tamanio = tamanioMemoria; 
		segmento -> ocupado = false; 
		list_add(tablaSegmentosGlobal, segmento);
		sem_init(&semaforoCompactacion, 0, 0); 
		pthread_mutex_init(&mutexTablaSegmentosGlobal, NULL);
		pthread_mutex_init(&mutexMemoriaPrincipal, NULL);
		pthread_mutex_init(&mutexListaReferenciasPatotas, NULL);
	}
}
int buscarEspacioNecesario(int tamanioTareas, int cantidadTripulantes) {
	log_info(loggerMiram, "Se necesitan: %d bytes. Buscando espacio necesario...", tamanioTareas + cantidadTripulantes * SIZEOF_TCB + SIZEOF_PCB);
	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

		return buscarEspacioNecesarioSegmentacion(tamanioTareas, cantidadTripulantes);
		
	}
	else{
		return -1; 
	}
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

 
PCB * crearPCB(uint32_t pid){

	PCB * patota = malloc(SIZEOF_PCB);
	patota->pid = pid;
	patota->tareas = SIZEOF_PCB;
	
	
	
	return patota; 
}

//MANEJO DE SEÑALES

void * hiloSIGUSR1(){
	signal(SIGUSR1, sig_handler); 

	return NULL; 
}

void * hiloSIGUSR2(){
	signal(SIGUSR2, sig_handler); 

	return NULL ;
}

void sig_handler(int senial){

	if(senial == SIGUSR1){
		if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
			dumpMemoriaSegmentacion();  
		}
		if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
			//dumpDeMemoriaPaginacion();
		}
	}
	
	if(senial == SIGUSR2 && (strcmp(esquemaMemoria, "SEGMENTACION") == 0)){
		compactarMemoriaSegmentacion();
	}
	
	
	
}




//---------------------------SEGMENTACION------------
int buscarEspacioNecesarioSegmentacion(int tamanioTareas, int cantidadTCBs){
    int espacioNecesario = tamanioTareas + cantidadTCBs * SIZEOF_TCB + SIZEOF_PCB; 
    int bytesDisponibles = 0; 
    
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
    t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre);
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal); 

    if(segmentosLibres == NULL){
       
        return -1; 
    }
    else{
        for(int i = 0; i<list_size(segmentosLibres); i++){
            t_segmento * segmento = list_get(segmentosLibres, i); 
            bytesDisponibles = bytesDisponibles + segmento->tamanio; 
        }
        if(bytesDisponibles >= espacioNecesario){
            return 1; 
        }
        else{
            return -1; 
        }
        
    } 
}

uint32_t buscarSegmentoLibre(int tamanioContenido){
	if(strcmp(criterioSeleccion, "FIRST_FIT") == 0){
		return firstFitSegmentacion(tamanioContenido); 
	}
	if(strcmp(criterioSeleccion, "BEST_FIT") == 0){
		return bestFitSegmentacion(tamanioContenido); 
	}
	else{
		log_error(loggerMiram, "No se reconoce el criterio de Seleccion"); 
		return 0; 
	}
}

uint32_t bestFitSegmentacion(int tamanioContenido){
	
	log_info(loggerSegmentacion, "Ingrese a best fit"); 
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre); 
	for(int i =0; i< list_size(segmentosLibres); i++){
		t_segmento * segmento = list_get(segmentosLibres, i);
		log_info(loggerSegmentacion, "Segmento libre que empieza en: %d y tiene un tamanio de: %d", segmento->base, segmento->tamanio);
		
	}
	
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal); 

	bool cabeElContenido(t_segmento * segmento){

			if((segmento->tamanio) >= tamanioContenido){
				return true; 
			}
			else{
				return false; 
			}
		}

	if(!list_any_satisfy(segmentosLibres, cabeElContenido)){
		log_info(loggerMiram, "El contenido no entra en ningun segmento libre. Es necesario compactar.");
		log_info(loggerSegmentacion, "El contenido no entra en ningun segmento libre. Es necesario compactar y volver a buscar un segmento disponible."); 
		compactarMemoriaSegmentacion(); 
		return bestFitSegmentacion(tamanioContenido); 
	}

	t_list * segmentosPosibles = list_filter(segmentosLibres, cabeElContenido); 
	list_sort(segmentosPosibles, segmentoMasPequenio); 
	t_segmento * segmentoElegido = list_get(segmentosPosibles, 0); 
	log_info(loggerSegmentacion, "El segmento elegido empieza en: %d y tiene un tamanio de: %d", segmentoElegido->base, segmentoElegido->tamanio);

	bool coincideBase(t_segmento * segmento){
		return (segmento->base == segmentoElegido->base); 
	}

	t_segmento * nuevoSegmentoLibre = malloc(sizeof(t_segmento)); 
	t_segmento * nuevoSegmentoOcupado = malloc(sizeof(t_segmento)); 
	nuevoSegmentoOcupado->base = segmentoElegido->base; 
	nuevoSegmentoOcupado->tamanio = tamanioContenido; 
	nuevoSegmentoOcupado->ocupado = true; 

	nuevoSegmentoLibre->base = segmentoElegido->base + tamanioContenido; 
	nuevoSegmentoLibre->tamanio = segmentoElegido->tamanio - tamanioContenido; 
	nuevoSegmentoLibre->ocupado = false;


	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	list_remove_and_destroy_by_condition(tablaSegmentosGlobal, coincideBase, free); 
	list_add(tablaSegmentosGlobal, nuevoSegmentoOcupado); 
	list_add(tablaSegmentosGlobal, nuevoSegmentoLibre);
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal); 

	return nuevoSegmentoOcupado->base; 

}

uint32_t firstFitSegmentacion(int tamanioContenido){
	log_info(loggerSegmentacion, "Ingrese a first fit");
	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	list_sort(tablaSegmentosGlobal, seEncuentraPrimeroEnMemoria); 
	t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre);
	 
	for(int i = 0; i < list_size(segmentosLibres); i++){
		t_segmento * segmento = list_get(segmentosLibres, i); 
		log_info(loggerSegmentacion, "Segmento libre que empieza en: %d y tiene un tamanio de: %d", segmento->base, segmento->tamanio);

		
	}
	 
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal); 
	
	
	bool cabeElContenido(t_segmento * segmento){

			if((segmento->tamanio) >= tamanioContenido){
				return true; 
			}
			else{
				return false; 
			}
		}
	
	
	if(!list_any_satisfy(segmentosLibres, cabeElContenido)){
		log_info(loggerMiram, "El contenido no entra en ningun segmento libre. Es necesario compactar.");
		log_info(loggerSegmentacion, "El contenido no entra en ningun segmento libre. Es necesario compactar y volver a buscar un segmento disponible."); 
		compactarMemoriaSegmentacion(); 
		return firstFitSegmentacion(tamanioContenido);
		 

	}

	

	int i = 0; 
	
	t_segmento * segmentoElegido = list_get(segmentosLibres, i); 
	 

	while((segmentoElegido->tamanio) < tamanioContenido){
		i++; 
		segmentoElegido = list_get(segmentosLibres, i);
		
	}
	log_info(loggerSegmentacion, "El segmento elegido empieza en: %d y tiene un tamanio de: %d", segmentoElegido->base, segmentoElegido->tamanio);

	bool coincideBase(t_segmento * segmento){
		return (segmento->base == segmentoElegido->base); 
	}
	
	
	t_segmento * nuevoSegmentoLibre = malloc(sizeof(t_segmento)); 
	t_segmento * nuevoSegmentoOcupado = malloc(sizeof(t_segmento)); 
	nuevoSegmentoOcupado->base = segmentoElegido->base; 
	nuevoSegmentoOcupado->tamanio = tamanioContenido; 
	nuevoSegmentoOcupado->ocupado = true; 

	nuevoSegmentoLibre->base = segmentoElegido->base + tamanioContenido; 
	nuevoSegmentoLibre->tamanio = segmentoElegido->tamanio - tamanioContenido; 
	nuevoSegmentoLibre->ocupado = false;


	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	list_remove_and_destroy_by_condition(tablaSegmentosGlobal, coincideBase, free); 
	list_add(tablaSegmentosGlobal, nuevoSegmentoOcupado); 
	list_add(tablaSegmentosGlobal, nuevoSegmentoLibre);
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);

	
	
	return nuevoSegmentoOcupado->base;
} 

uint32_t asignarMemoriaSegmentacionTCB(void * tripulante, int tripulanteID, t_list * tablaSegmentos, int pid){

		uint32_t direccionLogica = buscarSegmentoLibre(SIZEOF_TCB); 
		
		pthread_mutex_lock(&mutexMemoriaPrincipal);
		memcpy(memoriaPrincipal + direccionLogica, tripulante, SIZEOF_TCB); 
		pthread_mutex_unlock(&mutexMemoriaPrincipal); 
		   

		bool coincideBase(t_segmento * segmento){
			return (segmento->base == direccionLogica);
		}
		
		pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
		t_segmento * segmentoNuevo = list_find(tablaSegmentosGlobal, coincideBase);
		segmentoNuevo ->tid = tripulanteID;
		segmentoNuevo -> tamanio = SIZEOF_TCB; 
		segmentoNuevo -> base = direccionLogica;
		segmentoNuevo->tipoSegmento = SEG_TCB;
		segmentoNuevo->pid = pid;
		segmentoNuevo->ocupado = true;
		pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
		list_add(tablaSegmentos, segmentoNuevo); 

		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionPCB(void * pcb , t_list * tablaSegmentos){
	
		
		uint32_t direccionLogica = buscarSegmentoLibre(SIZEOF_PCB); 
		
		pthread_mutex_lock(&mutexMemoriaPrincipal);
		memcpy(memoriaPrincipal + direccionLogica , pcb, SIZEOF_PCB);
		pthread_mutex_unlock(&mutexMemoriaPrincipal); 

		int pid = 0;
		memcpy(&pid, pcb, 4);

		bool coincideBase(t_segmento * segmento){
			return (segmento->base == direccionLogica);
		}
		
		pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
		t_segmento * segmentoNuevo = list_find(tablaSegmentosGlobal, coincideBase);
		pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
		segmentoNuevo->tid = -1; 
		segmentoNuevo->tamanio = SIZEOF_PCB; 
		segmentoNuevo->base = direccionLogica;
		segmentoNuevo->tipoSegmento = SEG_PCB;
		segmentoNuevo->pid = pid;
		segmentoNuevo->ocupado = true;
		

		list_add_in_index(tablaSegmentos, 0,segmentoNuevo);
		   
		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionTareas(char * tareas, int tamanioTareas, t_list * tablaSegmentos, int pid){

	
	uint32_t direccionLogica = buscarSegmentoLibre(tamanioTareas); 
	

	
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	memcpy(memoriaPrincipal + direccionLogica, tareas, tamanioTareas);
	pthread_mutex_unlock(&mutexMemoriaPrincipal); 

	bool coincideBase(t_segmento * segmento){
			return (segmento->base == direccionLogica);
		}

	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	t_segmento * segmentoNuevo = list_find(tablaSegmentosGlobal, coincideBase);
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
	segmentoNuevo->tid = -1; 
	segmentoNuevo->tamanio = tamanioTareas; 
	segmentoNuevo->base = direccionLogica;
	segmentoNuevo->tipoSegmento = SEG_TAREAS;
	segmentoNuevo->pid = pid;
	segmentoNuevo->ocupado = true;
	

	list_add(tablaSegmentos, segmentoNuevo); 
	//mem_hexdump(memoriaPrincipal, tamanioMemoria);

	return direccionLogica;

}

void actualizarPosicionTripulanteSegmentacion(uint32_t idPatota, uint32_t idTripulante, uint32_t nuevaPosx, uint32_t nuevaPosy){
	uint32_t direccionTripulante = obtenerDireccionTripulanteSegmentacion(idPatota, idTripulante); 
	pthread_mutex_lock(&memoriaPrincipal); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t), &nuevaPosx, sizeof(uint32_t)); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t)*2, &nuevaPosy, sizeof(uint32_t));
	pthread_mutex_unlock(&memoriaPrincipal); 
} 

void actualizarEstadoTripulanteSegmentacion(uint32_t pid, uint32_t tid, char estadoNuevo){
	uint32_t direccionTripulante = obtenerDireccionTripulanteSegmentacion(pid, tid); 
	//printf("Entre a actualizar estado \n");
	pthread_mutex_lock(&mutexMemoriaPrincipal); 
	//printf("Entre al mutex de actualizar estado \n");
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t)*3, &estadoNuevo, sizeof(char)); 
	//printf("hice el memcpy actualizar estado \n");
	pthread_mutex_unlock(&mutexMemoriaPrincipal); 
	//printf("termine actualizarEstado \n");
} 

void eliminarTripulanteSegmentacion(uint32_t idPatota, uint32_t idTripulante){

	bool coincidePID(referenciaTablaPatota * unaReferencia){return unaReferencia->pid == idPatota; }
	bool coincideTID(t_segmento * unSegmento){return unSegmento->tid == idTripulante;}

	pthread_mutex_lock(&mutexListaReferenciasPatotas); 
	referenciaTablaPatota * referenciaPatota = list_find(listaReferenciasPatotaSegmentacion, coincidePID); 
	pthread_mutex_unlock(&mutexListaReferenciasPatotas); 

	if(referenciaPatota == NULL){ return;}
	t_segmento * segmentoTripulante = list_find(referenciaPatota->tablaPatota, coincideTID); 

	if(segmentoTripulante == NULL || segmentoTripulante->ocupado == false){
		
		return ;
	}
	
	memset(memoriaPrincipal + segmentoTripulante->base, 0, SIZEOF_TCB);
	list_remove_by_condition(referenciaPatota->tablaPatota, coincideTID);
	segmentoTripulante->ocupado = false; 
	
	esElUltimoTripulante(idPatota); 


}

void esElUltimoTripulante(uint32_t idPatota){

	
	bool coincidePID(referenciaTablaPatota * unaReferencia){return unaReferencia->pid == idPatota;}
	pthread_mutex_lock(&mutexListaReferenciasPatotas); 
	
	referenciaTablaPatota * referenciaPatota = list_find(listaReferenciasPatotaSegmentacion, coincidePID); 
	
	
	pthread_mutex_unlock(&mutexListaReferenciasPatotas);

	t_list * tablaPatota = referenciaPatota->tablaPatota; 
	if(tablaPatota == NULL ){
		log_info(loggerMiram, "Referencia nula");
	}
	//t_list * segmentosOcupados = list_filter(tablaPatota, segmentoOcupado); 
	if(list_size(tablaPatota) == 2){
		log_info(loggerMiram, "Todos los tripulantes de la patota %d terminaron sus tareas. Eliminando patota...", idPatota);
		t_segmento * segmentoTareas = list_get(tablaPatota, 1);
		
		t_segmento * segmentoPCB = list_get(tablaPatota, 0); 
		
		memset(memoriaPrincipal + segmentoTareas->base, 0, segmentoTareas->tamanio); 
		memset(memoriaPrincipal + segmentoPCB->base, 0 , segmentoPCB->tamanio); 
		segmentoTareas->ocupado = false; 
		segmentoPCB->ocupado = false; 
		
		list_remove_by_condition(listaReferenciasPatotaSegmentacion, coincidePID);
		log_info(loggerMiram, "Patota %d eliminada.", idPatota);
		
	}
	
}
char * obtenerProximaTareaSegmentacion(uint32_t direccionTarea, uint32_t direccionTCB){
	char * tareaObtenida = malloc(40); 
	char caracterComparacion = 'a'; 
	int desplazamiento = 0;  
	char termino = '\0';


	//printf("ENtre a obtener proxima tarea \n direccionTArea: %d \n" , direccionTarea); 
	if(direccionTarea == (tamanioMemoria + 1)){
		//printf("entre aca direccionTarea == tamanioMemoria+1 \n");
		return "NO_HAY_TAREA"; 
	}
	
	//printf("Saltee el no hay tarea \n"); 
	while(caracterComparacion != '\n' && caracterComparacion != '|'){
		//printf("Entre al while"); 
		pthread_mutex_lock(&mutexMemoriaPrincipal); 
		memcpy(tareaObtenida + desplazamiento, memoriaPrincipal+direccionTarea+desplazamiento, 1 ); 
		memcpy(&caracterComparacion, memoriaPrincipal + direccionTarea + desplazamiento + 1, 1); 
		pthread_mutex_unlock(&mutexMemoriaPrincipal); 
		//printf("Caracter de comparcaion %c \n", caracterComparacion);
		desplazamiento ++; 
	}

	if(caracterComparacion == '\n'){
		//printf("Caracter de comparcaion %c \n", caracterComparacion);
		int direccionProximaTarea = direccionTarea + desplazamiento +1;
		actualizarProximaTareaSegmentacion(direccionTCB, direccionProximaTarea);
	}

	if(caracterComparacion == '|'){
		//printf("Caracter de comparcaion %c \n", caracterComparacion);
		actualizarProximaTareaSegmentacion(direccionTCB, tamanioMemoria + 1); 	
	}

	//preguntar con bene: el desplazamiento seria el tamaño de la tarea? convendria entonces copiarla a otra variable de ese tamaño exacto 
	//y luego hacer un free al malloc mas grande?? 
	memcpy(tareaObtenida + desplazamiento, &termino, 1);
	return tareaObtenida;
}

void actualizarProximaTareaSegmentacion(uint32_t direccionTCB, uint32_t direccionProximaTarea){
	//printf("DIreccion proxima tarea: %x \n", direccionProximaTarea);
	pthread_mutex_lock(&mutexMemoriaPrincipal); 
	memcpy(memoriaPrincipal + direccionTCB + 3* sizeof(uint32_t) + sizeof(char), &direccionProximaTarea, sizeof(uint32_t));
	pthread_mutex_unlock(&mutexMemoriaPrincipal); 
	//printf("Ya actualice la direccion de la proxima tarea \n");
	//mem_hexdump(memoriaPrincipal, tamanioMemoria);
}

void compactarMemoriaSegmentacion(){


	compactacion = true; 
	log_info(loggerMiram, "Iniciando compactacion de la memoria...");
	sleep(2); 
	
	mem_hexdump(memoriaPrincipal, tamanioMemoria);
	
	t_list * listaLibres = list_filter(tablaSegmentosGlobal, segmentoOcupado);
	tablaSegmentosGlobal = listaLibres; 
	

	list_sort(tablaSegmentosGlobal, seEncuentraPrimeroEnMemoria); 
	if(list_size(tablaSegmentosGlobal)>0){

		
		t_segmento * primerSegmento = list_get(tablaSegmentosGlobal, 0); 
		 
		if(primerSegmento->base != 0){
			void * memAux = malloc(primerSegmento->tamanio);
			memcpy(memAux, memoriaPrincipal + primerSegmento->base, primerSegmento->tamanio); 
			memcpy(memoriaPrincipal, memAux, primerSegmento->tamanio); 
			free(memAux);
			actualizarEstructurasSegmentacion(primerSegmento, 0); 
			primerSegmento->base = 0;
			
		}
		
		for(int i = 0 ; i < list_size(tablaSegmentosGlobal)-1; i++){
			
			t_segmento * segmentoAnterior = list_get(tablaSegmentosGlobal, i); 
			t_segmento * segmentoActual = list_get(tablaSegmentosGlobal, i+1); 
			
			int nuevaBase = segmentoAnterior->base + segmentoAnterior->tamanio; 
			void * memAux = malloc(segmentoActual->tamanio); 
			memcpy(memAux, memoriaPrincipal + segmentoActual->base, segmentoActual->tamanio); 
			memcpy(memoriaPrincipal + nuevaBase, memAux, segmentoActual->tamanio); 
			free(memAux);
			actualizarEstructurasSegmentacion(segmentoActual, nuevaBase); 
			segmentoActual->base = nuevaBase; 
			mem_hexdump(memoriaPrincipal, tamanioMemoria);
		}
		
		t_segmento * ultimoSegmentoOcupado = list_get(tablaSegmentosGlobal, list_size(tablaSegmentosGlobal)-1); 
		t_segmento * segmentoLibre = malloc(sizeof(t_segmento)); 
		segmentoLibre->base = ultimoSegmentoOcupado->base + ultimoSegmentoOcupado->tamanio; 
		segmentoLibre->tamanio = tamanioMemoria - segmentoLibre->base; 
		segmentoLibre->ocupado = false; 
		memset(memoriaPrincipal + segmentoLibre->base, 0, segmentoLibre->tamanio); 
		list_add(tablaSegmentosGlobal, segmentoLibre); 
		
		
	}
	else{ 

		t_segmento * segmentoLibre = malloc(sizeof(t_segmento)); 
		segmentoLibre->base = 0 ; 
		segmentoLibre->tamanio = tamanioMemoria;
		list_clean_and_destroy_elements(tablaSegmentosGlobal, free);
		list_add(tablaSegmentosGlobal, segmentoLibre);
	}

	mem_hexdump(memoriaPrincipal, tamanioMemoria);

	compactacion = false; 
	int valorSemaforo = 0;
	sem_getvalue(&semaforoCompactacion, &valorSemaforo);
	valorSemaforo = valorSemaforo * (-1); 
	for(int i = 0 ; i< valorSemaforo; i++){
		sem_post(&semaforoCompactacion);
	}
	
	log_info(loggerMiram, "Compactacion de la memoria finalizada.");
	
}


void actualizarDireccionesTareasTCB(t_segmento * segmento, uint32_t baseAnterior, uint32_t nuevaBase){
	 
	 uint32_t direccionAnterior = 0; 
	 memcpy(&direccionAnterior, memoriaPrincipal + segmento->base + 13, sizeof(uint32_t)); 
	 if(direccionAnterior != tamanioMemoria + 1){
	 	int offsetTareas = direccionAnterior - baseAnterior; 
	 	nuevaBase = nuevaBase + offsetTareas; 
	 	memcpy(memoriaPrincipal + segmento->base + 13, &nuevaBase, sizeof(uint32_t));
	 }
	  
}

void dumpMemoriaSegmentacion(){
	
	log_info(loggerMiram, "Iniciando dump de memoria..."); 

	char * fecha = temporal_get_string_time("%d-%m-%y_%H:%M:%S"); 
	char * nombreArchivo = string_from_format("dumpMemoria_%s.dmp", fecha);
	FILE* dump = fopen(nombreArchivo, "w+"); 
	
	fwrite("DUMP DE MEMORIA \n", 17, 1, dump); 
	

	


	 
	t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre); 
	


	//PARA LOS SEGMENTOS OCUPADOS
	fwrite("Segmentos Ocupados \n\n", 21, 1, dump); 
	
	if(list_any_satisfy(tablaSegmentosGlobal, segmentoOcupado)){
		for(int i = 0; i<list_size(listaReferenciasPatotaSegmentacion); i++){
			referenciaTablaPatota * referenciaActual = list_get(listaReferenciasPatotaSegmentacion, i); 
			t_list * segmentosOcupados = list_filter(referenciaActual->tablaPatota, segmentoOcupado); 

			for(int x = 0; x<list_size(segmentosOcupados); x++){
				t_segmento * segmentoActual = list_get(segmentosOcupados, x); 
				char * escribirSegmento = string_from_format("Proceso: %2d ---- Segmento: %2d -- Inicio: %3d -- Tamanio: %3d \n", referenciaActual->pid, x, segmentoActual->base, segmentoActual->tamanio); 
				fwrite(escribirSegmento, strlen(escribirSegmento), 1, dump); 
				
			}
		}
	}else{
		fwrite("No hay segmentos ocupados.\n", 27, 1, dump);
		
	}

	//PARA LOS SEGMENTOS LIBRES
	fwrite("\n\nSegmentos Libres \n\n",21,1, dump); 
	
	if(list_size(segmentosLibres)> 0){
		list_sort(segmentosLibres, seEncuentraPrimeroEnMemoria); 
		for (int i = 0; i < list_size(segmentosLibres); i++){
		t_segmento * segmentoActual = list_get(segmentosLibres, i); 
		char * escribirSegmento = string_from_format("Segmento libre %2d -- Inicio: %3d -- Tamanio: %3d\n", i, segmentoActual->base, segmentoActual->tamanio);
		fwrite(escribirSegmento, strlen(escribirSegmento), 1, dump); 
		
	}
	}else{
		fwrite("No hay segmentos libres.\n", 25, 1, dump); 
		
	}

	fclose(dump); 
	log_info(loggerMiram, "Dump de memoria finalizado.");
	
}

uint32_t obtenerDireccionTripulanteSegmentacion(uint32_t idPatota, uint32_t idTripulante){

	bool coincidePID(referenciaTablaPatota * unaReferencia){return unaReferencia->pid == idPatota; }
	bool coincideTID(t_segmento * unSegmento){return unSegmento->tid == idTripulante;}
	
	pthread_mutex_lock(&mutexListaReferenciasPatotas); 
	referenciaTablaPatota * referenciaPatota = list_find(listaReferenciasPatotaSegmentacion, coincidePID); 
	pthread_mutex_unlock(&mutexListaReferenciasPatotas); 

	if(referenciaPatota == NULL){ return tamanioMemoria+1;}
	t_segmento * segmentoTripulante = list_find(referenciaPatota->tablaPatota, coincideTID); 

	if(segmentoTripulante == NULL || segmentoTripulante->ocupado == false){
		
		return tamanioMemoria + 1; 
	}

	//printf("Encontre al tripulante. Base: %x \n", segmentoTripulante->base);
	return segmentoTripulante->base;

}
//FUNCIONES AUXILIARES SEGMENTACION

void actualizarEstructurasSegmentacion(t_segmento * segmento,  uint32_t nuevaBase){

	bool coincidePID(referenciaTablaPatota * referencia){ return referencia->pid == segmento->pid;}

	if(segmento->tipoSegmento == SEG_TAREAS){
		
		referenciaTablaPatota * referencia = list_find(listaReferenciasPatotaSegmentacion, coincidePID); 
		t_segmento * segmentoPCB = list_get(referencia->tablaPatota, 0); 
		memcpy(memoriaPrincipal + segmentoPCB->base + sizeof(uint32_t), &nuevaBase, sizeof(uint32_t)); 
		for(int i = 2 ; i<list_size(referencia->tablaPatota); i++){
			t_segmento * segmentoTCB = list_get(referencia->tablaPatota, i); 
			actualizarDireccionesTareasTCB(segmentoTCB, segmento->base, nuevaBase); 
		}
	}
	if(segmento->tipoSegmento == SEG_PCB){
		
		referenciaTablaPatota * referencia =list_find(listaReferenciasPatotaSegmentacion, coincidePID);
		for(int i = 2 ; i<list_size(referencia->tablaPatota); i++){
			t_segmento * segmento = list_get(referencia->tablaPatota, i); 
			memcpy(memoriaPrincipal + segmento->base + 17, &nuevaBase, sizeof(uint32_t)); 
		}
	}
}

bool segmentoLibre(t_segmento * segmento){
	return !(segmento->ocupado); 
}

bool segmentoOcupado(t_segmento * segmento){
	return segmento->ocupado;
}

bool segmentoMasPequenio(t_segmento * unSegmento, t_segmento * otroSegmento){

	if((unSegmento ->tamanio <= (otroSegmento->tamanio))){
		return true; 
	}

	else{
		return false; 
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


// ------------------------------------------------------ MAPA ----------------------------------------------

// void iniciarMapa() {
// 	sem_init(&semaforoMoverTripulante,0,1);
// 	sem_init(&semaforoTerminarMapa,0,0);
// 	nivel_gui_inicializar();
// 	pthread_mutex_lock(&semaforoTerminarMapa);
// }

// void agregarTripulanteAlMapa(uint32_t tid, uint32_t x, uint32_t y) { 
// 	char id = idMapa(tid); //aca el tid es un uint  
//     personaje_crear(navePrincipal, id, x, y);
// 	nivel_gui_dibujar(navePrincipal);
// }

// void moverTripulanteEnMapa(uint32_t tid, uint32_t x, uint32_t y){
// 	char id = idMapa(tid);
// 	pthread_mutex_lock(&semaforoMoverTripulante);
// 	item_mover(navePrincipal, id, x, y);
// 	nivel_gui_dibujar(navePrincipal);
// 	pthread_mutex_unlock(&semaforoMoverTripulante);
// }

// void expulsarTripulanteDelMapa(uint32_t tid) {
// 	char id = idMapa(tid);
// 	item_borrar(navePrincipal, id);
// 	nivel_gui_dibujar(navePrincipal);
// }

// char idMapa(uint32_t tid){

// 	char id = 0;

// 	id = tid + 65;

// 	if (id > 90) {
// 		id += 6;
// 	}
// 	return id;

// }