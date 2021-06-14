#include "discordiador.h"

// FACU: INICIAR_PATOTA 4 /home/facundin/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|0


int main(int argc, char ** argv){

	 loggerDiscordiador = log_create("discordiador.log", "discordiador.c", 0, LOG_LEVEL_INFO); 

	log_info(loggerDiscordiador, "---------PROGRAMA INICIADO----------");

	printf("Ingrese un comando o ingrese EXIT para salir del programa \n");

	listaTripulantes = list_create();
	listaReady = list_create();
	listaBloqueados = list_create();


	t_config * configuracion = config_create("./cfg/discordiador.config");
	sem_init(&semaforoTripulantes, 0,  config_get_int_value(configuracion, "GRADO_MULTITAREA"));

	 pthread_t hiloConsola;
	 pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	 pthread_join(hiloConsola, NULL);
 
sem_destroy(&semaforoTripulantes);


return 0;
}

//-----------------------------CONECTAR---------------------------------------------------------------------------------------

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

		if(strcmp(vectorInstruccion[0], "trabajar") == 0){
												
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
		
		if(strcmp(vectorInstruccion[0], "INICIAR_PLANIFICACION") == 0) {
		

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
	
		
		/*
		if(strcmp(vectorInstruccion[0], "PAUSAR_PLANIFICACION") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "OBTENER_BITACORA") == 0) {
		}

		*/

		

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
				t_list * listaTCBs;
				listaTCBs = list_create();


				for(i = 0; i < cantidadTripulantes; i++ ) {  
					pthread_t hilo;

					TCB_DISCORDIADOR* tripulante = malloc(sizeof(TCB_DISCORDIADOR));
					if (vectorInstruccion[indice_posiciones] != NULL) {
						tripulante = crearTCB(vectorInstruccion[3 + i]);
						indice_posiciones++;
					} else {
						tripulante = crearTCB(posicionBase);
					}

					list_add(listaTCBs, tripulante);

					pthread_create(&tripulantes[i], NULL, tripulanteVivo , tripulante);
					log_info(loggerDiscordiador, "Tripulante creado: ID: %d, PosX: %d, PosY: %d, estado: %c ", tripulante->tid, tripulante->posicionX, tripulante->posicionY, tripulante->estado ); 
					tripulante->estado = 'R'; 
					log_info(loggerDiscordiador, "Estado tripulante %d cambiado a %c", tripulante->tid, tripulante->estado);
					list_add(listaReady, tripulante); 
					
				}


				serializarYMandarPCB(vectorInstruccion[2],socket, cantidadTripulantes, listaTCBs);
				

	close(socket);
	
}


TCB_DISCORDIADOR * crearTCB(char * posiciones){

		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB_DISCORDIADOR * tripulante = malloc(sizeof(TCB_DISCORDIADOR));

		sem_init(&tripulante->semaforoTrabajo, 0, 0); 
	
		tripulante->estado = 'N';
		tripulante->tid = proximoTID;
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		//tripulante->proximaInstruccion; //falta

		list_add(listaTripulantes, tripulante);

		proximoTID ++; //ver sincronizacion

		return tripulante;   //preguntar liberar malloc
	} 


void tripulanteVivo(TCB_DISCORDIADOR * tripulante) { 

	//  free(a_enviar);
	//  free(paquete->buffer->size);
	//  free(paquete->buffer); 
	//  free(paquete);

	bool tareaTerminada = false; // PONER EN TRUE CUANDOR RECIBA TAREAS
	tarea_struct * tarea = malloc(sizeof(tarea_struct));


	while (1) 
	{
		sem_wait(&tripulante->semaforoTrabajo);
		log_info(loggerDiscordiador, "Tripulante %d trabajando, estado: %c", tripulante->tid, tripulante->estado);
		

		if(tareaTerminada){
				int socket = conectarMiRAM();
				char ** vectorTarea;

				// send() hay que pedir la tarea para despues recibirla
				// recv(socket, vectorTarea, sizeof(strlen(vectorTarea)), 0);
				
				char ** requerimientosTarea;

				if(string_is_empty(vectorTarea)){				// Se fija que la tarea no este vacía

				vectorTarea = string_split(tarea, " ");
				requerimientosTarea = string_split(vectorTarea[1],";");
				tarea->descripcionTarea = vectorTarea[0];
				tarea->parametro = atoi(requerimientosTarea[0]);			//Llena el struct tarea 
				tarea->posicionX = atoi(requerimientosTarea[1]);
				tarea->posicionY = atoi(requerimientosTarea[2]);
				tarea->tiempo = atoi(requerimientosTarea[3]);
				tarea->tareaTerminada = false;

				} else {
					break;					//Si la tarea esta vacía entonces procedera a salir del while para terminar con el hilo.
				}

		}

		tareaTerminada = tarea->tareaTerminada;

		t_config * config = config_create("./cfg/discordiador.config");
		char * tipoAlgoritmo = config_get_string_value(config, "ALGORITMO");

		tarea->parametro =12; // Esto es solo de prueba
		tarea->posicionX = 2;
		tarea->posicionY = 3;
		tarea->tiempo = 5;
		tarea->tareaTerminada = false;
		tarea->descripcionTarea = "GENERAR_OXIGENO";

	

		if(strcmp(tipoAlgoritmo, "FIFO") == 0){
			
				trasladarseA(tarea->posicionX,tarea->posicionY, tripulante); 
				sleep(tarea -> tiempo);	
				gestionarTarea(tarea,tripulante->tid);	// ver si  cuando termina el tiempo recien ejecutar la tarea
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
														sleep(1);
														tripulante->posicionX ++;

														if( tripulante->posicionX == tarea->posicionX){ // si llego sale del for y pasa a Y
															break; 
														}
													}

												}

													// Moverse en y

												if(tripulante->posicionY != tarea->posicionY){ // Igual que con x
													
													for(contador; contador < quantum ;  contador ++){
														sleep(1);
														tripulante->posicionY++;

														if(tripulante->posicionY == tarea->posicionY){
															break; 
														}

														// TODO mandar posicion a mi ram
													}

												}

												

												for( contador ; contador < quantum ;  contador ++){ //Si entra aca, es porque ya llego a la psocion y todavia le queda quantum para ejecutar

													sleep(1);
													tarea->tiempo --; // Cada tarea tiene un tiempo, cuando ese tiempo llegue a 0 va a gestionar la tarea en el if de abajo, si no llega a 0 va a seguir haciendo tiempo si su quantum lo permite

													if(tarea->tiempo == 0){
															tarea->tareaTerminada = true;
															gestionarTarea(tarea,tripulante->tid);
															log_info(loggerDiscordiador, "Tripulante %d terminó su tarea", tripulante->tid);
															break; 
															}
													   																	
																						}

		
											}

		
		
		sem_post(&semaforoTripulantes); 
		tripulante->estado = 'R';
		list_add(listaReady, tripulante); 

		}
															
		tripulante->estado = 'F';
		log_info(loggerDiscordiador, "Tripulante %d terminó de trabajar, estado: %c", tripulante->tid, tripulante->estado);

}


