#include "mi_ram.h"


int main(int argc, char ** argv){

	navePrincipal = nivel_crear("Nave Principal");
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

	pthread_t mapa;
	pthread_create(&mapa, NULL, iniciarMapa, NULL);
	pthread_join(mapa, NULL);
	nivel_destruir(navePrincipal); 
	nivel_gui_terminar();

	 	
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
	path_SWAP = config_get_string_value(config, "PATH_SWAP");
	tamanioSwap = config_get_int_value(config, "TAMANIO_SWAP");
	ipRam = config_get_string_value(config, "IP");

}
void servidorPrincipal() {
	int listeningSocket = crear_conexionServer(puertoMemoria, ipRam);

	

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
		log_info(loggerMiram, "PID: %c", idMapa(idPatota)); 
		//log_info(loggerMiram, "Tareas recibidas: %s Tamanio de las tareas: %d", tareas, tamanioTareas); 
		log_info(loggerMiram, "Cantidad de tripulantes: %d \n", cantidadTCBs);

		//Nos aseguramos de que hay espacio para recibir la patota 
		
		int hayLugar = buscarEspacioNecesario(tamanioTareas, cantidadTCBs);

		if(hayLugar == -1){
			log_info(loggerMiram, "Rechazo patota por falta de espacio en memoria.");
					
		}
			
				
		if(hayLugar == 1){
			log_info(loggerMiram, "Espacio necesario existente. Agrego patota a memoria...");

			if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
				
				
				referenciaTablaPatota * referencia = malloc(sizeof(referenciaTablaPatota)); 
				referencia->tablaPatota = list_create();
				pthread_mutex_init(&(referencia->semaforoPatota), NULL);
			
				PCB * pcb = crearPCB(idPatota);
				referencia->pid = pcb->pid;
				void * streamPCB = malloc(SIZEOF_PCB);
				memcpy(streamPCB, &(pcb->pid), sizeof(uint32_t)); 
				memset(streamPCB + sizeof(uint32_t), 0, sizeof(uint32_t)); 
				
				 
				//printf("ID DE LA PATOTA %d \n", pcb->pid);
				uint32_t direccionPCB = asignarMemoriaSegmentacionPCB(streamPCB, referencia);
				log_info(loggerMiram, "PCB de la patota %d agregado a memoria", pcb->pid);  
				pthread_mutex_lock(&mutexListaReferenciasPatotas); 
				list_add(listaReferenciasPatotaSegmentacion, referencia);
				pthread_mutex_unlock(&mutexListaReferenciasPatotas);
				
				
				uint32_t direccionTareas = asignarMemoriaSegmentacionTareas(tareas, tamanioTareas, referencia, (pcb->pid)); 
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
 
					
					

					uint32_t direccionLogica = asignarMemoriaSegmentacionTCB(tripulante, tripulanteID, referencia, (pcb->pid)); 
					log_info(loggerMiram, "Tripulante agregado a memoria. TID: %d", tripulanteID);
					
					

				agregarTripulanteAlMapa(tripulanteID, posx, posy);
				
			
				}


			int headerAEnviar = PATOTA_CREADA; 
			send(socketCliente, &headerAEnviar, sizeof(int), 0); 

			
			
			}

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

					list_add(listaTripulantes, tripu);
					log_info(loggerMiram, "Tripulante agregado a memoria. TID: %d", tid);

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

				//mem_hexdump(memoriaPrincipal, tamanioMemoria);

				//dumpDeMemoriaPaginacion();
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
		
		
		stringTarea = obtenerProximaTarea(pid, tid); 
		
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
			if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {sleep(2);}
			
			 
			eliminarTripulante(pid, tid);
			log_info(loggerMiram, "Tripulante: %d termino sus tareas", tid);
			expulsarTripulanteDelMapa(tid);
			

			  

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

		moverTripulanteEnMapa(tripulanteid,posx,posy);

		actualizarPosicionTripulante(patotaid, tripulanteid, posx, posy);

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
		actualizarEstadoTripulante(pat, trip, estadoNuevo); 
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
		eliminarTripulante(patid, tripid);
		expulsarTripulanteDelMapa(tripid);
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
		pthread_mutex_init(&mutexListaTripulantes, NULL);
		contadorLRU = 0;
		iniciarFrames();
		iniciarSwap();	
	}
}

