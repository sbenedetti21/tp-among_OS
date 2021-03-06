#include "discordiador.h"  
  
// FACU: INICIAR_PATOTA 4 /home/facundin/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|5 2|1 9|2 6|4
// FACU: INICIAR_PATOTA 5 /home/facundin/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|5 2|1 9|2 6|4
// FRAN: INICIAR_PATOTA 2 /home/utnso/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|0
// FRAN: INICIAR_PATOTA 5 /home/utnso/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/tareas.txt 0|5 2|1 9|2 6|4
// BENE: INICIAR_PATOTA 1 /home/utnso/TPCUATRI/tp-2021-1c-Pascusa/Discordiador/PAG_PatotaA.txt 1|1
/*  
DIFI:    
INICIAR_PATOTA 3 ES3_Patota1.txt 9|9 0|0 5|5
INICIAR_PATOTA 3 ES3_Patota2.txt 4|0 2|6 8|2
INICIAR_PATOTA 3 ES3_Patota3.txt 2|3 5|8 5|3
INICIAR_PATOTA 3 ES3_Patota4.txt 0|9 4|4 9|0
INICIAR_PATOTA 3 ES3_Patota5.txt 0|2 9|6 3|5
 
INICIAR_PATOTA 3 /home/utnso/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota1.txt 9|9 0|0 5|5
INICIAR_PATOTA 3 /home/utnso/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota2.txt 4|0 2|6 8|2
INICIAR_PATOTA 3 /home/utnso/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota3.txt 2|3 5|8 5|3
INICIAR_PATOTA 3 /home/utnso/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota4.txt 0|9 4|4 9|0
INICIAR_PATOTA 3 /home/utnso/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota5.txt 0|2 9|6 3|5
 
INICIAR_PATOTA 3 /home/facundin/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota1.txt 9|9 0|0 5|5
INICIAR_PATOTA 3 /home/facundin/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota2.txt 4|0 2|6 8|2
INICIAR_PATOTA 3 /home/facundin/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota3.txt 2|3 5|8 5|3
INICIAR_PATOTA 3 /home/facundin/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota4.txt 0|9 4|4 9|0
INICIAR_PATOTA 3 /home/facundin/TPCUATRI/a-mongos-pruebas/Finales/ES3_Patota5.txt 0|2 9|6 3|5
 
//FILESISTEM
INICIAR_PATOTA 3 FS_PatotaA.txt
INICIAR_PATOTA 3 FS_PatotaB.txt
 
60 OXIGENO
60 COMIDA
60 BASURA

855

//SABOTAJE
INICIAR_PATOTA 1 FSCK_PatotaA.txt 0|0
INICIAR_PATOTA 1 FSCK_PatotaB.txt 8|0
INICIAR_PATOTA 1 FSCK_PatotaC.txt 8|8
INICIAR_PATOTA 1 FSCK_PatotaD.txt 0|8


definitely lost: 1,176 bytes in 78 blocks
indirectly lost: 1,209 bytes in 195 blocks

*/ 


     
int main(int argc, char ** argv){  

	loggerDiscordiador = log_create("discordiador.log", "discordiador.c", 0, LOG_LEVEL_INFO); 

	log_info(loggerDiscordiador, "---------PROGRAMA INICIADO---------- ");

	listaTripulantes = list_create();
	listaReady = list_create();
	listaBloqueados = list_create(); 
	listaTrabajando = list_create();
	tareasDeIO = list_create();
	listaTerminados = list_create();
	listaNuevos = list_create();  
	listaBloqueadosEmergencia = list_create();
	listaBloqueadosSabotaje = list_create();

	list_add(tareasDeIO,"GENERAR_OXIGENO");
	list_add(tareasDeIO,"CONSUMIR_OXIGENO");
	list_add(tareasDeIO,"GENERAR_OXIGENO");
	list_add(tareasDeIO,"GENERAR_COMIDA");
	list_add(tareasDeIO,"CONSUMIR_COMIDA");
	list_add(tareasDeIO,"GENERAR_BASURA");
	list_add(tareasDeIO,"DESCARTAR_BASURA");

	posicionBase = malloc(sizeof("0|0"));
	config = malloc(sizeof(config_create("./cfg/discordiador.config")));
	posicionBase = "0|0";
	config = config_create("./cfg/discordiador.config");
	

	socketParaSabotajes = conectarImongo();

	pthread_t hiloSabotajes;
    pthread_create(&hiloSabotajes, NULL, (void*) atenderImongo, NULL);


	cicloCPU = config_get_int_value(config, "RETARDO_CICLO_CPU");
	tiempoSabotaje = config_get_int_value(config, "DURACION_SABOTAJE");
	gradoMultitarea = config_get_int_value(config, "GRADO_MULTITAREA");
	tipoAlgoritmo = config_get_string_value(config, "ALGORITMO");
	//free(config);

	sem_init(&semaforoTripulantes, 0,  gradoMultitarea);
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
	sem_init(&semaforoSabotaje,0,0);
	sem_init(&semaforoPlanificacionPausada,0,0);
	sem_init(&mutexPID, 0, 1); 
	sem_init(&estoyListo,0,0);
	sem_init(&contadorBloqueados, 0, 1);
	

	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	pthread_join(hiloConsola, NULL);

	


	
sem_destroy(&semaforoTripulantes);


return 0;
}



//-----------------------------CONECTAR---------------------------------------------------------------------------------------



int conectarImongo(){
		char * ip = config_get_string_value(config, "IP_I_MONGO_STORE");
		char * puerto = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
		return crear_conexion(ip, puerto);
}

int conectarMiRAM(){
	char * ip = config_get_string_value(config, "IP_MI_RAM_HQ");
	char * puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
	 return crear_conexion(ip, puerto);
}

//-----------------------------CONSOLA---------------------------------------------------------------------------------------

