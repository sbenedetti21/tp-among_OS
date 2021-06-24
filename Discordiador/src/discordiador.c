#include "discordiador.h"
 
// FACU: INICIAR_PATOTA 4 /home/facundin/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|5 2|1 9|2 6|4
// FRAN: INICIAR_PATOTA 2 /home/utnso/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|0
// FRAN: INICIAR_PATOTA 5 /home/utnso/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt

  
int main(int argc, char ** argv){

	loggerDiscordiador = log_create("discordiador.log", "discordiador.c", 0, LOG_LEVEL_INFO); 

	log_info(loggerDiscordiador, "---------PROGRAMA INICIADO---------- ");

	printf("Ingrese un comando o ingrese EXIT para salir del programa \n");

	listaTripulantes = list_create();
	listaReady = list_create();
	listaBloqueados = list_create(); 
	listaTrabajando = list_create();
	tareasDeIO = list_create();
	listaTerminados = list_create();
	listaNuevos = list_create(); 
	listaBloqueadosEmergencia = list_create();


	list_add(tareasDeIO,"GENERAR_OXIGENO");
	list_add(tareasDeIO,"CONSUMIR_OXIGENO");
	list_add(tareasDeIO,"GENERAR_COMIDA");
	list_add(tareasDeIO,"CONSUMIR_COMIDA");
	list_add(tareasDeIO,"GENERAR_BASURA");
	list_add(tareasDeIO,"DESCARTAR_BASURA");

	
	t_config * config = config_create("./cfg/discordiador.config");
	/* 
	pthread_t servidor;
	pthread_create(&servidor, NULL, servidorPrincipal, config);
	*/

	cicloCPU = config_get_int_value(config, "RETARDO_CICLO_CPU");
	tiempoSabotaje = config_get_int_value(config, "DURACION_SABOTAJE");

	sem_init(&semaforoTripulantes, 0,  config_get_int_value(config, "GRADO_MULTITAREA"));
	sem_init(&consultarSiHayVacios, 0,  1);
	sem_init(&consultarSiHayVacios, 0,  2);
	sem_init(&esperarAlgunTripulante, 0,  0);
	sem_init(&IO,0,1);
	sem_init(&cambiarABloqueado,0,1);
	sem_init(&cambiarAFinalizado,0,1); 
	sem_init(&cambiarANuevo,0,1);
	sem_init(&cambiarAReady,0,1);
	sem_init(&cambiarATrabajando,0,1);
	sem_init(&gestionarIO,0,0);
	sem_init(&cambiarABloqueadosEmergencia,0,1);
	sem_init(&mutexPID, 0, 1); 




	 pthread_t hiloConsola;
	 pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	 pthread_join(hiloConsola, NULL);
 
sem_destroy(&semaforoTripulantes);


return 0;
}



//-----------------------------CONECTAR---------------------------------------------------------------------------------------

void servidorPrincipal(t_config * config) {
	char * puerto = config_get_string_value(config, "PUERTO");
	int listeningSocket = crear_conexionServer(puerto);

	int socketCliente;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	pthread_t receptorDiscordiador;


	while(1){
		socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
		if(socketCliente == -1){printf("Error en la conexión"); log_info(loggerDiscordiador, "error en la conexion con IMongoStore");}
		else {
			log_info(loggerDiscordiador, "Conexión establecida con Discordiador");
			pthread_create(&receptorDiscordiador, NULL, atenderImongo, socketCliente);
		}
	}

	close(socketCliente);
	close(listeningSocket);
}

int conectarImongo(){
	t_config * config = config_create("./cfg/discordiador.config");
		char * ip = config_get_string_value(config, "IP_I_MONGO_STORE");
		char * puerto = config_get_string_value(config, "PUERTO_I_MONGO_STORE");

		return crear_conexion(ip, puerto);
}

int conectarMiRAM(){


	t_config * config = config_create("./cfg/discordiador.config");
	char * ip = config_get_string_value(config, "IP_MI_RAM_HQ");
	char * puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");

	 int conexion = crear_conexion(ip, puerto);

	 return conexion; 
}

//-----------------------------CONSOLA---------------------------------------------------------------------------------------