int buscarEspacioNecesario(int tamanioTareas, int cantidadTripulantes) {

	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {

		return buscarEspacioNecesarioSegmentacion(tamanioTareas, cantidadTripulantes);
		
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
			// for(int i = 0; i < framesNecesariosEnSwap; i++) {
			// 	llevarPaginaASwap();
			// }
			return 1;
		}

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

char * obtenerProximaTarea(uint32_t idPatota, uint32_t tid) {
	
	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		uint32_t direccionTCB = obtenerDireccionTripulanteSegmentacion(idPatota, tid);
		if(direccionTCB == tamanioMemoria + 1){
			return  "TRIPULANTE_ELIMINADO";
		}
		 
		uint32_t direccionTarea = 0; 
		
		pthread_mutex_lock(&mutexMemoriaPrincipal); 
		memcpy(&direccionTarea, memoriaPrincipal + direccionTCB + sizeof(uint32_t)*3 + sizeof(char), sizeof(uint32_t));
		pthread_mutex_unlock(&mutexMemoriaPrincipal);
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

void actualizarPosicionTripulante(uint32_t pid, uint32_t tid, uint32_t posx, uint32_t posy){

	if(strcmp(esquemaMemoria, "SEGMENTACION") == 0 ){
		actualizarPosicionTripulanteSegmentacion(pid, tid, posx, posy); 
	}

	if(strcmp(esquemaMemoria, "PAGINACION") == 0) {
		actualizarPosicionPaginacion(pid,  tid, posx, posy);
	}
}

void eliminarTripulante(uint32_t patid, uint32_t tripid) {
	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		eliminarTripulanteSegmentacion(patid, tripid);
	}	
	if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
		eliminarTripulantePaginacion(patid, tripid);
	}
}

void actualizarEstadoTripulante(uint32_t pid, uint32_t tid, char estadoNuevo) {
	if (strcmp(esquemaMemoria, "SEGMENTACION") == 0) {
		actualizarEstadoTripulanteSegmentacion(pid, tid, estadoNuevo);
	}	
	if (strcmp(esquemaMemoria, "PAGINACION") == 0) {
		actualizarEstadoPaginacion(pid, tid, estadoNuevo);
	}
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
			dumpDeMemoriaPaginacion();
		}
	}
	
	if(senial == SIGUSR2 && (strcmp(esquemaMemoria, "SEGMENTACION") == 0)){
		compactarMemoriaSegmentacion();
	}
	
	
	
}




//---------------------------SEGMENTACION------------
int buscarEspacioNecesarioSegmentacion(int tamanioTareas, int cantidadTCBs){
	log_info(loggerSegmentacion, "INgrese a buscarEspacioNecesario");
    int espacioNecesario = tamanioTareas + cantidadTCBs * SIZEOF_TCB + SIZEOF_PCB; 
    int bytesDisponibles = 0; 
    log_info(loggerSegmentacion, "Antes del mutex");

	if(tablaSegmentosGlobal == NULL ){
		log_info(loggerSegmentacion, "APunta a nulo la tabla global :/");
	}
	else
	{
		log_info(loggerSegmentacion, "no apunta a nulo la tabla global");
		log_info(loggerSegmentacion, "Tamanio tabla global :%d", list_size(tablaSegmentosGlobal));
	}
	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	//log_info(loggerSegmentacion, "Tamanio tabla segmentos global buscar espacio: %d", list_size(tablaSegmentosGlobal)); 
    t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre);
	//log_info(loggerSegmentacion, "Tamanio tabla segmnentos libres: %d", list_size(segmentosLibres));
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
	log_info(loggerSegmentacion, "Entre al mutex first fit");
	list_sort(tablaSegmentosGlobal, seEncuentraPrimeroEnMemoria); 
	log_info(loggerSegmentacion, "Despues del sort");
	t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre);
	if(segmentosLibres == NULL){
		log_info(loggerSegmentacion, "La referencia a segmentos libres es nula");
	}
	log_info(loggerSegmentacion, "El tamanio de segmentosLibres: %d", list_size(segmentosLibres)); 
	 
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
	
	
	
	t_segmento * nuevoSegmentoOcupado = malloc(sizeof(t_segmento)); 
	nuevoSegmentoOcupado->base = segmentoElegido->base; 
	nuevoSegmentoOcupado->tamanio = tamanioContenido; 
	nuevoSegmentoOcupado->ocupado = true; 
	if(segmentoElegido->tamanio != tamanioContenido){
		t_segmento * nuevoSegmentoLibre = malloc(sizeof(t_segmento)); 
		nuevoSegmentoLibre->base = segmentoElegido->base + tamanioContenido; 
		nuevoSegmentoLibre->tamanio = segmentoElegido->tamanio - tamanioContenido; 
		nuevoSegmentoLibre->ocupado = false;
		pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
		list_add(tablaSegmentosGlobal, nuevoSegmentoLibre);
		pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
	}
	

	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	list_remove_and_destroy_by_condition(tablaSegmentosGlobal, coincideBase, free); 
	list_add(tablaSegmentosGlobal, nuevoSegmentoOcupado); 
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
	
	
	
	return nuevoSegmentoOcupado->base;
} 