void consola(){

	char * instruccion; 
	char ** vectorInstruccion;


	while(1) {

		instruccion = readline("Ingrese pr??xima instrucci??n: \n");

		log_info(loggerDiscordiador, "INSTRUCCION LEIDA: %s", instruccion); 

		vectorInstruccion = string_split(instruccion, " ");

		free(instruccion);

		if(strcmp(vectorInstruccion[0], "INICIAR_PATOTA") == 0) {

			iniciarPatota(vectorInstruccion);
			
		}
		

		else if(strcmp(vectorInstruccion[0], "INICIAR_PLANIFICACION") == 0){
				
				if(planificacionPausada){

				ponerReadyNuevosTripulantes();

				planificacionPausada = false;
				int r = 0;
				for(r = 0 ; r < ( gradoMultitarea + cantidadBloqueados ); r ++){
				sem_post(&semaforoPlanificacionPausada);
				}
				r=0;
				sem_wait(&contadorBloqueados);
				cantidadBloqueados = 0;
				sem_post(&contadorBloqueados);
			

				} else {						
				pthread_t  hiloTrabajadorFIFO; 
				pthread_create(&hiloTrabajadorFIFO, NULL, ponerATrabajar, NULL );
				}

		}	

		else if(strcmp(vectorInstruccion[0], "LISTAR_TRIPULANTES") == 0) {
			
			listarTripulantes();

		}
	
		else if(strcmp(vectorInstruccion[0], "EXPULSAR_TRIPULANTE") == 0) {

			bool coincideID(TCB_DISCORDIADOR * tripulantee){
				return tripulantee->tid ==  atoi(vectorInstruccion[1]);
			}

			TCB_DISCORDIADOR * tripulante = list_find(listaTripulantes,coincideID);	
		
			tripulante->fueExpulsado = true;
			free(vectorInstruccion[1]);
			
		}
		
		
		else if(strcmp(vectorInstruccion[0], "PAUSAR_PLANIFICACION") == 0) {

			planificacionPausada = true;

		}

		else if(strcmp(vectorInstruccion[0], "OBTENER_BITACORA") == 0) {
			serializarYMandarPedidoDeBitacora(atoi(vectorInstruccion[1]));
			free(vectorInstruccion[1]);
		}

		else if(strcmp(vectorInstruccion[0], "EXIT") == 0){
			log_info(loggerDiscordiador, "-------------PROGRAMA TERMINADO-------------"); 
			return 0; 
		}

		else{
			printf("Instruccion invalida \n"); 
		}

		free(vectorInstruccion[0]);
		free(vectorInstruccion);

	}

	free(posicionBase);
	free(config);

}


//-----------------------------INICIAR PATOTA---------------------------------------------------------------------------------------


void iniciarPatota(char ** vectorInstruccion){

	char * tareas = leerTareas(vectorInstruccion[2]);
	if(strcmp(tareas, "ERROR") == 0){
		return; 
	}
	int socket = conectarMiRAM();

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

					if (vectorInstruccion[indice_posiciones] != NULL) {
						
						TCB_DISCORDIADOR * tripulante = crearTCB(vectorInstruccion[3 + i],idPatota);
						free(vectorInstruccion[3+i]);
						list_add(listaTCBsNuevos, tripulante);
						pthread_create(&tripulantes[i], NULL, subModuloTripulante , tripulante);
						pthread_detach(tripulantes[i]);
						log_info(loggerDiscordiador, "Tripulante creado: ID: %d, Posicion %d|%d, Estado: %c ", tripulante->tid, tripulante->posicionX, tripulante->posicionY, tripulante->estado ); 			

						indice_posiciones++;
					} else {
						TCB_DISCORDIADOR * tripulante = crearTCB(posicionBase,idPatota);
						list_add(listaTCBsNuevos, tripulante);
						pthread_create(&tripulantes[i], NULL, subModuloTripulante , tripulante);
						pthread_detach(tripulantes[i]);
						log_info(loggerDiscordiador, "Tripulante creado: ID: %d, Posicion %d|%d, Estado: %c ", tripulante->tid, tripulante->posicionX, tripulante->posicionY, tripulante->estado ); 			

					}					
					
				}  
				
				serializarYMandarPCB(tareas,socket, idPatota, cantidadTripulantes, listaTCBsNuevos);

				int header;
				recv(socket, &(header) , sizeof(int), 0);
				if(header == PATOTA_CREADA){
					log_info(loggerDiscordiador, "Patota creada por MiRam"); 
				}
				
				
				int y;
				for(y = 0 ; y < cantidadTripulantes ; y++){
				if(!planificacionPausada){

						TCB_DISCORDIADOR * tripulante = list_get(listaTCBsNuevos,y);
						
						salirDeListaEstado(tripulante);
						

						tripulante->estado = 'R';
						
						sem_wait(&cambiarAReady);
						list_add(listaReady, tripulante);
						sem_post(&cambiarAReady);

											
					}
				sem_post(&esperarAlgunTripulante); 
				}  

				list_destroy(listaTCBsNuevos);

				free(vectorInstruccion[1]);
	
}


TCB_DISCORDIADOR * crearTCB(char * posiciones, uint32_t pid){

		char ** vectorPosiciones = string_split(posiciones,"|" );
		TCB_DISCORDIADOR * tripulante = malloc(sizeof(TCB_DISCORDIADOR));

		sem_init(&tripulante->semaforoTrabajo, 0, 0); 
		sem_init(&tripulante->termineIO,0,0);
		sem_init(&tripulante->empeceIO,0,0);
	 
		tripulante->estado = 'N';
		tripulante->tid = proximoTID;  
		tripulante->posicionX = atoi(vectorPosiciones[0]);
		tripulante->posicionY = atoi(vectorPosiciones[1]);
		tripulante->pid = pid;
		tripulante->fueExpulsado = false; 
		tripulante->descripcionTarea = malloc(30);
		tripulante->parametros = -1;
		tripulante->tiempo = 0;

		sem_wait(&cambiarANuevo);	
		list_add(listaNuevos, tripulante);
		sem_post(&cambiarANuevo);

		list_add(listaTripulantes, tripulante);

		proximoTID ++;

		free(vectorPosiciones[0]);
		free(vectorPosiciones[1]);
		free(vectorPosiciones);
		//free(posiciones);

		return tripulante;  
	} 

 