void consola(){

	char * instruccion; 
	char ** vectorInstruccion;


	while(1) {

		instruccion = readline("Ingrese próxima instrucción: \n");

		log_info(loggerDiscordiador, "INSTRUCCION LEIDA: %s", instruccion); 

		vectorInstruccion = string_split(instruccion, " ");


		if(strcmp(vectorInstruccion[0], "INICIAR_PATOTA") == 0) {


			iniciarPatota(vectorInstruccion);

			
		}

		if(strcmp(vectorInstruccion[0], "INICIAR_PLANIFICACION") == 0){
												
				pthread_t  hiloTrabajadorFIFO; 
				pthread_create(&hiloTrabajadorFIFO, NULL, ponerATrabajar, NULL );	

		}

	

		if(strcmp(vectorInstruccion[0], "LISTAR_TRIPULANTES") == 0) {
			
			listarTripulantes();

		}


		
		if(strcmp(vectorInstruccion[0], "EXPULSAR_TRIPULANTE") == 0) {
				
			bool coincideID(TCB_DISCORDIADOR * tripulante){
				return tripulante->tid ==  atoi(vectorInstruccion[1]);
			}

			list_remove_by_condition(listaTripulantes, coincideID); 
			
			//LE FALTAN COSAS 

		}
		

		if(strcmp(vectorInstruccion[0], "EXIT") == 0){
			log_info(loggerDiscordiador, "-------------PROGRAMA TERMINADO-------------"); 
			return 0; 
		}

		if(strcmp(vectorInstruccion[0], "generarOxigeno") == 0){

			tarea_struct * tarea = malloc(sizeof(tarea_struct));
			tarea->parametro = 5;
			tarea->descripcionTarea = "GENERAR_OXIGENO";

			gestionarTarea(tarea, 1);
		}

		if(strcmp(vectorInstruccion[0], "generarBasura") == 0){

			tarea_struct * tarea = malloc(sizeof(tarea_struct));
			tarea->parametro = 5;
			tarea->descripcionTarea = "GENERAR_BASURA";

			gestionarTarea(tarea, 1);
		}

		if(strcmp(vectorInstruccion[0], "consumirOxigeno") == 0){

			tarea_struct * tarea = malloc(sizeof(tarea_struct));
			tarea->parametro = 5;
			tarea->descripcionTarea = "CONSUMIR_OXIGENO";

			gestionarTarea(tarea, 1);
		}
	
		
		
		if(strcmp(vectorInstruccion[0], "PAUSAR_PLANIFICACION") == 0) {

			planificacionPausada = true;

			cambiarEstadoTripulantesA('B');

		}

		if(strcmp(vectorInstruccion[0], "OBTENER_BITACORA") == 0) {
		}

		if(strcmp(vectorInstruccion[0], "tripulanteMasCercano") == 0) {
			TCB_DISCORDIADOR * tripulante = tripulanteMasCercano(3,4);
			printf("El tripulante mas cercano esta en: %d|%d \n", tripulante->posicionX, tripulante->posicionY);
		}

		if(strcmp(vectorInstruccion[0], "a") == 0) {
			TCB_DISCORDIADOR * tripulante = list_get(listaTripulantes,0);

			cambiarDeEstado(tripulante, 'B');

			TCB_DISCORDIADOR * tripulante2 = list_get(listaBloqueados,0);

			printf("El tripulante esta en: %c \n", tripulante2->estado);
		}

		if(strcmp(vectorInstruccion[0], "b") == 0) {
			for(int a = 0 ; a < list_size(listaTerminados) ; a++){
			TCB_DISCORDIADOR * tripulante = list_get(listaTerminados,a);
			printf("El tripulante termino: %d \n", tripulante->tid);
			}
		}

		if(strcmp(vectorInstruccion[0], "mostrarListas") == 0) {
			
			printf("\n");
			printf("Lista NEW: ");
			for(int a = 0 ; a < list_size(listaNuevos) ; a++){
			TCB_DISCORDIADOR * tripulante = list_get(listaTerminados,a);
			printf("%d ", tripulante->tid);
			}
			printf("\n");

			printf("Lista READY: ");
			for(int a = 0 ; a < list_size(listaReady) ; a++){
			TCB_DISCORDIADOR * tripulante = list_get(listaTerminados,a);
			printf("%d ", tripulante->tid);
			}
			printf("\n");

			printf("Lista BLOQUEADOS: ");
			for(int a = 0 ; a < list_size(listaBloqueados) ; a++){
			TCB_DISCORDIADOR * tripulante = list_get(listaTerminados,a);
			printf("%d ", tripulante->tid);
			}
			printf("\n");

			printf("Lista TRABAJANDO: ");
			for(int a = 0 ; a < list_size(listaTrabajando) ; a++){
			TCB_DISCORDIADOR * tripulante = list_get(listaTerminados,a);
			printf("%d ", tripulante->tid);
			}
			printf("\n");

			printf("Lista TERMINADOS: ");
			for(int a = 0 ; a < list_size(listaTerminados) ; a++){
			TCB_DISCORDIADOR * tripulante = list_get(listaTerminados,a);
			printf("%d ", tripulante->tid);
			}
			printf("\n");
			printf("\n");

		}

		if(strcmp(vectorInstruccion[0], "pedirTarea") == 0) {

			tarea_struct * tarea = malloc(sizeof(tarea_struct));
			TCB_DISCORDIADOR * tripulante = list_get(listaTripulantes,0);

			int socket = conectarMiRAM();
				char ** vectorTarea;
				char ** requerimientosTarea; //MALLOC ???

				serializarYMandarPedidoDeTarea(socket, tripulante->pid, tripulante->tid);

				t_paquete* paquete = malloc(sizeof(t_paquete));
				paquete->buffer = malloc(sizeof(t_buffer));

				int headerRECV = recv(socket, &(paquete->header) , sizeof(int), 0);
	
				int statusTamanioBuffer = recv(socket,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

				paquete->buffer->stream = malloc(paquete->buffer->size);

				int BUFFER_RECV = recv(socket,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream

				switch (paquete->header)
				{
				case HAY_TAREA:;

					void* stream = malloc(paquete->buffer->size);
					stream = paquete->buffer->stream;

					int tamanioTareas;
					memcpy(&tamanioTareas, stream, sizeof(int));
					stream += sizeof(int);
					char* stringTarea = malloc(tamanioTareas);
					memcpy(stringTarea, stream, tamanioTareas);
					stream += tamanioTareas;

					vectorTarea = string_split(stringTarea, ";");
					requerimientosTarea = string_split(vectorTarea[0]," "); 

					tarea->descripcionTarea = requerimientosTarea[0];

					if(requerimientosTarea[1] != NULL){
						tarea->parametro = atoi(requerimientosTarea[1]);	//Esto esta rari cuanto menos
					}

					tarea->posicionX = atoi(vectorTarea[1]);			//Llena el struct tarea 
					tarea->posicionY = atoi(vectorTarea[2]);
					tarea->tiempo = atoi(vectorTarea[3]);
					tarea->tareaTerminada = false;

					log_info(loggerDiscordiador,"Nombre tarea: %s \n Posicion: %d|%d \n Duracion: %d \n",tarea->descripcionTarea, tarea->posicionX, tarea->posicionY, tarea->tiempo);
					
					break;
				
				case NO_HAY_TAREA:
									printf("NO RECIBI TAREA");
									break;
				}

		}

		

		//	mostrarLista(listaReady); 

	}

}


//-----------------------------INICIAR PATOTA---------------------------------------------------------------------------------------


void iniciarPatota(char ** vectorInstruccion){

	int socket = conectarMiRAM();
	log_info(loggerDiscordiador, "Discordiador conectado con Mi RAM");

				char * posicionBase = "0|0";
				int i;
				int indice_posiciones = 3;
				int cantidadTripulantes = atoi(vectorInstruccion[1]);
				pthread_t tripulantes[cantidadTripulantes];
				listaTCBsNuevos = list_create();
				sem_wait(&mutexPID); 
				uint32_t idPatota = proximoPID; 
				proximoPID ++; 
				sem_post(&mutexPID);


				for(i = 0; i < cantidadTripulantes; i++ ) {  
					pthread_t hilo;

					TCB_DISCORDIADOR* tripulante = malloc(sizeof(TCB_DISCORDIADOR));
					if (vectorInstruccion[indice_posiciones] != NULL) {
						tripulante = crearTCB(vectorInstruccion[3 + i], idPatota);
						indice_posiciones++;
					} else {
						tripulante = crearTCB(posicionBase,idPatota);
					}

					list_add(listaTCBsNuevos, tripulante);

					pthread_create(&tripulantes[i], NULL, tripulanteVivo , tripulante);
					log_info(loggerDiscordiador, "Tripulante creado: ID: %d, Posicion %d|%d, Estado: %c ", tripulante->tid, tripulante->posicionX, tripulante->posicionY, tripulante->estado ); 
					
					if(!planificacionPausada){
					cambiarDeEstado(tripulante,'R');	
					}

					sem_post(&esperarAlgunTripulante); 

					
				}


				serializarYMandarPCB(vectorInstruccion[2],socket, idPatota, cantidadTripulantes, listaTCBsNuevos);
				

	close(socket);
	
}


TCB_DISCORDIADOR * crearTCB(char * posiciones, uint32_t pid){

		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB_DISCORDIADOR * tripulante = malloc(sizeof(TCB_DISCORDIADOR));

		sem_init(&tripulante->semaforoTrabajo, 0, 0); 
		sem_init(&tripulante->termineIO,0,0);
	
		tripulante->estado = 'N';
		tripulante->tid = proximoTID;
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		tripulante->pid = pid;
		

		sem_wait(&cambiarANuevo);	
		list_add(listaNuevos, tripulante);
		sem_post(&cambiarANuevo);

		list_add(listaTripulantes, tripulante);

		proximoTID ++; 

		return tripulante;  
	} 


void tripulanteVivo(TCB_DISCORDIADOR * tripulante) { 

	bool tareaTerminada = true; 
	bool noHayMasTareas = false;
	tarea_struct * tarea = malloc(sizeof(tarea_struct));

	while (1) 
	{
		sem_wait(&tripulante->semaforoTrabajo);		

		if(tareaTerminada){
				int socket = conectarMiRAM();
				char ** vectorTarea;
				char ** requerimientosTarea; //MALLOC ???

				serializarYMandarPedidoDeTarea(socket, tripulante->pid, tripulante->tid);

				t_paquete* paquete = malloc(sizeof(t_paquete));
				paquete->buffer = malloc(sizeof(t_buffer));

				int headerRECV = recv(socket, &(paquete->header) , sizeof(int), MSG_WAITALL);
				if(!headerRECV) { log_error(loggerDiscordiador, "No se pudo recibir el header al recibir una tarea");}

				
				switch (paquete->header)
				{
				case HAY_TAREA:;

					int statusTamanioBuffer = recv(socket,&(paquete-> buffer-> size), sizeof(uint32_t), MSG_WAITALL);
					if(! statusTamanioBuffer){ log_error(loggerDiscordiador, "No se pudo recibir el tamanio del buffer al recibir una tarea");}

					paquete->buffer->stream = malloc(paquete->buffer->size);

					int BUFFER_RECV = recv(socket,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream
					if(! BUFFER_RECV){ log_error(loggerDiscordiador,"No se pudo recibir el buffer al recibir una tarea");}

					void* stream = malloc(paquete->buffer->size);
					stream = paquete->buffer->stream;

					int tamanioTareas;
					memcpy(&tamanioTareas, stream, sizeof(int));
					stream += sizeof(int);
					char* stringTarea = malloc(tamanioTareas);
					memcpy(stringTarea, stream, tamanioTareas);
					stream += tamanioTareas;

					vectorTarea = string_split(stringTarea, ";");
					requerimientosTarea = string_split(vectorTarea[0]," "); 

					tarea->descripcionTarea = requerimientosTarea[0];

					if(requerimientosTarea[1] != NULL){
						tarea->parametro = atoi(requerimientosTarea[1]);	//Esto esta rari cuanto menos
					}

					tarea->posicionX = atoi(vectorTarea[1]);			//Llena el struct tarea 
					tarea->posicionY = atoi(vectorTarea[2]);
					tarea->tiempo = atoi(vectorTarea[3]);
					tarea->tareaTerminada = false;

					log_info(loggerDiscordiador,"Tarea pedida por tripulante %d: %s Posicion: %d|%d Duracion: %d ",tripulante->tid ,tarea->descripcionTarea, tarea->posicionX, tarea->posicionY, tarea->tiempo);

					tripulante->tareaActual = tarea;
					
					break;
				
				case NO_HAY_TAREA:
									noHayMasTareas = true;
									log_info(loggerDiscordiador,"Tripulante %d termino de trabajar ",tripulante->tid);

									break;
				}
				
				

		}

		if(noHayMasTareas){ break;} //Si MIRAM avisa que no hay mas tareas termina el hilo

		tareaTerminada = tarea->tareaTerminada;

		t_config * config = config_create("./cfg/discordiador.config");
		char * tipoAlgoritmo = config_get_string_value(config, "ALGORITMO");

	

		if(strcmp(tipoAlgoritmo, "FIFO") == 0){

				trasladarseA(tarea->posicionX,tarea->posicionY, tripulante); 

				if(esTareaDeIO(tarea->descripcionTarea)){
					sem_post(&semaforoTripulantes);
					cambiarDeEstado(tripulante,'B');
					sem_post(&gestionarIO);
					sleep((tarea -> tiempo) * cicloCPU + cicloCPU);	
					sem_wait(&tripulante->termineIO);
					sem_post(&esperarAlgunTripulante);
				}

				else{
					sleep((tarea -> tiempo) * cicloCPU);	
					cambiarDeEstado(tripulante,'R');					 
					sem_post(&semaforoTripulantes); 
					sem_post(&esperarAlgunTripulante);
				}

				
				tareaTerminada = true; 
				
				log_info(loggerDiscordiador, "Tripulante %d terminó su tarea", tripulante->tid);

									     } else{
											 	int contador = 0; // cantidad de quantum ya utilizado

												t_config * config = config_create("./cfg/discordiador.config");
												int quantum = atoi(config_get_string_value(config, "QUANTUM"));

												// Primero tiene que ir a la posicion en la que esta la tarea, y para esto gasta
												// quantum. Es por eso que primero se mueve en X lo que pueda, y cuando llega a 
												// su posicion en X, hace lo mismo con Y. Una vez haya llegado a donde se encuentra
												// la tarea la ejecuta poco a poco respetando el quantum
													
													// Moverse en x

												if( tripulante->posicionX != tarea->posicionX){  // Aca se fija que no haya llegado a su posicion en x, si llego ni entra

													for(contador; contador < quantum ;  contador ++){

														sleep(cicloCPU);
														
														if(tarea->posicionX < tripulante->posicionX){
															tripulante->posicionX--;
														} else {
															tripulante->posicionX++;
														}	

														if( tripulante->posicionX == tarea->posicionX){ // si llego sale del for y pasa a Y
															contador++;
															break; 
														}


														// mandar posi a imongo y miram
													}

													if(tripulante->posicionY != tarea->posicionY){
													log_info(loggerDiscordiador, "Tripulante %d termino su Q en %d|%d", tripulante->tid, tripulante->posicionX, tripulante->posicionY);
													}
												}

													// Moverse en y

												if(tripulante->posicionY != tarea->posicionY && contador > 0 ){ // Igual que con x
													
													for(contador; contador < quantum ;  contador ++){
														sleep(cicloCPU);

														if(tarea->posicionY < tripulante->posicionY){
															tripulante->posicionY--;
														} else {
															tripulante->posicionY++;
														}	

														if(tripulante->posicionY == tarea->posicionY){
															contador++;
															break; 
														}

														// TODO mandar posicion a mi ram e imongo
													}

													if(tripulante->posicionY != tarea->posicionY){
														log_info(loggerDiscordiador, "Tripulante %d termino su Q en %d|%d", tripulante->tid, tripulante->posicionX, tripulante->posicionY);
													} else{
														log_info(loggerDiscordiador, "Tripulante %d llego a la posicion de su tarea, le queda %d de quantum", tripulante->tid, quantum - contador);
													}
												

												}

												

												for( contador ; contador < quantum ;  contador ++){ //Si entra aca, es porque ya llego a la psocion y todavia le queda quantum para ejecutar
			
													

													if(esTareaDeIO(tarea->descripcionTarea)){
															tareaTerminada = true;
															tarea->tareaTerminada = true;
															sem_post(&semaforoTripulantes); 
															cambiarDeEstado(tripulante,'B');
															sem_post(&gestionarIO);
															sleep((tarea -> tiempo) * cicloCPU + cicloCPU);											
															sem_wait(&tripulante->termineIO);
															sem_post(&esperarAlgunTripulante);			
															log_info(loggerDiscordiador, "Tripulante %d terminó su tarea", tripulante->tid);
															contador++;
															break;
														}

														else{

															sleep(cicloCPU);
															tarea->tiempo --; // Cada tarea tiene un tiempo, cuando ese tiempo llegue a 0 va a gestionar la tarea en el if de abajo, si no llega a 0 va a seguir haciendo tiempo si su quantum lo permite
															
																
																if(tarea->tiempo == 0){
																	tareaTerminada = true;
																	tarea->tareaTerminada = true; 

																	sleep((tarea -> tiempo) * cicloCPU);	
																	cambiarDeEstado(tripulante,'R');
																	sem_post(&esperarAlgunTripulante);																			 
																	sem_post(&semaforoTripulantes); 
																	log_info(loggerDiscordiador, "Tripulante %d terminó su tarea", tripulante->tid);
																	contador++;
																	break;
																}

															
														}

																log_info(loggerDiscordiador, "Tripulante  %d le faltan %ds para terminar la tarea y le queda %d quantum restante", tripulante->tid, tarea->tiempo, quantum-contador);

													
																	}


												if(!tareaTerminada){
												cambiarDeEstado(tripulante, 'R');
												sem_post(&esperarAlgunTripulante);
												sem_post(&semaforoTripulantes);} 
		
											}

			if(planificacionPausada){
				cambiarDeEstado(tripulante,'B');				
			}


		}

		//sem_post(&esperarAlgunTripulante);
		sem_post(&semaforoTripulantes); 					
		cambiarDeEstado(tripulante,'F');				

}

 
void ponerATrabajar(){

	pthread_t gestionarIO;
	pthread_create(&gestionarIO, NULL, (void*) gestionadorIO, NULL);

	cambiarEstadoTripulantesA('R'); //RARI
	 
	 planificacionPausada = false;
	
	while(1){ 

			if(planificacionPausada){ break; } // esta bien puesto??

			sem_wait(&semaforoTripulantes);

			sem_wait(&esperarAlgunTripulante);

			sem_wait(&cambiarAReady);
			TCB_DISCORDIADOR* tripulantee = list_get(listaReady, 0);
			sem_post(&cambiarAReady);

			cambiarDeEstado(tripulantee,'E');	

			sem_post(&tripulantee->semaforoTrabajo); // donde se pone? -> sem_destroy(&tripulantee->semaforoTrabajo);
				
		 } 
			
			
}

void gestionadorIO(){

	 
	planificacionPausada = false;
	
	while(1){

			if(planificacionPausada){ break; } 

			sem_wait(&gestionarIO);

			sem_wait(&cambiarABloqueado);
			TCB_DISCORDIADOR* tripulantee = list_get(listaBloqueados, 0);
			sem_post(&cambiarABloqueado);

			gestionarTarea(tripulantee->tareaActual,tripulantee->tid);	

			cambiarDeEstado(tripulantee,'R');

			sem_post(&tripulantee->termineIO);
		 } 
			
			
}
//-----------------------------LISTAR TRIPULANTES---------------------------------------------------------------------------------------


void listarTripulantes(){

printf("--------------------------------------------------------- \nEstado actual de la nave: %s    \n\n", temporal_get_string_time("%d/%m/%y %H:%M:%S"));

	for(int i = 0; i < list_size(listaTripulantes) ; i++){

		TCB_DISCORDIADOR *tripulante = list_get(listaTripulantes,i);
		// PCB *patota = tripulante->punteroPCB;

		printf("Tripulante: %d    Patota:    Estado: %c \n", tripulante->tid , /* patota->pid  ,*/  tripulante->estado);

	}

printf("--------------------------------------------------------- \n");

}


void mostrarLista(t_list * unaLista){
		for(int e = 0; e < list_size(unaLista); e++){
					TCB_DISCORDIADOR *tripulante = list_get(unaLista, e);

					printf("index: %d, ID:%d, X:%d, Y:%d \n",e ,tripulante->tid, tripulante->posicionX, tripulante->posicionY);
				}
}

void cambiarEstadoTripulantesA(char estado){
	for(int i = 0 ; i < list_size(listaTripulantes) ; i ++){
				TCB_DISCORDIADOR * tripulante = list_get(listaTripulantes,i); 

				if(tripulante->estado != 'F'){
					
					cambiarDeEstado(tripulante, estado);

				}

			}
}



//-----------------------------TAREAS---------------------------------------------------------------------------------------------------


void trasladarseA(uint32_t posicionX,uint32_t posicionY, TCB_DISCORDIADOR * tripulante){

	while(posicionX != tripulante->posicionX)
	{
		if(posicionX < tripulante->posicionX){
			tripulante->posicionX--;
		} else {
			tripulante->posicionX++;
		}

		sleep(cicloCPU);
		// MANDAR POSICION A MI RAM e IMONGO
	}
	
	while(posicionY != tripulante->posicionY)
	{
		if(posicionY < tripulante->posicionY){
			tripulante->posicionY--;
		} else {
			tripulante->posicionY++;
		}

		sleep(cicloCPU);
		// MANDAR POSICION A MI RAM e IMONGO
	}
	
	log_info(loggerDiscordiador, "Tripulante %d ahora esta en %d|%d ",tripulante->tid,tripulante->posicionX,tripulante->posicionY);

}

void gestionarTarea(tarea_struct * tarea, uint32_t tid){
	char * descripcionTarea = tarea->descripcionTarea;
	int parametros = tarea->parametro;
				if( strcmp(descripcionTarea,"GENERAR_OXIGENO") == 0 ){
						serializarYMandarInicioTareaIO(parametros, GENERAR_OXIGENO,tid);
					} 

					else if(strcmp(tarea->descripcionTarea,"CONSUMIR_OXIGENO") == 0){
						serializarYMandarInicioTareaIO(parametros, CONSUMIR_OXIGENO,tid);
					}

					else if(strcmp(descripcionTarea,"GENERAR_COMIDA") == 0){
						serializarYMandarInicioTareaIO(parametros, GENERAR_COMIDA,tid);
					}

					else if(strcmp(descripcionTarea,"CONSUMIR_COMIDA") == 0){
						serializarYMandarInicioTareaIO(parametros, CONSUMIR_COMIDA,tid);
					}

					else if(strcmp(descripcionTarea,"GENERAR_BASURA") == 0){
						serializarYMandarInicioTareaIO(parametros, GENERAR_BASURA,tid);
					}

					else if(strcmp(descripcionTarea,"DESCARTAR_BASURA") == 0){
						serializarYMandarInicioTareaIO(parametros, DESCARTAR_BASURA,tid);
					}
					else{
					}
					

}


char * leerTareas(char* pathTareas) {
	//TODO
	FILE* archivo = fopen(pathTareas,"r");
	if (archivo == NULL)
	{
		printf("no pude abrir las tareas :( \n");
	}
	else
	{
		log_info(loggerDiscordiador, "path de tareas recibido: %s", pathTareas);
	}
	
	
    fseek(archivo, 0, SEEK_END);
    int tamanioArchivo = ftell(archivo);
	printf("cantidad de carac:%d \n", tamanioArchivo);
    fseek(archivo, 0, SEEK_SET);

    char* lineas = (char*) calloc(tamanioArchivo, sizeof(char)); //USANDO MALLOC ME TIRABA ERROR
    fread(lineas, sizeof(char), tamanioArchivo, archivo);

    // acá tengo un string "lineas" con todas las lineas del archivo juntas, incluyendo los saltos de linea

	fclose(archivo);

	printf("%s \n", lineas);

	return lineas;
}

bool esTareaDeIO(char * tarea){
	
	for(int a = 0 ; a < list_size(tareasDeIO) ; a++){
		if( strcmp( tarea , list_get(tareasDeIO, a)) == 0){
			return true;
		}

		
	}
	return false;
}


//-----------------------------PAQUETES---------------------------------------------------------------------------------------------------

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

	memcpy(a_enviar + offset2, paquete-> buffer-> stream, paquete->buffer->size);

	send(socket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);

}


void serializarYMandarPCB(char * pathTareas, int socket, uint32_t pid, int cantidadTCB, t_list * listaTCBS){

	int  offset = 0;
	
	char * tareas = leerTareas(pathTareas);
	int  tamanioTareas = strlen(tareas);

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = sizeof(tamanioTareas) + tamanioTareas + sizeof(cantidadTCB) + sizeof(TCB) * cantidadTCB + sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream+offset, &tamanioTareas, sizeof(tamanioTareas));
	offset += sizeof(int);

	memcpy(stream+offset, tareas, tamanioTareas);
	offset += tamanioTareas;

	memcpy(stream + offset, &pid, sizeof(uint32_t)); 
	offset += sizeof(uint32_t); 
	
	memcpy(stream+offset, & cantidadTCB, sizeof(cantidadTCB));
	offset += sizeof(cantidadTCB);

	for(int i = 0;i<cantidadTCB; i++){

		TCB_DISCORDIADOR * tripulante = malloc(sizeof(tripulante));
		tripulante = list_get(listaTCBS,i);
		
			memcpy(stream+offset, &(tripulante->tid), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy(stream+offset, &(tripulante->posicionX), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy(stream+offset, &(tripulante->posicionY), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy(stream+offset, &(tripulante->estado), sizeof(char));
			offset += sizeof(char);

	}

	buffer->stream = stream;

	mandarPaqueteSerializado(buffer, socket, INICIAR_PATOTA);

	list_destroy(listaTCBS);
}

void serializarYMandarInicioTareaIO(int parametro, int tipoTarea, uint32_t tid ){
	int socket = conectarImongo();

	t_parametro * parametroS = malloc(sizeof(parametroS));
	parametroS->parametro = parametro;
	parametroS->tid = tid;

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = sizeof(int) + sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &(parametroS->parametro), sizeof(int));
	offset += sizeof(int);

	memcpy(stream+offset, &(parametroS->tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, tipoTarea);

}

void serializarYMandarPedidoDeTarea(int socket, uint32_t pid, uint32_t tid){

	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer-> size = sizeof(uint32_t) * 2;

	void* stream = malloc(buffer->size);

	int offset = 0;

	log_info(loggerDiscordiador ,"DATOS TRIPU QUE PIDIO TAREA, PID: %d, TID: %d", pid, tid);

	memcpy(stream + offset, &tid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	memcpy(stream + offset, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, PEDIR_TAREA);
}

void serializarYMandarPosicion(TCB_DISCORDIADOR * tripulante){

	int socketIMONGO = conectarImongo();
	int socketMIRAM = conectarMiRAM();

	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer-> size = sizeof(uint32_t) * 2;

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &(tripulante->posicionX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->posicionY), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socketIMONGO, NUEVA_POSICION);
	mandarPaqueteSerializado(buffer, socketMIRAM, PEDIR_TAREA);  // VER HEADER

}

void serializarYMandarInicioTareaNormal(uint32_t tid, char * stringTareas){
	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));

	int tamanioTarea = strlen(stringTareas) + 1;

	buffer-> size = sizeof(int) + sizeof(uint32_t) + tamanioTarea;

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tamanioTarea), sizeof(int));
	offset += sizeof(int);

	memcpy(stream+offset, stringTareas, tamanioTarea);
	offset += tamanioTarea;

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, INICIO_TAREA_NORMAL);
}