uint32_t asignarMemoriaSegmentacionTCB(void * tripulante, int tripulanteID, referenciaTablaPatota * referencia, int pid){

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
		pthread_mutex_lock(&(referencia->semaforoPatota)); 
		list_add(referencia->tablaPatota, segmentoNuevo); 
		pthread_mutex_unlock(&(referencia->semaforoPatota));


		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionPCB(void * pcb , referenciaTablaPatota * referencia){
	
		
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
		segmentoNuevo->tid = -1; 
		segmentoNuevo->tamanio = SIZEOF_PCB; 
		segmentoNuevo->base = direccionLogica;
		segmentoNuevo->tipoSegmento = SEG_PCB;
		segmentoNuevo->pid = pid;
		segmentoNuevo->ocupado = true;
		pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
		
		pthread_mutex_lock(&(referencia->semaforoPatota)); 
		list_add_in_index(referencia->tablaPatota, 0,segmentoNuevo);
		pthread_mutex_unlock(&(referencia->semaforoPatota));
		   
		return direccionLogica;
}

uint32_t asignarMemoriaSegmentacionTareas(char * tareas, int tamanioTareas, referenciaTablaPatota * referencia, int pid){

	
	uint32_t direccionLogica = buscarSegmentoLibre(tamanioTareas); 
	

	
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	memcpy(memoriaPrincipal + direccionLogica, tareas, tamanioTareas);
	pthread_mutex_unlock(&mutexMemoriaPrincipal); 

	bool coincideBase(t_segmento * segmento){
			return (segmento->base == direccionLogica);
		}

	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	t_segmento * segmentoNuevo = list_find(tablaSegmentosGlobal, coincideBase);
	segmentoNuevo->tid = -1; 
	segmentoNuevo->tamanio = tamanioTareas; 
	segmentoNuevo->base = direccionLogica;
	segmentoNuevo->tipoSegmento = SEG_TAREAS;
	segmentoNuevo->pid = pid;
	segmentoNuevo->ocupado = true;
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
	
	pthread_mutex_lock(&(referencia->semaforoPatota)); 
	list_add(referencia->tablaPatota, segmentoNuevo); 
	pthread_mutex_unlock(&(referencia->semaforoPatota)); 
	//mem_hexdump(memoriaPrincipal, tamanioMemoria);

	return direccionLogica;

}

void actualizarPosicionTripulanteSegmentacion(uint32_t idPatota, uint32_t idTripulante, uint32_t nuevaPosx, uint32_t nuevaPosy){
	uint32_t direccionTripulante = obtenerDireccionTripulanteSegmentacion(idPatota, idTripulante); 
	if(direccionTripulante == tamanioMemoria + 1){
		log_info(loggerSegmentacion, "quisieron actualizar posicion de tripulante %d eliminado", idTripulante);
		return; 
	}
	pthread_mutex_lock(&mutexMemoriaPrincipal); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t), &nuevaPosx, sizeof(uint32_t)); 
	memcpy(memoriaPrincipal + direccionTripulante + sizeof(uint32_t)*2, &nuevaPosy, sizeof(uint32_t));
	pthread_mutex_unlock(&mutexMemoriaPrincipal); 
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
	log_info(loggerSegmentacion, "Entre a eliminar tripulante segmentacion PID: %d TID: %d", idPatota, idTripulante); 
	bool coincidePID(referenciaTablaPatota * unaReferencia){return unaReferencia->pid == idPatota; }
	bool coincideTID(t_segmento * unSegmento){return unSegmento->tid == idTripulante;}

	log_info(loggerSegmentacion, "Tamanio de la lista referencias: %d", list_size(listaReferenciasPatotaSegmentacion));
	//pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	pthread_mutex_lock(&mutexListaReferenciasPatotas); 
	referenciaTablaPatota * referenciaPatota = list_find(listaReferenciasPatotaSegmentacion, coincidePID); 
	pthread_mutex_unlock(&mutexListaReferenciasPatotas); 
	//pthread_mutex_unlock(&mutexTablaSegmentosGlobal);

	if(referenciaPatota == NULL){ log_info(loggerSegmentacion, "la referencia a la patota es nula");  return;}
	log_info(loggerSegmentacion, "La referencia a la patota no es nula"); 
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	pthread_mutex_lock(&(referenciaPatota->semaforoPatota)); 
	t_segmento * segmentoTripulante = list_find(referenciaPatota->tablaPatota, coincideTID); 
	pthread_mutex_unlock(&(referenciaPatota->semaforoPatota));
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);

	if(segmentoTripulante == NULL || segmentoTripulante->ocupado == false){
		log_info(loggerSegmentacion, "El segmento tripulante es nulo");
		return ;
	}
	log_info(loggerSegmentacion, "Encontre el segmento del tripulante a eliminar. Base: %d, tamanio: %d", segmentoTripulante->base, segmentoTripulante->tamanio);
	
	pthread_mutex_lock(&mutexMemoriaPrincipal); 
	memset(memoriaPrincipal + segmentoTripulante->base, 0, SIZEOF_TCB);
	pthread_mutex_unlock(&mutexMemoriaPrincipal);

	log_info(loggerSegmentacion, "Tamanio de la tabla de la patota %d antes del remove: %d", idPatota, list_size(referenciaPatota->tablaPatota)); 
	pthread_mutex_lock(&(referenciaPatota->semaforoPatota));
	list_remove_by_condition(referenciaPatota->tablaPatota, coincideTID);
	pthread_mutex_unlock(&(referenciaPatota->semaforoPatota));
	log_info(loggerSegmentacion, "Tamanio de la tabla de la patota %d despues del remove: %d", idPatota, list_size(referenciaPatota->tablaPatota)); 
	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
	segmentoTripulante->ocupado = false; 
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);


	
	
	esElUltimoTripulante(idPatota); 


}