void subModuloTripulante(TCB_DISCORDIADOR * tripulante) { 
	sleep(2);
	int contador = 0; // cantidad de quantum ya utilizado
	int quantum = atoi(config_get_string_value(config, "QUANTUM"));
	bool esLaPrimera = true;
	bool ultimaTareaDeIO = false;
	bool estaEmpezando;
	int tiempoBloqueo = 0;
	uint32_t posxV;
	uint32_t posyV;
	bool tareaTerminada = true; 
	bool noHayMasTareas = false;
	tarea_struct * tarea = malloc(sizeof(tarea_struct)); //HECHO
	char * descripcionTareaAnterior;
	char ** vectorTarea = malloc(45);
	 vectorTarea[0]= malloc(30);
	 vectorTarea[1]= malloc(5); 
	 vectorTarea[2]= malloc(5); 
	 vectorTarea[3]= malloc(5); 
	//LISTO   
	char ** requerimientosTarea = malloc(30); //LISTO
	requerimientosTarea[0] = malloc(25);
	requerimientosTarea[1] = malloc(5);
	while (1) {

		

		if( tripulante->fueExpulsado){
			expulsarTripulate(tripulante);
			 break; 
			 }
		

		if(tareaTerminada && !esLaPrimera ){

				//free(requerimientosTarea[0]);
				//free(requerimientosTarea[1]);
				//free(requerimientosTarea);
				
				//free(tarea->descripcionTarea);
				//free(tarea);

				posxV = tripulante->posicionX;
				posyV = tripulante->posicionY;

				int socket = conectarMiRAM();
				

				serializarYMandarPedidoDeTarea(socket, tripulante->pid, tripulante->tid);

				t_paquete* paquete = malloc(sizeof(t_paquete)); //LISTO
				paquete->buffer = malloc(sizeof(t_buffer)); //LISTO

				int headerRECV = recv(socket, &(paquete->header) , sizeof(int), MSG_WAITALL);
				if(!headerRECV) { log_error(loggerDiscordiador, "No se pudo recibir el header al recibir una tarea");}

				
				switch (paquete->header)
				{
				case HAY_TAREA:;
				
				free(vectorTarea[0]);
				free(vectorTarea[1]);
				free(vectorTarea[2]);
				free(vectorTarea[3]);
				free(vectorTarea);

					int statusTamanioBuffer = recv(socket,&(paquete-> buffer-> size), sizeof(uint32_t), MSG_WAITALL);
					if(! statusTamanioBuffer){ log_error(loggerDiscordiador, "No se pudo recibir el tamanio del buffer al recibir una tarea");}

					paquete->buffer->stream = malloc(paquete->buffer->size); //LISTO

					int BUFFER_RECV = recv(socket,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream
					if(! BUFFER_RECV){ log_error(loggerDiscordiador,"No se pudo recibir el buffer al recibir una tarea");}

					void* stream = paquete->buffer->stream;

					int tamanioTareas;
					memcpy(&tamanioTareas, stream, sizeof(int));
					stream += sizeof(int);
					char* stringTarea = malloc(tamanioTareas);  // LISTO
					memcpy(stringTarea, stream, tamanioTareas);
					stream += tamanioTareas;

					vectorTarea = string_split(stringTarea, ";");
					

					if(string_contains(vectorTarea[0]," ")){
						requerimientosTarea = string_split(vectorTarea[0]," ");
						tarea->descripcionTarea = requerimientosTarea[0];
						tarea->parametro = atoi(requerimientosTarea[1]);
					} else{
						tarea->descripcionTarea = vectorTarea[0];
						tarea->parametro = -1;
					}

					tarea->posicionX = atoi(vectorTarea[1]);			//Llena el struct tarea 
					tarea->posicionY = atoi(vectorTarea[2]);
					tarea->tiempo = atoi(vectorTarea[3]);

					log_info(loggerDiscordiador,"Tarea pedida por tripulante %d: %s Posicion: %d|%d Duracion: %d ",tripulante->tid ,tarea->descripcionTarea, tarea->posicionX, tarea->posicionY, tarea->tiempo);

					free(stringTarea);
					free(paquete->buffer->stream);
					free(paquete->buffer);
					free(paquete);
					
					break;
				
				case NO_HAY_TAREA:								
									free(paquete->buffer);
									free(paquete);
									noHayMasTareas = true;
									log_info(loggerDiscordiador,"Tripulante %d termino de trabajar ",tripulante->tid);

									break;
				}
				
				tareaTerminada = false;
				estaEmpezando = true;

		}


		if(ultimaTareaDeIO){
			sem_post(&gestionarIO);
			sem_wait(&tripulante->empeceIO);

			for(int e = 0; e < tripulante->tiempo + cicloCPU; e++){
				
				if( tripulante->fueExpulsado){
					expulsarTripulate(tripulante);
						break; 
						}

				if(haySabotaje){ sem_wait(&semaforoSabotaje);}
				

				sleep(cicloCPU);	
			}
			
			if( tripulante->fueExpulsado){
					expulsarTripulate(tripulante);
						break; 
						}
			if(haySabotaje){ sem_wait(&semaforoSabotaje);}
			if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
					
			sem_post(&tripulante->termineIO);
			serializarYMandarFinalizacionTarea(tripulante->tid, tripulante->descripcionTarea);
			

			log_info(loggerDiscordiador, "Tripulante %d termin?? su tarea", tripulante->tid);
		
			sem_wait(&estoyListo);

			if(!noHayMasTareas){ 
				sem_post(&esperarAlgunTripulante);
			}
		
		}

		if(noHayMasTareas){
				free(vectorTarea[0]);
				free(vectorTarea[1]);
				free(vectorTarea[2]);
				free(vectorTarea[3]);
				free(vectorTarea);
			break;} //Si MIRAM avisa que no hay mas tareas termina el hilo

		if(strcmp(tipoAlgoritmo, "FIFO") == 0){

				ultimaTareaDeIO = false;

				sem_wait(&tripulante->semaforoTrabajo);	

				if(esLaPrimera){

				posxV = tripulante->posicionX;
				posyV = tripulante->posicionY;

				int socket = conectarMiRAM();
				
				serializarYMandarPedidoDeTarea(socket, tripulante->pid, tripulante->tid);

				t_paquete* paquete = malloc(sizeof(t_paquete)); //LISTO
				paquete->buffer = malloc(sizeof(t_buffer)); //LISTO

				int headerRECV = recv(socket, &(paquete->header) , sizeof(int), MSG_WAITALL);
				if(!headerRECV) { log_error(loggerDiscordiador, "No se pudo recibir el header al recibir una tarea");}

				
				switch (paquete->header)
				{
				case HAY_TAREA:;

					int statusTamanioBuffer = recv(socket,&(paquete-> buffer-> size), sizeof(uint32_t), MSG_WAITALL);
					if(! statusTamanioBuffer){ log_error(loggerDiscordiador, "No se pudo recibir el tamanio del buffer al recibir una tarea");}

					paquete->buffer->stream = malloc(paquete->buffer->size); //LISTO

					int BUFFER_RECV = recv(socket,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream
					if(! BUFFER_RECV){ log_error(loggerDiscordiador,"No se pudo recibir el buffer al recibir una tarea");}

					void* stream = paquete->buffer->stream;

					int tamanioTareas;
					memcpy(&tamanioTareas, stream, sizeof(int));
					stream += sizeof(int);
					char* stringTarea = malloc(tamanioTareas);  // LISTO
					memcpy(stringTarea, stream, tamanioTareas);
					stream += tamanioTareas;

					vectorTarea = string_split(stringTarea, ";");
					requerimientosTarea = string_split(vectorTarea[0]," ");

					if(string_contains(vectorTarea[0]," ")){
						tarea->descripcionTarea = requerimientosTarea[0];
						tarea->parametro = atoi(requerimientosTarea[1]);
					} else{
						tarea->descripcionTarea = vectorTarea[0];
						tarea->parametro = -1;
					}

					tarea->posicionX = atoi(vectorTarea[1]);			//Llena el struct tarea 
					tarea->posicionY = atoi(vectorTarea[2]);
					tarea->tiempo = atoi(vectorTarea[3]);

					log_info(loggerDiscordiador,"Tarea pedida por tripulante %d: %s Posicion: %d|%d Duracion: %d ",tripulante->tid ,tarea->descripcionTarea, tarea->posicionX, tarea->posicionY, tarea->tiempo);

					free(stringTarea);
					free(paquete->buffer->stream);
					free(paquete->buffer);
					free(paquete);
					
					break;
				
				case NO_HAY_TAREA:								
									free(paquete->buffer);
									free(paquete);
									noHayMasTareas = true;
									log_info(loggerDiscordiador,"Tripulante %d termino de trabajar ",tripulante->tid);

									break;
				}
				
				esLaPrimera = false;

		}

				memcpy((tripulante->descripcionTarea),tarea->descripcionTarea,30); //liberar cuando expulso
				memcpy(&(tripulante->parametros),&(tarea->parametro), 4);	
				memcpy(&(tripulante->tiempo),&(tarea->tiempo), 4);	

				
				trasladarseA(tarea->posicionX,tarea->posicionY, tripulante);

				if( tripulante->fueExpulsado){
							expulsarTripulate(tripulante);
							 break; 
				}
				serializarYMandarPosicionBitacora(tripulante->tid, posxV, posyV, tripulante->posicionX, tripulante->posicionY);

				if(esTareaDeIO(tarea->descripcionTarea)){

					cambiarDeEstado(tripulante,'B');
					if(haySabotaje){ sem_wait(&semaforoSabotaje);}
					if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
					sem_post(&semaforoTripulantes);
					ultimaTareaDeIO = true;

					free(requerimientosTarea[0]);
					free(requerimientosTarea[1]);
					free(requerimientosTarea);

				} else{

					serializarYMandarInicioTareaNormal(tripulante->tid, tarea->descripcionTarea);
					for(int e = 0; e < tarea->tiempo; e++){

						if( tripulante->fueExpulsado){
							expulsarTripulate(tripulante);
							 break; 
						}

						if(haySabotaje){ sem_wait(&semaforoSabotaje);}
						if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}

						sleep(cicloCPU);	
					}

					if( tripulante->fueExpulsado){
						expulsarTripulate(tripulante);
						 break; 
					 }

					serializarYMandarFinalizacionTarea(tripulante->tid, tarea->descripcionTarea);
					sem_post(&tripulante->semaforoTrabajo); 

					log_info(loggerDiscordiador, "Tripulante %d termin?? su tarea", tripulante->tid);

				}
					if(haySabotaje){ sem_wait(&semaforoSabotaje);}
					if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
				
				tareaTerminada = true;
					// free(requerimientosTarea[0]);
					// free(requerimientosTarea[1]);
					// free(requerimientosTarea);
					// free(vectorTarea[0]);
					// free(vectorTarea[1]);
					// free(vectorTarea[2]);
					// free(vectorTarea[3]);
					// free(vectorTarea);
				//free(tarea->descripcionTarea);
				// free(tarea);


									     } else{	

											 	if(esLaPrimera){
													 		contador = 0;

															sem_wait(&tripulante->semaforoTrabajo);
																				
															posxV = tripulante->posicionX;
															posyV = tripulante->posicionY;

															int socket = conectarMiRAM();
															

															serializarYMandarPedidoDeTarea(socket, tripulante->pid, tripulante->tid);

															t_paquete* paquete = malloc(sizeof(t_paquete)); //LISTO
															paquete->buffer = malloc(sizeof(t_buffer)); //LISTO

															int headerRECV = recv(socket, &(paquete->header) , sizeof(int), MSG_WAITALL);
															if(!headerRECV) { log_error(loggerDiscordiador, "No se pudo recibir el header al recibir una tarea");}

															
															switch (paquete->header)
															{
															case HAY_TAREA:;

																int statusTamanioBuffer = recv(socket,&(paquete-> buffer-> size), sizeof(uint32_t), MSG_WAITALL);
																if(! statusTamanioBuffer){ log_error(loggerDiscordiador, "No se pudo recibir el tamanio del buffer al recibir una tarea");}

																paquete->buffer->stream = malloc(paquete->buffer->size); //LISTO

																int BUFFER_RECV = recv(socket,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream
																if(! BUFFER_RECV){ log_error(loggerDiscordiador,"No se pudo recibir el buffer al recibir una tarea");}

																void* stream = paquete->buffer->stream;

																int tamanioTareas;
																memcpy(&tamanioTareas, stream, sizeof(int));
																stream += sizeof(int);
																char* stringTarea = malloc(tamanioTareas);  // LISTO
																memcpy(stringTarea, stream, tamanioTareas);
																stream += tamanioTareas;

																vectorTarea = string_split(stringTarea, ";");
																requerimientosTarea = string_split(vectorTarea[0]," ");

																if(string_contains(vectorTarea[0]," ")){
																	tarea->descripcionTarea = requerimientosTarea[0];
																	tarea->parametro = atoi(requerimientosTarea[1]);
																} else{
																	tarea->descripcionTarea = vectorTarea[0];
																	tarea->parametro = -1;
																}

																tarea->posicionX = atoi(vectorTarea[1]);			//Llena el struct tarea 
																tarea->posicionY = atoi(vectorTarea[2]);
																tarea->tiempo = atoi(vectorTarea[3]);

																log_info(loggerDiscordiador,"Tarea pedida por tripulante %d: %s Posicion: %d|%d Duracion: %d ",tripulante->tid ,tarea->descripcionTarea, tarea->posicionX, tarea->posicionY, tarea->tiempo);

																free(stringTarea);
																free(paquete->buffer->stream);
																free(paquete->buffer);
																free(paquete);
																
																break; 
															
															case NO_HAY_TAREA:								
																				free(paquete->buffer);
																				free(paquete);
																				noHayMasTareas = true;
																				log_info(loggerDiscordiador,"Tripulante %d termino de trabajar ",tripulante->tid);

																				break;
															}
															
															esLaPrimera = false;
															tareaTerminada = false;
															estaEmpezando = true;
												}

												memcpy((tripulante->descripcionTarea),tarea->descripcionTarea,30);
												memcpy(&(tripulante->parametros),&(tarea->parametro), 4);
												memcpy(&(tripulante->tiempo),&(tarea->tiempo), 4);						

												if(contador == quantum && !ultimaTareaDeIO){
												cambiarDeEstado(tripulante,'R');
												sem_post(&esperarAlgunTripulante);
												sem_post(&semaforoTripulantes);
												sem_wait(&tripulante->semaforoTrabajo);
												contador = 0;
												}

												if(ultimaTareaDeIO){
													ultimaTareaDeIO = false;
													sem_wait(&tripulante->semaforoTrabajo);
													contador = 0;
												}

												//free(config);
												// Primero tiene que ir a la posicion en la que esta la tarea, y para esto gasta
												// quantum. Es por eso que primero se mueve en X lo que pueda, y cuando llega a 
												// su posicion en X, hace lo mismo con Y. Una vez haya llegado a donde se encuentra
												// la tarea la ejecuta poco a poco respetando el quantum
													
													// Moverse en x

												if( tripulante->posicionX != tarea->posicionX){  // Aca se fija que no haya llegado a su posicion en x, si llego ni entra

													for(contador; contador < quantum ;  contador ++){

														if( tripulante->fueExpulsado){
															expulsarTripulate(tripulante);
															 break; 
														 }

														if(haySabotaje){ sem_wait(&semaforoSabotaje);}
														if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}

														sleep(cicloCPU);
														
														if(tarea->posicionX < tripulante->posicionX){
															tripulante->posicionX--;
														} else {
															tripulante->posicionX++;
														}	

														serializarYMandarPosicion(tripulante);
														
														if( tripulante->posicionX == tarea->posicionX){ // si llego sale del for y pasa a Y
															contador++;
															break; 
														}


													}

													if(tripulante->posicionY == tarea->posicionY){
														serializarYMandarPosicionBitacora(tripulante->tid, posxV, posyV, tripulante->posicionX, tripulante->posicionY);
													}	

													if(tripulante->posicionY != tarea->posicionY){
													log_info(loggerDiscordiador, "Tripulante %d termino su Q en %d|%d", tripulante->tid, tripulante->posicionX, tripulante->posicionY);
													}
												}

													// Moverse en y

												if(tripulante->posicionY != tarea->posicionY && contador < quantum ){ // Igual que con x
													
													for(contador; contador < quantum ;  contador ++){
														
														if( tripulante->fueExpulsado){
															expulsarTripulate(tripulante);
															 break; 
														 }

														if(haySabotaje){ sem_wait(&semaforoSabotaje);}
														if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}

														sleep(cicloCPU);

														if(tarea->posicionY < tripulante->posicionY){
															tripulante->posicionY--;
														} else {
															tripulante->posicionY++;
														}

														if(haySabotaje){ sem_wait(&semaforoSabotaje);}
														if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
																								
														serializarYMandarPosicion(tripulante);

														if(tripulante->posicionY == tarea->posicionY){
															contador++;
															serializarYMandarPosicionBitacora(tripulante->tid, posxV, posyV, tripulante->posicionX, tripulante->posicionY);
															break; 
														}		
														
													}

													if(haySabotaje){ sem_wait(&semaforoSabotaje);}
													if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
													if(tripulante->posicionY != tarea->posicionY){
														log_info(loggerDiscordiador, "Tripulante %d termino su Q en %d|%d", tripulante->tid, tripulante->posicionX, tripulante->posicionY);
													} else{
														log_info(loggerDiscordiador, "Tripulante %d llego a la posicion de su tarea, le queda %d de quantum", tripulante->tid, quantum - contador);
													}

													if(haySabotaje){ sem_wait(&semaforoSabotaje);}
													if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
													
												}

												

												for( contador ; contador < quantum ;  contador ++){ //Si entra aca, es porque ya llego a la psocion y todavia le queda quantum para ejecutar
			
													if(estaEmpezando){
														serializarYMandarInicioTareaNormal(tripulante->tid, tarea->descripcionTarea);
														estaEmpezando = false;
													}

													if(esTareaDeIO(tarea->descripcionTarea)){

															if(haySabotaje){ sem_wait(&semaforoSabotaje);}
															if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
															tareaTerminada = true;
															//free(tarea->descripcionTarea);
															cambiarDeEstado(tripulante, 'B');
															sem_post(&semaforoTripulantes);
															//sem_post(&esperarAlgunTripulante);
															descripcionTareaAnterior = tarea->descripcionTarea;
															tiempoBloqueo = tarea->tiempo;			
															contador++;
															ultimaTareaDeIO = true;
															free(requerimientosTarea[0]);
															free(requerimientosTarea[1]);
															free(requerimientosTarea);
															// free(vectorTarea[0]);
															// free(vectorTarea[1]);
															// free(vectorTarea[2]);
															// free(vectorTarea[3]);
															// free(vectorTarea);
															break;

														}

														else{

															if(haySabotaje){ sem_wait(&semaforoSabotaje);}
															if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
															if( tripulante->fueExpulsado){
																expulsarTripulate(tripulante);
																	 break; 
			 													}

															sleep(cicloCPU);
															tarea->tiempo --; // Cada tarea tiene un tiempo, cuando ese tiempo llegue a 0 va a gestionar la tarea en el if de abajo, si no llega a 0 va a seguir haciendo tiempo si su quantum lo permite
															
																
																if(tarea->tiempo == 0){
																	if( tripulante->fueExpulsado){
																		expulsarTripulate(tripulante);
																	 	break; 
			 														}
																	serializarYMandarFinalizacionTarea(tripulante->tid, tarea->descripcionTarea);
																	tareaTerminada = true;
																	// free(requerimientosTarea[0]);
																	// free(requerimientosTarea[1]);
																	// free(requerimientosTarea);
																	// free(vectorTarea[0]);
																	// free(vectorTarea[1]);
																	// free(vectorTarea[2]);
																	// free(vectorTarea[3]);
																	// free(vectorTarea);
																	//free(tarea->descripcionTarea);
																	// free(tarea);								
																	//cambiarDeEstado(tripulante,'R');
																	//sem_post(&esperarAlgunTripulante);																			 
																	//sem_post(&semaforoTripulantes); 
																	log_info(loggerDiscordiador, "Tripulante %d termin?? su tarea", tripulante->tid);
																	contador++;
																	break;
																}

															
														}
																if( tripulante->fueExpulsado){
																expulsarTripulate(tripulante);
																	 break; 
			 													}
																//log_info(loggerDiscordiador, "Tripulante  %d le faltan %ds para terminar la tarea y le queda %d quantum restante", tripulante->tid, tarea->tiempo, quantum-contador);

													
																	}


												
		
											}

			


		}
 

		//sem_post(&esperarAlgunTripulante);
		sem_post(&semaforoTripulantes); 					
		salirDeListaEstado(tripulante); 
		sem_wait(&cambiarAFinalizado);
		log_info(loggerDiscordiador, "%s  tripulante %d", tripulante->descripcionTarea, tripulante->tid);
		tripulante->estado = 'F';
		list_add(listaTerminados, tripulante); 
		sem_post(&cambiarAFinalizado);
		//close(socket);


}
  
  
void ponerATrabajar(){

	pthread_t gestionarIO;
	pthread_create(&gestionarIO, NULL, (void*) gestionadorIO, NULL);
	 
	planificacionPausada = false;
	
	while(1){ 

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
	
	while(1){

			sem_wait(&gestionarIO);

			sem_wait(&cambiarABloqueado);
			TCB_DISCORDIADOR* tripulantee = list_get(listaBloqueados, 0);
			sem_post(&cambiarABloqueado);

			sem_post(&tripulantee->empeceIO);

			log_info(loggerDiscordiador,"Arranque IO de %d", tripulantee->tid);

			gestionarTarea(tripulantee->descripcionTarea, tripulantee->parametros,tripulantee->tid);	

			if(haySabotaje){
						sem_wait(&semaforoSabotaje);
					}

			sem_wait(&tripulantee->termineIO);

			cambiarDeEstado(tripulantee,'R');
			
			sem_post(&estoyListo);
			
		 } 
			
			
}
//-----------------------------LISTAR TRIPULANTES---------------------------------------------------------------------------------------


void listarTripulantes(){

printf("--------------------------------------------------------- \nEstado actual de la nave: %s    \n\n", temporal_get_string_time("%d/%m/%y %H:%M:%S"));

	for(int i = 0; i < list_size(listaTripulantes) ; i++){

		TCB_DISCORDIADOR *tripulante = list_get(listaTripulantes,i);
		// PCB *patota = tripulante->punteroPCB;

		printf("Tripulante: %2d (%c)  Patota: %2d   Estado: %c   Posicion: %d|%d \n", tripulante->tid , idMapa(tripulante->tid) , tripulante->pid  , tripulante->estado, tripulante->posicionX, tripulante->posicionY);

	}

printf("--------------------------------------------------------- \n");

}

char idMapa(uint32_t tid){

	char id = 0;

	id = tid + 65;

	if (id > 90) {
		id += 6;
	}
	return id;

}

//-----------------------------EXPULAR_TRIPULANTE---------------------------------------------------------------------------------------------------

void expulsarTripulate(TCB_DISCORDIADOR * tripulante){

			 uint32_t tid = tripulante->tid;

			bool coincideID(TCB_DISCORDIADOR * tripulante){
				return tripulante->tid == tid;
			}

 
			serializaYMandarExpulsado(tripulante->tid, tripulante->pid);

			salirDeListaEstado(tripulante);
			list_remove_by_condition(listaTripulantes, coincideID);
			//free(tripulante);  VER BIEN
}

//-----------------------------TAREAS---------------------------------------------------------------------------------------------------
 

void trasladarseA(uint32_t posicionX,uint32_t posicionY, TCB_DISCORDIADOR * tripulante){

	while(posicionX != tripulante->posicionX)
	{
		if( tripulante->fueExpulsado){ return 0;}		
		if(haySabotaje){ sem_wait(&semaforoSabotaje);}
		if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}

		if(posicionX < tripulante->posicionX){
			tripulante->posicionX--;
		} else {
			tripulante->posicionX++;
		}

		sleep(cicloCPU);
		serializarYMandarPosicion(tripulante);
	}
	
	while(posicionY != tripulante->posicionY)
	{	
		if(haySabotaje){ sem_wait(&semaforoSabotaje);}
		if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}
		if( tripulante->fueExpulsado){ return 0;}

		if(posicionY < tripulante->posicionY){
			tripulante->posicionY--;
		} else {
			tripulante->posicionY++;
		}

		sleep(cicloCPU);
		serializarYMandarPosicion(tripulante);
	}
	
	log_info(loggerDiscordiador, "Tripulante %d ahora esta en %d|%d ",tripulante->tid,tripulante->posicionX,tripulante->posicionY);

}