void serializarYMandarFinalizacionTarea(uint32_t tid){
	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, FINALIZO_TAREA_NORMAL);
}

void serializarYMandarElegidoDelSabotaje(uint32_t tid){
	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, INICIO_SABOTAJE);
}

//-----------------------------SABOTAJES---------------------------------------------------------------------------------------------------

void atenderImongo(int socketCliente){
 
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);
	if(headerRECV) { log_info(loggerDiscordiador, "Recibi header: %d\n", paquete->header);} else{ log_error(loggerDiscordiador, "No se pudo recibir el header");}
	
	int BUFFER_RECV  = 0;
	
	int statusTamanioBuffer = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);
	if(! statusTamanioBuffer){ log_error(loggerDiscordiador, "No se pudo recibir el tamanio del buffer ");}

	paquete->buffer->stream = malloc(paquete->buffer->size);

	BUFFER_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream

	if(! BUFFER_RECV){ log_error(loggerDiscordiador,"No se pudo recibir el buffer");}

	switch (paquete->header)
	{
	case ALERTA_DE_SABOTAJE: ;

		void* stream = malloc(paquete->buffer->size);
		stream = paquete->buffer->stream;

		uint32_t posX;
		uint32_t posY;

		int tamanioTareas;
		memcpy(&posX, stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);

		memcpy(&posY, stream, tamanioTareas);

		TCB_DISCORDIADOR * tripulante = tripulanteMasCercano(posX, posY);

		// PASAR A ESTADOS CORRESPONDIENTES

		trasladarseA(posX, posY, tripulante);

		cambiarDeEstado(tripulante, 'S');

		sleep(tiempoSabotaje);

		serializarYMandarElegidoDelSabotaje(tripulante->tid);
		
		break;
	
	case SABOTAJE_TERMINADO:;

		// recv() señal de que termino fsck

		// PASAR A ESTADOS CORRESPONDIENTES

		break;
	}

}