void esElUltimoTripulante(uint32_t idPatota){

	log_info(loggerSegmentacion, "Entre a es el ultimo tripulante");
	bool coincidePID(referenciaTablaPatota * unaReferencia){return unaReferencia->pid == idPatota;}
	
	pthread_mutex_lock(&mutexListaReferenciasPatotas); 
	referenciaTablaPatota * referenciaPatota = list_find(listaReferenciasPatotaSegmentacion, coincidePID); 
	pthread_mutex_unlock(&mutexListaReferenciasPatotas);

	t_list * tablaPatota = referenciaPatota->tablaPatota; 
	if(tablaPatota == NULL ){
		log_info(loggerMiram, "Referencia nula");
	}
	//t_list * segmentosOcupados = list_filter(tablaPatota, segmentoOcupado); 
	pthread_mutex_lock(&(referenciaPatota->semaforoPatota));
	if(list_size(tablaPatota) == 2){
		log_info(loggerMiram, "Todos los tripulantes de la patota %d terminaron sus tareas. Eliminando patota...", idPatota);
		t_segmento * segmentoTareas = list_get(tablaPatota, 1);
		
		t_segmento * segmentoPCB = list_get(tablaPatota, 0); 
		
		memset(memoriaPrincipal + segmentoTareas->base, 0, segmentoTareas->tamanio); 
		memset(memoriaPrincipal + segmentoPCB->base, 0 , segmentoPCB->tamanio); 
		pthread_mutex_lock(&mutexTablaSegmentosGlobal); 
		segmentoTareas->ocupado = false; 
		segmentoPCB->ocupado = false; 
		pthread_mutex_unlock(&mutexTablaSegmentosGlobal);
		
		list_remove_by_condition(listaReferenciasPatotaSegmentacion, coincidePID);
		log_info(loggerMiram, "Patota %d eliminada.", idPatota);
		
	}
	pthread_mutex_unlock(&(referenciaPatota->semaforoPatota));
	
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
	
	//mem_hexdump(memoriaPrincipal, tamanioMemoria);
	
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
			//mem_hexdump(memoriaPrincipal, tamanioMemoria);
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
		segmentoLibre->ocupado = false;
		list_clean_and_destroy_elements(tablaSegmentosGlobal, free);
		list_add(tablaSegmentosGlobal, segmentoLibre);
	}

	//mem_hexdump(memoriaPrincipal, tamanioMemoria);

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
	
	fwrite("\n\nDUMP DE MEMORIA \n\n", 20, 1, dump); 
	
	pthread_mutex_lock(&mutexTablaSegmentosGlobal);
	t_list * segmentosLibres = list_filter(tablaSegmentosGlobal, segmentoLibre); 
	pthread_mutex_unlock(&mutexTablaSegmentosGlobal);


	//PARA LOS SEGMENTOS OCUPADOS
	fwrite("Segmentos Ocupados \n\n", 21, 1, dump); 
	

	void mostrar(t_segmento * segmento){
		log_info(loggerSegmentacion, "ENtre a mostrar");
		if(segmento->ocupado){
			log_info(loggerSegmentacion, "Base: %d tamanio:%d OCUPADO", segmento->base, segmento->tamanio);
		}
		if(!(segmento->ocupado)){
			log_info(loggerSegmentacion, "Base: %d tamanio:%d LIBRE", segmento->base, segmento->tamanio);
		}
		 
	}

	if(list_any_satisfy(tablaSegmentosGlobal, segmentoOcupado)){
		list_iterate(tablaSegmentosGlobal, mostrar); 
		log_info(loggerSegmentacion, "Entre al dump con segmentos ocupados y la cantidad de ref patotas qe registro es: %d", list_size(listaReferenciasPatotaSegmentacion));
		
		for(int i = 0; i<list_size(listaReferenciasPatotaSegmentacion); i++){
			referenciaTablaPatota * referenciaActual = list_get(listaReferenciasPatotaSegmentacion, i); 
			pthread_mutex_lock(&(referenciaActual->semaforoPatota)); 
			t_list * segmentosOcupados = list_filter(referenciaActual->tablaPatota, segmentoOcupado); 
			pthread_mutex_unlock(&(referenciaActual->semaforoPatota));
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
	pthread_mutex_lock(&(referenciaPatota->semaforoPatota)); 
	t_segmento * segmentoTripulante = list_find(referenciaPatota->tablaPatota, coincideTID); 
	pthread_mutex_unlock(&(referenciaPatota->semaforoPatota));
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
	if(segmento == NULL){
		log_info(loggerSegmentacion, "EL segmneto es null");
	}
	 
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
	fseek(swap, nroFrameSwap*tamanioPagina, SEEK_SET);
	void * ceros = malloc(tamanioPagina);
	memset(ceros, 0, tamanioPagina);
	fwrite(ceros, tamanioPagina, 1, swap);
	//log_info(loggerMiram, "%s", mem_hexstring(memAux, tamanioPagina));
	

	// buscar el frame en la lista de frames swapp y poner que esta libre  --> TODO
	pthread_mutex_lock(&mutexListaFramesSwap);
	t_frame * frameSwap = list_get(listaFramesSwap, nroFrameSwap);
	frameSwap->ocupado = 1;
	//list_replace(listaFramesSwap, frameSwap->inicio / tamanioPagina, frameSwap);
	pthread_mutex_unlock(&mutexListaFramesSwap);
	

	t_frame * frameLibre = buscarFrame();

	//pthread_mutex_lock(&mutexMemoriaPrincipal);
	memcpy(memoriaPrincipal + frameLibre->inicio, memAux, tamanioPagina);
	//log_info(loggerMiram, "\n%s", mem_hexstring(memoriaPrincipal, tamanioMemoria));
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

	log_info(loggerMiram, "Se trae la pagina %d, del proceso %c a MEMORIA", frameLibre->pagina->numeroPagina, idMapa(frameLibre->pagina->pid));
}

void llevarPaginaASwap() {

	t_frame * frameVictima = seleccionarVictima();
	frameVictima->pagina->bitDeValidez = 0;  // Cambia el valor de la pagina real???
	pthread_mutex_lock(&mutexContadorLRU);
	frameVictima->pagina->ultimaReferencia = contadorLRU;
	contadorLRU++;
	pthread_mutex_unlock(&mutexContadorLRU);

	void * memAux = malloc(tamanioPagina);

	//pthread_mutex_lock(&mutexMemoriaPrincipal);
	memcpy(memAux, memoriaPrincipal + frameVictima->inicio, tamanioPagina);
	//pthread_mutex_unlock(&mutexMemoriaPrincipal);

	frameVictima->ocupado = 0;
	// int index = frameVictima->inicio / tamanioPagina;
	// pthread_mutex_lock(&mutexListaFrames);
	// list_replace(listaFrames, index, frameVictima);
	// pthread_mutex_unlock(&mutexListaFrames);
	t_frame * frameSwap = buscarFrameSwap();
	frameSwap->ocupado = 1;
	frameSwap->pagina = frameVictima->pagina;
	frameSwap->pagina->numeroFrame = frameSwap->inicio / tamanioPagina;
	// pthread_mutex_lock(&mutexContadorLRU);
	// frameSwap->pagina->ultimaReferencia = contadorLRU;   ojo aca con este lru
	// contadorLRU++;
	// pthread_mutex_unlock(&mutexContadorLRU);

	

	FILE * swap = fopen(path_SWAP, "a+");
	fseek(swap, frameSwap->inicio, SEEK_SET);
	fwrite(memAux, tamanioPagina, 1, swap);
	fclose(swap);
	free(memAux);
}

t_frame * seleccionarVictima() {

	t_frame * victima = 0;

	if (strcmp(algoritmoReemplazo, "LRU") == 0) {


		t_frame * ultReferenciaLRU(t_frame * unFrame, t_frame * otroFrame) {
			if (!otroFrame->ocupado) {
				return otroFrame;
			}
			if (!unFrame->ocupado) {
				return unFrame;
			}
			if(unFrame->pagina->ultimaReferencia <= otroFrame->pagina->ultimaReferencia){
				return unFrame;
			}else{
				return otroFrame; 
			}
		}

		pthread_mutex_lock(&mutexListaFrames);
		victima = list_get_minimum(listaFrames, ultReferenciaLRU); 
		pthread_mutex_unlock(&mutexListaFrames);
	}

	if (strcmp(algoritmoReemplazo, "CLOCK") == 0) {
		
		pthread_mutex_lock(&mutexListaFrames);
		while(1) {
			t_frame * frame = list_get(listaFrames, punteroClock);
			if(!(frame->pagina->bitDeUso)) {
				punteroClock++;
				checkeoPunteroClock();
				log_info(loggerMiram, "Victima elegida para llevar a SWAP: pagina %d de proceso %c", frame->pagina->numeroPagina, idMapa(frame->pagina->pid));
				pthread_mutex_unlock(&mutexListaFrames);
				return frame;
			}
			if(frame->pagina->bitDeUso) {
				frame->pagina->bitDeUso = 0;
				punteroClock++;
				checkeoPunteroClock();
			}
		}

	}

	log_info(loggerMiram, "Victima elegida para llevar a SWAP: pagina %d de proceso %c", victima->pagina->numeroPagina, idMapa(victima->pagina->pid));
	return victima;
}

void dumpDeMemoriaPaginacion() {
	
	char * fecha = temporal_get_string_time("%d-%m-%y_%H:%M:%S"); 
	char * nombreArchivo = string_from_format("dumpMemoria_%s.dmp", fecha); 

	FILE * dump = fopen(nombreArchivo, "w+"); 
	
	fwrite("\n\n DUMP DE MEMORIA \n\n",21, 1,dump);
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
			estado = "Libre  \0"; 

			pagina = " -\0";
			proceso = " -\0";
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
		//log_info(loggerMiram,"direc prox frame a escribir %d \n", direcProximoFrame);

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
		
		// t_frame * frameOcupado = malloc(sizeof(t_frame));
		// frameOcupado->inicio = direcProximoFrame;
		pthread_mutex_lock(&mutexListaFrames);
		frameLibre->ocupado = 1;
		frameLibre->pagina = pagina; 
		pthread_mutex_unlock(&mutexListaFrames);
		j=0; // NO BOIRRAR
		// pthread_mutex_lock(&mutexListaFrames);
		// t_frame * frameParaLiberar = list_replace(listaFrames, numeroDeFrame, frameOcupado);  // ver si se puede usar replace and destroy para liberar memoria
		// pthread_mutex_unlock(&mutexListaFrames);
		//free(frameParaLiberar);
	}

	referenciaTablaPaginas * tablaDePaginas = malloc(sizeof(referenciaTablaPaginas));
	tablaDePaginas->longitudTareas = longitudTareas;
	tablaDePaginas->pid = pid;
	tablaDePaginas->listaPaginas = listaDePaginas;
	tablaDePaginas->contadorTCB = cantidadTCBs;
	pthread_mutex_init(&(tablaDePaginas->semaforoPatota), NULL);

	pthread_mutex_lock(&mutexListaTablas);
	list_add(listaTablasDePaginas, tablaDePaginas);
	pthread_mutex_unlock(&mutexListaTablas);

	
}