void trasladarseADuranteSabotaje(uint32_t posicionX,uint32_t posicionY, TCB_DISCORDIADOR * tripulante){

	while(posicionX != tripulante->posicionX)
	{

		if(posicionX < tripulante->posicionX){
			tripulante->posicionX--;
		} else {
			tripulante->posicionX++;
		}

		sleep(cicloCPU);
		serializarYMandarPosicion(tripulante); 
	}
	
	while(posicionY != tripulante->posicionY)
	{	


		if(posicionY < tripulante->posicionY){
			tripulante->posicionY--;
		} else {
			tripulante->posicionY++;
		}

		sleep(cicloCPU);
		serializarYMandarPosicion(tripulante);
	}
	
	log_info(loggerDiscordiador, "Tripulante %d ahora esta en %d|%d ",tripulante->tid,tripulante->posicionX,tripulante->posicionY);

}

void gestionarTarea(char * descripcionTarea, int parametros, uint32_t tid){

				if( strcmp(descripcionTarea,"GENERAR_OXIGENO") == 0 ){
						serializarYMandarInicioTareaIO(parametros, GENERAR_OXIGENO,tid);
						log_info(loggerDiscordiador, "GENERAR_OXIGENO %d", parametros);
					} 

					else if(strcmp(descripcionTarea,"CONSUMIR_OXIGENO") == 0){
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
						log_info(loggerDiscordiador, "GENERAR_BASURA %d", parametros);
					}

					else if(strcmp(descripcionTarea,"DESCARTAR_BASURA") == 0){
						serializarYMandarInicioTareaIO(parametros, DESCARTAR_BASURA,tid);
					}
					else{
					}
					

}