TCB_DISCORDIADOR * tripulanteMasCercano(uint32_t posX, uint32_t posY){

	uint32_t distanciaMenor;
	uint32_t tidMasCercano;

	for(int a = 0 ; a < list_size(listaTripulantes) ; a++){

		TCB_DISCORDIADOR * tripulante = list_get(listaTripulantes,a); 

		uint32_t * distanciaASabotaje = abs(posX - tripulante->posicionX) + abs(posY - tripulante->posicionY);

		if(a == 0){
			distanciaMenor = distanciaASabotaje; //Al primero lo asigna asi
		}

		if(distanciaASabotaje < distanciaMenor){
			distanciaMenor = distanciaASabotaje;
			tidMasCercano = tripulante->tid;
		}

	
	}

	bool coincideID(TCB_DISCORDIADOR * tripulante){
				return tripulante->tid ==  tidMasCercano;
			}

	return list_find(listaTripulantes, coincideID); 


}

//-----------------------------ESTADOS---------------------------------------------------------------------------------------------------

void cambiarDeEstado(TCB_DISCORDIADOR * tripulante, char estado){

	salirDeListaEstado(tripulante);

	tripulante->estado = estado;

	switch (estado)
	{
	case 'E': ;
		
		sem_wait(&cambiarATrabajando);
		list_add(listaTrabajando, tripulante);
		sem_post(&cambiarATrabajando);

		break;
	
	case 'B': ;
		
		sem_wait(&cambiarABloqueado);
		list_add(listaBloqueados, tripulante);
		sem_post(&cambiarABloqueado);

		break;

	case 'R': ;

		sem_wait(&cambiarAReady);
		list_add(listaReady, tripulante);
		sem_post(&cambiarAReady);

		break;

		case 'F': ;
		
		sem_wait(&cambiarAFinalizado);		
		list_add(listaTerminados, tripulante);
		sem_post(&cambiarAFinalizado);

		break;

		case 'N': ;
		
		sem_wait(&cambiarANuevo);		
		list_add(listaNuevos, tripulante);
		sem_post(&cambiarANuevo);

		break;

		case 'S': ;
		
		sem_wait(&cambiarABloqueadosEmergencia);		
		list_add(listaBloqueadosEmergencia, tripulante);
		sem_post(&cambiarABloqueadosEmergencia);

		break;

	default:
		break;
	}

		log_info(loggerDiscordiador, "Tripulante %d cambio su estado a %c", tripulante->tid, tripulante->estado);


}