void ponerATrabajar(){
	 
	
	while(1){  

			sem_wait(&semaforoTripulantes); 
				
			TCB_DISCORDIADOR* tripulantee = list_remove(listaReady, 0);

			tripulantee->estado = 'E';

			sem_post(&tripulantee->semaforoTrabajo); // donde se pone? -> sem_destroy(&tripulantee->semaforoTrabajo);
				
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



//-----------------------------TAREAS---------------------------------------------------------------------------------------------------


void trasladarseA(int posicionX,int posicionY, TCB_DISCORDIADOR * tripulante){

	for (tripulante->posicionX; tripulante->posicionX < posicionX; tripulante->posicionX ++) 
	{
		sleep(1);
		// MANDAR POSICION A MI RAM
	}
	
	for(tripulante->posicionY; tripulante->posicionY < posicionY; tripulante->posicionY ++){
		sleep(1);
		// MANDAR POSICION A MI RAM

	}

	printf("Soy el tripulante %d y ahora estoy en X: %d    Y: %d \n",tripulante->tid,tripulante->posicionX,tripulante->posicionY);
}

void gestionarTarea(tarea_struct * tarea, uint32_t tid){
	char * descripcionTarea = tarea->descripcionTarea;
	int parametros = tarea->parametro;
				if( strcmp(descripcionTarea,"GENERAR_OXIGENO") == 0 ){
						serializarYMandarTarea(parametros, GENERAR_OXIGENO,tid);
					} 

					else if(strcmp(tarea->descripcionTarea,"CONSUMIR_OXIGENO") == 0){
						serializarYMandarTarea(parametros, CONSUMIR_OXIGENO,tid);
					}

					else if(strcmp(descripcionTarea,"GENERAR_COMIDA") == 0){
						serializarYMandarTarea(parametros, GENERAR_COMIDA,tid);
					}

					else if(strcmp(descripcionTarea,"CONSUMIR_COMIDA") == 0){
						serializarYMandarTarea(parametros, CONSUMIR_COMIDA,tid);
					}

					else if(strcmp(descripcionTarea,"GENERAR_BASURA") == 0){
						serializarYMandarTarea(parametros, GENERAR_BASURA,tid);
					}

					else if(strcmp(descripcionTarea,"DESCARTAR_BASURA") == 0){
						serializarYMandarTarea(parametros, DESCARTAR_BASURA,tid);
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


void serializarYMandarPCB(char * pathTareas, int socket, int cantidadTCB, t_list * listaTCBS){

	int  offset = 0;
	
	char * tareas = leerTareas(pathTareas);
	int  tamanioTareas = strlen(tareas);

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = sizeof(tamanioTareas) + tamanioTareas + sizeof(cantidadTCB) + sizeof(TCB) * cantidadTCB;

	void* stream = malloc(buffer->size);

	memcpy(stream+offset, &tamanioTareas, sizeof(tamanioTareas));
	offset += sizeof(int);

	memcpy(stream+offset, tareas, tamanioTareas);
	offset += tamanioTareas;
	
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

void serializarYMandarTarea(int parametro, tareasTripulantes tipoTarea, uint32_t tid ){
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