char * leerTareas(char* nombreTareas) {

	char * pathTareas = string_from_format("/home/utnso/%s", nombreTareas);
	FILE* archivo = fopen(pathTareas,"r");
	if (archivo == NULL)
	{
		printf("El path de tareas recibido es invalido \n");
		return "ERROR"; 
	}
	else 
	{
		log_info(loggerDiscordiador, "path de tareas recibido: %s", pathTareas);
	}
	
	
    fseek(archivo, 0, SEEK_END);
    int tamanioArchivo = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    char* lineas = (char*) calloc(tamanioArchivo, sizeof(char)); 
    fread(lineas, sizeof(char), tamanioArchivo, archivo);

    // ac?? tengo un string "lineas" con todas las lineas del archivo juntas, incluyendo los saltos de linea

	fclose(archivo);

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

	t_paquete* paquete = malloc(sizeof(t_paquete)); // HECHO
	paquete->buffer;  //HECHO

	paquete->header = header;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t) ); //HECHO
	int offset2 = 0;

	memcpy(a_enviar + offset2, &(paquete->header), sizeof(int));
	offset2 += sizeof(int);

	memcpy(a_enviar + offset2, &(paquete->buffer->size), sizeof(uint32_t));
	offset2 += sizeof(uint32_t);

	memcpy(a_enviar + offset2, paquete-> buffer-> stream, paquete->buffer->size);

	send(socket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


void serializarYMandarPCB(char * tareas, int socket, uint32_t pid, int cantidadTCB, t_list * listaTCBS){

	int  offset = 0;
	
	
	int  tamanioTareas = strlen(tareas);

	t_buffer* buffer = malloc(sizeof(t_buffer));   //HECHO

	buffer-> size = sizeof(tamanioTareas) + tamanioTareas + sizeof(cantidadTCB) + sizeof(TCB) * cantidadTCB + sizeof(uint32_t);

	void* stream = malloc(buffer->size); //HECHO

	memcpy(stream+offset, &tamanioTareas, sizeof(tamanioTareas));
	offset += sizeof(int);

	memcpy(stream+offset, tareas, tamanioTareas);
	offset += tamanioTareas;

	memcpy(stream + offset, &pid, sizeof(uint32_t)); 
	offset += sizeof(uint32_t); 
	
	memcpy(stream+offset, & cantidadTCB, sizeof(cantidadTCB));
	offset += sizeof(cantidadTCB);

	for(int i = 0;i<cantidadTCB; i++){

		TCB_DISCORDIADOR * tripulante = list_get(listaTCBS,i);
		
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

	close(socket);
	
	free(tareas);
}

void serializarYMandarInicioTareaIO(int parametro, int tipoTarea, uint32_t tid ){
	int socket = conectarImongo();

	t_parametro * parametroS = malloc(sizeof(parametroS)); // HECHO
	parametroS->parametro = parametro;
	parametroS->tid = tid;

	t_buffer* buffer = malloc(sizeof(t_buffer)); //HECHO

	buffer-> size = 2 * sizeof(int) + sizeof(uint32_t);

	void* stream = malloc(buffer->size);  // HECHO

	int offset = 0;

	memcpy(stream+offset, &tipoTarea, sizeof(int));
	offset += sizeof(int);

	memcpy(stream+offset, &(parametroS->parametro), sizeof(int));
	offset += sizeof(int);

	memcpy(stream+offset, &(parametroS->tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, HACER_TAREA);

	free(parametroS);
	close(socket);
}

void serializarYMandarPedidoDeTarea(int socket, uint32_t pid, uint32_t tid){

	t_buffer* buffer = malloc(sizeof(t_buffer));   //HECHO
	buffer-> size = sizeof(uint32_t) * 2;

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream + offset, &tid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	memcpy(stream + offset, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream; 

	mandarPaqueteSerializado(buffer, socket, PEDIR_TAREA);
	
}

void serializarYMandarPosicion(TCB_DISCORDIADOR * tripulante){

	int socketMIRAM = conectarMiRAM();

	t_buffer* buffer = malloc(sizeof(t_buffer));  //HECHO
	buffer-> size = sizeof(uint32_t) * 4;

	void* stream = malloc(buffer->size); //HECHO

	int offset = 0;

	memcpy(stream+offset, &(tripulante->tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->posicionX), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->posicionY), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream; 

	if(planificacionPausada){sem_wait(&semaforoPlanificacionPausada);}

	mandarPaqueteSerializado(buffer, socketMIRAM, ACTUALIZAR_POS);  
	close(socketMIRAM);
}

void serializarYMandarInicioTareaNormal(uint32_t tid, char * stringTareas){
	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));   //HECHO

	int tamanioTarea = strlen(stringTareas) + 1;

	buffer-> size = sizeof(int) + sizeof(uint32_t) + tamanioTarea;

	void* stream = malloc(buffer->size); //HECHO

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tamanioTarea), sizeof(int));
	offset += sizeof(int);

	memcpy(stream+offset, stringTareas, tamanioTarea);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, INICIO_TAREA_NORMAL);
	close(socket);
	//free(stringTareas);
}