void salirDeListaEstado(TCB_DISCORDIADOR * tripulante){

	bool coincideID(TCB_DISCORDIADOR * tripulantee){
				return tripulantee->tid == tripulante->tid;
			}

	switch (tripulante->estado)
	{
	case 'E': ;
		
		sem_wait(&cambiarATrabajando);	
		list_remove_by_condition(listaTrabajando, coincideID); 
		sem_post(&cambiarATrabajando);

		break;
	
	case 'B': ;
		
		sem_wait(&cambiarABloqueado);	
		list_remove_by_condition(listaBloqueados, coincideID); 
		sem_post(&cambiarABloqueado);

		break;

	case 'R': ;

		sem_wait(&cambiarAReady);	
		list_remove_by_condition(listaReady, coincideID); 
		sem_post(&cambiarAReady);

		break;

	case 'F': ;
		
		sem_wait(&cambiarAFinalizado);	
		list_remove_by_condition(listaTerminados, coincideID); 
		sem_post(&cambiarAFinalizado);

		break;

	case 'N': ;
		
		sem_wait(&cambiarANuevo);	
		list_remove_by_condition(listaNuevos, coincideID); 
		sem_post(&cambiarANuevo);

		break;

	case 'S': ;

		sem_wait(&cambiarABloqueadosEmergencia);
		list_remove_by_condition(listaBloqueadosEmergencia, coincideID); 
		sem_post(&cambiarABloqueadosEmergencia);

		break;

	default:
		break; // Se puede sacar?????
	}

}