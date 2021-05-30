#include "discordiador.h"




int main(int argc, char ** argv){

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

	return crear_conexion(ip, puerto);


}

//-----------------------------CONSOLA---------------------------------------------------------------------------------------

void consola(){

	char * instruccion;
	char ** vectorInstruccion;


	while(1) {

		instruccion = readline("Ingrese próxima instrucción: \n");

		vectorInstruccion = string_split(instruccion, " ");


		if(strcmp(vectorInstruccion[0], "INICIAR_PATOTA") == 0) {


			iniciarPatota(vectorInstruccion);

			
		}

		if(strcmp(vectorInstruccion[0], "trabajar") == 0){
													
			t_config * config = config_create("./cfg/discordiador.config");
			char * tipoAlgoritmo = config_get_string_value(config, "ALGORITMO");

			if(strcmp(tipoAlgoritmo,"FIFO") == 0){
				pthread_t  hiloTrabajadorFIFO; 
				pthread_create(&hiloTrabajadorFIFO, NULL, trabajar, NULL );	
			} 
			if (strcmp(tipoAlgoritmo, "RR") == 0){
				//PLANIFICACION RR 
			}

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
	
		
		/*
		if(strcmp(vectorInstruccion[0], "PAUSAR_PLANIFICACION") == 0) {
		}
		if(strcmp(vectorInstruccion[0], "OBTENER_BITACORA") == 0) {
		}

		*/

		

			mostrarLista(listaReady); 

	}

}


//-----------------------------INICIAR PATOTA---------------------------------------------------------------------------------------

void iniciarPatota(char ** vectorInstruccion){

	int socket = conectarMiRAM();

	uint32_t punteroPCB = iniciarPCB(vectorInstruccion[2], socket);

	//INICIAR_PATOTA 3 txt 1|1 1|2 1|3
				char * posicionBase = "0|0";
				int i;
				int indice_posiciones = 3;
				int cantidadTripulantes = atoi(vectorInstruccion[1]);
				pthread_t tripulantes[cantidadTripulantes];

				for(i = 0; i < cantidadTripulantes; i++ ) {  // DEBERIA SER <, NO <=
					pthread_t hilo;

					TCB_DISCORDIADOR* tripulante = malloc(sizeof(TCB_DISCORDIADOR));
					if (vectorInstruccion[indice_posiciones] != NULL) {
						tripulante = crearTCB(vectorInstruccion[3 + i],punteroPCB);
						indice_posiciones++;
					} else {
						tripulante = crearTCB(posicionBase, punteroPCB);
					}

					pthread_create(&tripulantes[i], NULL, tripulanteVivo , tripulante);
					listarTripulantes(); 
					tripulante->estado = 'R'; 
					list_add(listaReady, tripulante);
					
				}

	close(socket);

}


uint32_t iniciarPCB(char * pathTareas, int socket){

	char * stringTareas = leerTareas(pathTareas);

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = strlen(stringTareas) + 1;

	void* stream = malloc(buffer->size);

	int offset = 0;

	// TCB_DISCORDIADOR * a = malloc(sizeof(a));
	// a->posicionX = 4;

	// memcpy(stream+offset, &(a->posicionX), sizeof(uint32_t));
	// offset += sizeof(uint32_t);
	
	memcpy(stream + offset, stringTareas, buffer->size);

	buffer-> stream = stream;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	 paquete->buffer = malloc(sizeof(buffer->size));

	paquete->header = CREAR_PCB;
	paquete->buffer = buffer;
	
	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t) ); //PUSE INT EN VEZ DE UINT_8 PQ NUESTRO HEADER ES UN INT
	int offset2 = 0;

	memcpy(a_enviar + offset2, &(paquete->header), sizeof(int));
	offset2 += sizeof(int);

	memcpy(a_enviar + offset2, &(paquete->buffer->size), sizeof(uint32_t));
	offset2 += sizeof(uint32_t);

	memcpy(a_enviar + offset2, paquete-> buffer-> stream, paquete->buffer->size);
	
	send(socket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);
	 
	//  free(a_enviar);
	//  free(paquete->buffer->size);
	//  free(paquete->buffer); DA ERROR
	//  free(paquete);
	//  free(a);

	uint32_t * punteroPCB = malloc(sizeof(uint32_t));
	int prueba = recv(socket, (void*)punteroPCB, sizeof(uint32_t),0);

	return *punteroPCB;
}

char * leerTareas(char* pathTareas) {
	//TODO
	FILE* archivo = fopen(pathTareas,"r");
	if (archivo == NULL)
	{
		printf("no pude abrir las tareas :( \n");
	}
	
    fseek(archivo, 0, SEEK_END);
    int tamanioArchivo = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    char* lineas = malloc(tamanioArchivo + 1);
    fread(lineas, 1, tamanioArchivo, archivo);

    // acá tengo un string "lineas" con todas las lineas del archivo juntas, incluyendo los saltos de linea

	fclose(archivo);

	return lineas;
}


TCB_DISCORDIADOR * crearTCB(char * posiciones, uint32_t punteroAPCB){

		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB_DISCORDIADOR * tripulante = malloc(sizeof(TCB_DISCORDIADOR));

		sem_init(&tripulante->semaforoTrabajo, 0, 0); 
	
		tripulante->estado = 'N';
		tripulante->tid = proximoTID;
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		tripulante->punteroPCB = punteroAPCB;  
		//tripulante->proximaInstruccion; //falta

		list_add(listaTripulantes, tripulante);

		proximoTID ++; //ver sincronizacion

		return tripulante;   //preguntar liberar malloc
	}



void tripulanteVivo(TCB_DISCORDIADOR * tripulante) { 

	int socket = conectarMiRAM();

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = sizeof(uint32_t) * 4  + sizeof(char);

	void* stream = malloc(buffer->size);

	int offset = 0; //desplazamiento

	memcpy(stream+offset, &(tripulante->tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->posicionX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->posicionY), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->punteroPCB), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->estado), sizeof(char));

	buffer-> stream = stream;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	 paquete->buffer = malloc(sizeof(buffer->size));

	paquete->header = CREAR_TCB;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t) ); //PUSE INT EN VEZ DE UINT_8 PQ NUESTRO HEADER ES UN INT
	int offset2 = 0;

	memcpy(a_enviar + offset2, &(paquete->header), sizeof(int));
	offset2 += sizeof(int);

	memcpy(a_enviar + offset2, &(paquete->buffer->size), sizeof(uint32_t));
	offset2 += sizeof(uint32_t);

	memcpy(a_enviar + offset2, paquete-> buffer-> stream, paquete->buffer->size);

	send(socket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);

	//  free(a_enviar);
	//  free(paquete->buffer->size);
	//  free(paquete->buffer); 
	//  free(paquete);


	while (1) 
	{
		sem_wait(&tripulante->semaforoTrabajo);
					 		
		printf("estoy trabajando soy: %d \n", tripulante->tid);
		sleep(5);
		sem_post(&semaforoTripulantes);
		tripulante->estado = 'R';
		list_add(listaReady, tripulante); 
								
					
	}


}


void trabajar(){
	 
	
			while(1){  

				sem_wait(&semaforoTripulantes); 
				
				TCB_DISCORDIADOR* tripulantee = list_remove(listaReady, 0);

				tripulantee->estado = 'E';
				
				sem_post(&tripulantee->semaforoTrabajo); // donde se pone? -> sem_destroy(&tripulantee->semaforoTrabajo);
				
				printf("------------------ \n "); 

				//mostrarLista(listaReady); 

			
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