void serializarYMandarFinalizacionTarea(uint32_t tid, char * nombreTarea){
	int socket = conectarImongo();

	int tamanioNombreTarea = strlen(nombreTarea) + 1;

	t_buffer* buffer = malloc(sizeof(t_buffer));   // HECHO

	buffer-> size = tamanioNombreTarea + sizeof(int) + sizeof(uint32_t);

	void* stream = malloc(buffer->size); //HECHO

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &tamanioNombreTarea, sizeof(tamanioNombreTarea));
	offset += sizeof(int);

	memcpy(stream+offset, nombreTarea, tamanioNombreTarea);
	offset += tamanioNombreTarea;

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, FINALIZO_TAREA);
	close(socket);
}

void serializarYMandarElegidoDelSabotaje(uint32_t tid){
	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));		//HECHO

	buffer-> size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, INICIAR_FSCK);
	close(socket);
}

void serializarYMandarPosicionBitacora(uint32_t tid, uint32_t posxV, uint32_t posyV, uint32_t posxN, uint32_t posyN){
	
	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));  //HECHO

	buffer-> size = 5 * sizeof(uint32_t);

	void* stream = malloc(buffer->size); //HECHO

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(posxV), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(posyV), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(posxN), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(posyN), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, NUEVA_POSICION);
	close(socket);
}