void * obtenerStreamTripulante(referenciaTablaPaginas * referenciaTabla, uint32_t tid){
	
	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}
    pthread_mutex_lock(&mutexListaTripulantes);
	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID); 
	pthread_mutex_unlock(&mutexListaTripulantes);
	
	pthread_mutex_lock(&(referenciaTabla->semaforoPatota));
	uint32_t offset = referenciaTripulante->offset; 
	uint32_t cantPaginas = referenciaTripulante-> cantidadDePaginas; 
	uint32_t pagina = referenciaTripulante ->nroPagina;
	pthread_mutex_unlock(&(referenciaTabla->semaforoPatota));
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
	//mem_hexdump(streamTripulante, SIZEOF_TCB);
	return streamTripulante; 
}

uint32_t obtenerDireccionFrame(referenciaTablaPaginas * referenciaTabla, int nroPagina){

	uint32_t direcFrame = 0; 

	bool coincideNro(t_pagina * pagina){
		return (pagina->numeroPagina == nroPagina);
	}

	pthread_mutex_lock(&(referenciaTabla->semaforoPatota));

	t_pagina * pagina = list_find(referenciaTabla->listaPaginas, coincideNro);
	if(pagina == NULL) {log_info(loggerMiram, "pagina null");}
	if (!(pagina->bitDeValidez)) {
		traerPaginaAMemoria(pagina);
	}
	direcFrame = pagina->numeroFrame * tamanioPagina; 
	pthread_mutex_unlock(&(referenciaTabla->semaforoPatota));

	return direcFrame; 
}

void actualizarEstadoPaginacion(uint32_t tid, uint32_t pid, char estadoNuevo){

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}

	bool coincidePID(referenciaTablaPaginas * referenciaTabla){
		return (referenciaTabla->pid == pid); 
	}
    pthread_mutex_lock(&mutexListaTripulantes);
	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID);
	pthread_mutex_unlock(&mutexListaTripulantes);
	if (referenciaTripulante == NULL) {log_info(loggerSegmentacion, "no existe el tripu"); return;}
	pthread_mutex_lock(&mutexListaTablas);
	referenciaTablaPaginas * referenciaTabla = list_find(listaTablasDePaginas, coincidePID); 
	pthread_mutex_unlock(&mutexListaTablas);
	if (referenciaTabla == NULL) {log_info(loggerSegmentacion, "no existe el tripu"); return;}

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

void actualizarPosicionPaginacion(uint32_t pid, uint32_t tid, uint32_t posx, uint32_t posy) {

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}

	bool coincidePID(referenciaTablaPaginas * referenciaTabla){
		return (referenciaTabla->pid == pid); 
	}
    pthread_mutex_lock(&mutexListaTripulantes);
	t_tripulantePaginacion * referenciaTripulante = list_find(listaTripulantes, coincideID); 
	pthread_mutex_unlock(&mutexListaTripulantes);
	pthread_mutex_lock(&mutexListaTablas);
	referenciaTablaPaginas * referenciaTabla = list_find(listaTablasDePaginas, coincidePID); 
	pthread_mutex_unlock(&mutexListaTablas);
	//t_list * tablaPaginas = referenciaTabla->listaPaginas; 
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