void serializarYMandarPedidoDeBitacora(uint32_t tid){

	int socket = conectarImongo();

	t_buffer* buffer = malloc(sizeof(t_buffer));  //HECHO

	buffer-> size = sizeof(uint32_t); 

	void* stream = malloc(buffer->size);

	memcpy(stream, &(tid), sizeof(uint32_t));
	
	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, BITACORA);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	recv(socket, &(paquete->header) , sizeof(int), 0);

	recv(socket,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	recv(socket,paquete->buffer->stream,paquete->buffer->size,0);

	void * stream2 = paquete->buffer->stream;

	int tamanioBitacora;
	memcpy(&(tamanioBitacora), stream2, sizeof(int));
	stream2 += sizeof(int);
	
	char* bitacora = malloc(tamanioBitacora ); // aca habia un + 2 adentro del malloc
	memcpy(bitacora, stream2, tamanioBitacora);
	stream2 += tamanioBitacora;

	log_info(loggerDiscordiador,"%d",tamanioBitacora);

	log_info(loggerDiscordiador,"\nBitacora tripulante %d: \n %s",tid,bitacora);

	/*
	close(socket);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(bitacora);
	free(paquete);*/ 
}

void serializarYMandarNuevoEstado(TCB_DISCORDIADOR * tripulante){
	int socket = conectarMiRAM();

	t_buffer* buffer = malloc(sizeof(t_buffer));  //HECHO

	buffer-> size = 2 * sizeof(uint32_t) + sizeof(char);

	void* stream = malloc(buffer->size); //HECHO

	int offset = 0;

	memcpy(stream+offset, &(tripulante->tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(tripulante->pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &( tripulante->estado ), sizeof(char));
	offset += sizeof(char);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, ACTUALIZAR_ESTADO);
	close(socket);

}

void serializaYMandarExpulsado(uint32_t tid, uint32_t pid ){
	
	int socket = conectarMiRAM();

	t_buffer* buffer = malloc(sizeof(t_buffer));  //HECHO

	buffer-> size = 2 * sizeof(uint32_t);

	void* stream = malloc(buffer->size); //HECHO

	int offset = 0;

	memcpy(stream+offset, &(tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream+offset, &(pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	mandarPaqueteSerializado(buffer, socket, EXPULSAR_TRIPULANTE);
	close(socket);
}

//-----------------------------SABOTAJES---------------------------------------------------------------------------------------------------

void atenderImongo(){

	while(1){
 
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	int headerRECV = recv(socketParaSabotajes, &(paquete->header) , sizeof(int), 0);
	if(headerRECV) { log_info(loggerDiscordiador, "Recibi header: %d\n", paquete->header);} else{ log_error(loggerDiscordiador, "No se pudo recibir el header");}
	
	int BUFFER_RECV  = 0;
	
	int statusTamanioBuffer = recv(socketParaSabotajes,&(paquete-> buffer-> size), sizeof(uint32_t), 0);
	if(! statusTamanioBuffer){ log_error(loggerDiscordiador, "No se pudo recibir el tamanio del buffer ");}

	paquete->buffer->stream = malloc(paquete->buffer->size);

	BUFFER_RECV = recv(socketParaSabotajes,paquete->buffer->stream,paquete->buffer->size, MSG_WAITALL); // se guardan las tareas en stream

	if(! BUFFER_RECV){ log_error(loggerDiscordiador,"No se pudo recibir el buffer");}

	if(paquete->header == ALERTA_DE_SABOTAJE){

		cantidadDeSabotajes++;
		haySabotaje = true;

		void* stream = malloc(paquete->buffer->size);
		stream = paquete->buffer->stream;

		uint32_t posX =0;
		uint32_t posY=0;
 

		memcpy(&(posX), stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
        log_info(loggerDiscordiador, "la posicion en x %d", posX);

		memcpy(&(posY), stream, sizeof(uint32_t) );
		stream += sizeof(uint32_t);
		log_info(loggerDiscordiador, "la posicion en y %d", posY);
		cambiarEstadosABloqueados();

		TCB_DISCORDIADOR * tripulante = tripulanteMasCercano(posX, posY);

		uint32_t  posXTripulante = tripulante->posicionX; 
		uint32_t  posyTripulante = tripulante->posicionY; 

		log_info(loggerDiscordiador, "Tripulante %d nos va a salvar del sabotaje en %d|%d !!", tripulante->tid, posX, posY);

		cambiarDeEstado(tripulante,'S');

		trasladarseADuranteSabotaje(posX, posY, tripulante);

		serializarYMandarElegidoDelSabotaje(tripulante->tid);

		sleep(tiempoSabotaje);

		log_info(loggerDiscordiador, "Tripulante %d nos ha salvado!!", tripulante->tid);

		trasladarseADuranteSabotaje(posXTripulante, posyTripulante, tripulante);

		cantidadDeSabotajes--;
		
		if(cantidadDeSabotajes == 0){
		volverAEstadosPostSabotaje();
		haySabotaje = false;

		for(int r = 0 ; r < gradoMultitarea ; r ++){
			sem_post(&semaforoSabotaje);
		}		
		}
	
	 }
		// PONER FREES?????????????
	}

}

TCB_DISCORDIADOR * tripulanteMasCercano(uint32_t posX, uint32_t posY){

	uint32_t distanciaMenor;
	uint32_t tidMasCercano;
	
	for(int a = 0 ; a < list_size(listaBloqueadosSabotaje) ; a++){
		
		TCB_DISCORDIADOR * tripulante = list_get(listaBloqueadosSabotaje,a); 
		uint32_t distanciaASabotaje = abs(posX - tripulante->posicionX) + abs(posY - tripulante->posicionY);
		log_info(loggerDiscordiador,"Tripulante %d se encuentra en %d|%d", tripulante->tid, tripulante->posicionX, tripulante->posicionY);

		if(a == 0){
			distanciaMenor = distanciaASabotaje; //Al primero lo asigna asi
			tidMasCercano = tripulante->tid;
		}

		if(distanciaASabotaje < distanciaMenor){
			distanciaMenor = distanciaASabotaje;
			tidMasCercano = tripulante->tid;
		}


	}

	bool coincideID(TCB_DISCORDIADOR * tripulante){
				return tripulante->tid == tidMasCercano;
			}
	
	TCB_DISCORDIADOR * tripulante = list_find(listaBloqueadosSabotaje, coincideID);
	list_remove_by_condition(listaBloqueadosSabotaje, coincideID); 

	return tripulante;

}

//-----------------------------ESTADOS---------------------------------------------------------------------------------------------------

void cambiarDeEstado(TCB_DISCORDIADOR * tripulante, char estado){

	if(! haySabotaje){
	salirDeListaEstado(tripulante);
	}

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
		sem_wait(&contadorBloqueados);
		cantidadBloqueados++;
		sem_post(&contadorBloqueados);
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

		serializarYMandarNuevoEstado(tripulante);
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

void cambiarEstadosABloqueados(){

	t_list * listaTrabajando2 = list_duplicate(listaTrabajando);
	list_sort(listaTrabajando2 , tripulanteConIDMasChico );

	t_list * listaReady2 = list_duplicate(listaReady);
	list_sort(listaReady2 , tripulanteConIDMasChico );

	for(int p = 0 ; p < list_size(listaTrabajando) ; p++){
		TCB_DISCORDIADOR * tripulante = list_get(listaTrabajando2, p);
		tripulante->estado = 'B';
		serializarYMandarNuevoEstado(tripulante);
		list_add(listaBloqueadosSabotaje, tripulante);
		log_info(loggerDiscordiador, "Tripulante %d cambio su estado a %c", tripulante->tid, tripulante->estado);

	}

	for(int j = 0 ; j < list_size(listaReady) ; j++){
		TCB_DISCORDIADOR * tripulante = list_get(listaReady2, j);
		tripulante->estado = 'B';
		serializarYMandarNuevoEstado(tripulante);
		list_add(listaBloqueadosSabotaje, tripulante);
		log_info(loggerDiscordiador, "Tripulante %d cambio su estado a %c", tripulante->tid, tripulante->estado);

	}
}

bool tripulanteConIDMasChico(TCB_DISCORDIADOR* tripulante1, TCB_DISCORDIADOR* tripulante2){
	if( tripulante1->tid < tripulante2->tid){
		return true;
	}
	return false;
}

void volverAEstadosPostSabotaje(){

	for(int p = 0 ; p < list_size(listaTrabajando) ; p++){
		TCB_DISCORDIADOR * tripulante = list_get(listaTrabajando, p);
		tripulante->estado = 'E';
		serializarYMandarNuevoEstado(tripulante);
	}

	for(int j = 0 ; j < list_size(listaReady) ; j++){
		TCB_DISCORDIADOR * tripulante = list_get(listaReady, j);
		tripulante->estado = 'R';
		serializarYMandarNuevoEstado(tripulante);
	}

}

void ponerReadyNuevosTripulantes(){

	for(int u = 0 ; u < list_size(listaNuevos); u++){

		TCB_DISCORDIADOR * tripulante = list_get(listaNuevos, 0);
		cambiarDeEstado(tripulante, 'R');
	

	}

} 