void eliminarTripulantePaginacion(uint32_t pid, uint32_t tid){

	bool coincideTID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}
	bool coincidePID(referenciaTablaPaginas * referenciaTabla){
		return (referenciaTabla->pid == pid); 
	}
	
	referenciaTablaPaginas * referenciaTabla = list_find(listaTablasDePaginas, coincidePID); 

	if (referenciaTabla->contadorTCB == 1) {

		void desocupar(t_pagina * pagina) {
			if (!pagina->bitDeValidez) {
				t_frame * frameADesocupar = list_get(listaFramesSwap, pagina->numeroFrame);
				frameADesocupar->ocupado = 0;
			}
			if (pagina->bitDeValidez) {
				t_frame * frameADesocupar = list_get(listaFrames, pagina->numeroFrame);
				frameADesocupar->ocupado = 0;
				pagina->bitDeValidez = 0;
			}
		}

		pthread_mutex_lock(&(referenciaTabla->semaforoPatota));
		t_list * listaPaginas = referenciaTabla->listaPaginas;
		list_iterate(listaPaginas, desocupar); 
		list_destroy_and_destroy_elements(listaPaginas, free);
		pthread_mutex_unlock(&(referenciaTabla->semaforoPatota));
		free(referenciaTabla);
	}
    pthread_mutex_lock(&mutexListaTripulantes);
	list_remove_and_destroy_by_condition(listaTripulantes, coincideTID, free);
	pthread_mutex_unlock(&mutexListaTripulantes);
	pthread_mutex_lock(&(referenciaTabla->semaforoPatota)); 
	referenciaTabla->contadorTCB--;
	pthread_mutex_unlock(&(referenciaTabla->semaforoPatota));

	dumpDeMemoriaPaginacion();

}

uint32_t obtenerDireccionProximaTareaPaginacion(void * streamTripulante){
	uint32_t direccion = 0; 
	memcpy(&direccion, streamTripulante + sizeof(uint32_t)*3 + sizeof(char), sizeof(uint32_t)); 
	return direccion;
}

char * obtenerProximaTareaPaginacion(referenciaTablaPaginas * referenciaTabla, uint32_t tid) {
	void * streamTripulante = malloc(SIZEOF_TCB);
	streamTripulante = obtenerStreamTripulante(referenciaTabla, tid);
	int direccionProximaTarea = 0;
	direccionProximaTarea = obtenerDireccionProximaTareaPaginacion(streamTripulante);
	if ((direccionProximaTarea) == 0) {
		return "NO_HAY_TAREA";
	}

	bool coincideID(t_tripulantePaginacion * unTripu) {
		return unTripu->tid == tid;
	}
    pthread_mutex_lock(&mutexListaTripulantes);
	t_tripulantePaginacion * referenciaTripu = list_find(listaTripulantes, coincideID);
	pthread_mutex_unlock(&mutexListaTripulantes);

	/// Encontrar los frames de las tareas
	int i = 0, j = 0;
	char c = 0;
	char * tarea = malloc(40);
	int paginaATraer = direccionProximaTarea / tamanioPagina;
	int offsetTareas = direccionProximaTarea % tamanioPagina;
	int direccionFrame = 0;
	

 	
	while (c != '\n' && c != '|') {
		//log_info(loggerMiram, "pagina: %d, offset: %d", paginaATraer, offsetTareas);
		direccionFrame = obtenerDireccionFrame(referenciaTabla, paginaATraer);
		while(c != '\n' && c != '|' && (offsetTareas) < tamanioPagina) {
			memcpy(tarea + i, memoriaPrincipal + direccionFrame + offsetTareas, 1);
			i++;
			offsetTareas++;
			memcpy(&c, memoriaPrincipal + direccionFrame + offsetTareas, 1);
		}

		paginaATraer++;
		offsetTareas = 0;

	}


	if(c == '\n') {
        direccionProximaTarea = i + direccionProximaTarea + 1;
    }

    if(c == '|') {
        direccionProximaTarea = 0;
    }

    //printf("prox tarea: %d", direccionProximaTarea);
    //realloc(tarea, j + 1);
    char barraCero = '\0';
    memcpy(tarea + i, &barraCero, 1);
	
	actualizarPunteroTarea(referenciaTripu, referenciaTabla->listaPaginas, direccionProximaTarea);

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
		t_pagina * pag = list_get(tablaDePaginas, nroPrimerPagina + 1 + z);
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

// Funciones auxiliares Paginacion

void checkeoPunteroClock() {
	int cantidadFrames = tamanioMemoria/tamanioPagina;
	if (punteroClock == cantidadFrames) {punteroClock = 0;}
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

	pthread_mutex_lock(&mutexListaFrames);
    t_frame * frameLibre = list_find(listaFrames, estaLibre);
	pthread_mutex_unlock(&mutexListaFrames);
	//free(frameLibre);

	if(frameLibre == NULL){
		llevarPaginaASwap(); 
		return buscarFrame(); 
	}

	frameLibre->ocupado = 1; 
	return frameLibre;
}

t_frame * buscarFrameSwap() {

	bool estaLibre(t_frame * frame) {
		return frame->ocupado == 0;
	}

	pthread_mutex_lock(&mutexListaFramesSwap);
	t_frame * frameLibre = list_find(listaFramesSwap, estaLibre);
	pthread_mutex_unlock(&mutexListaFramesSwap);
	//free(frameLibre);
	frameLibre->ocupado = 1; 

	return frameLibre;
}

int divisionRedondeadaParaArriba(int x, int y) {
	return (x -1)/y +1;
}

// ------------------------------------------------------ MAPA ----------------------------------------------

void iniciarMapa() {
	sem_init(&semaforoCambiosMapa,0,1);
	sem_init(&semaforoTerminarMapa,0,0);
	nivel_gui_inicializar();
	sem_wait(&semaforoTerminarMapa);
}

void agregarTripulanteAlMapa(uint32_t tid, uint32_t x, uint32_t y) { 
	char id = idMapa(tid); //aca el tid es un uint  
	sem_wait(&semaforoCambiosMapa);
    personaje_crear(navePrincipal, id, x, y);
	nivel_gui_dibujar(navePrincipal);
	sem_post(&semaforoCambiosMapa);
}

void moverTripulanteEnMapa(uint32_t tid, uint32_t x, uint32_t y){
	char id = idMapa(tid);
	sem_wait(&semaforoCambiosMapa);
	item_mover(navePrincipal, id, x, y);
	nivel_gui_dibujar(navePrincipal);
	sem_post(&semaforoCambiosMapa);
}

void expulsarTripulanteDelMapa(uint32_t tid) {
	char id = idMapa(tid);
	sem_wait(&semaforoCambiosMapa);
	item_borrar(navePrincipal, id);
	nivel_gui_dibujar(navePrincipal);
	sem_post(&semaforoCambiosMapa);
}

char idMapa(uint32_t tid){

	char id = 0;

	id = tid + 65;

	if (id > 90) {
		id += 6;
	}
	return id;

}