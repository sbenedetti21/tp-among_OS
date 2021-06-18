#include "imongo.h"

int main(int argc, char ** argv){
	loggerImongoStore = log_create("imongo.log", "imongo.c", 0, LOG_LEVEL_INFO); 

	leerConfig();

	char * valorInicio = readline("Ingrese 0 si se quiere crear un FileSystem limpio, cualquier otro caracter si se busca leer un FileSystem existente\n");
	int valor = atoi(valorInicio);
	free(valorInicio);

	inicializarFileSystem(valor);
	
	//Copia del mapeo del blocks
	mapBlocksCopia = malloc(tamanioBlocks);
	memcpy(mapBlocksCopia, mapBlocks, tamanioBlocks);
	
	//Inicializacion de los semaforos
	sem_init(&semaforoBloques, 0,  1 ); 
	sem_init(&semaforoArchivoRecurso, 0, 1);
	sem_init(&semaforoOxigeno, 0, 1);
	sem_init(&semaforoBasura, 0, 1);
	sem_init(&semaforoComida, 0, 1);
	
	//Inicio de servidor
	pthread_t servidor;
    pthread_create(&servidor, NULL, servidorPrincipal, NULL);
    pthread_join(servidor, NULL);

	//Necesario al finalizar
	munmap(mapBlocks,tamanioBlocks);
	close(archivoBlocks);
	guardarBitMap();
	liberarBitMap(punteroBitmap);
	sem_destroy(&semaforoArchivoRecurso);
	sem_destroy(&semaforoBloques);
	sem_destroy(&semaforoOxigeno);
	sem_destroy(&semaforoBasura);
	sem_destroy(&semaforoComida);
	free(mapBlocksCopia);
	return 0;
}
 
//-----------------------------------------------------------------------------------------------------//
//-------------------------------------------- LEE CONFIG --------------------------------------------//
void leerConfig(){ 
	t_config * config = config_create("./cfg/imongo.config");
	
	puntoDeMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	puertoImongoStore = config_get_string_value(config, "PUERTO");
	tiempoDeSinc = config_get_int_value(config, "TIEMPO_SINCRONIZACION");
	tamanioDeBloque = config_get_int_value(config, "BLOCK_SIZE");
    cantidadDeBloques = config_get_int_value(config, "CANTIDAD_BLOCKS");
}
//----------------------------------------------------------------------------------------------------//
//----------------------------------- ARCHIVO BLOCKS Y SUPERBLOQUE -----------------------------------//
void crearBlocks(){
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	FILE *archivoCrearBlocks = fopen(ubicacionBlocks, "w");
	tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	ftruncate(fileno(archivoCrearBlocks), tamanioBlocks);
	fclose(archivoCrearBlocks);
	mapearBlocks();
}

void mapearBlocks(){
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	archivoBlocks = open(ubicacionBlocks, O_RDWR , S_IRUSR | S_IWUSR);
	mapBlocks = mmap(NULL, tamanioBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBlocks,0);
	free(ubicacionBlocks);
}

void crearSuperBloque(){
	ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"w");
	
	fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);
	
	fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);

 	crearBitMap();
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);
}
//---------------------------------------------------------------------------------------------------//
//---------------------------------------- USO DEL FILESYSTEM ---------------------------------------//
void crearFileSystem(){
	mkdir(puntoDeMontaje,0777);
	crearSuperBloque();
	crearBlocks();
	mkdir(string_from_format("%s/Files",puntoDeMontaje),0777);
	mkdir(string_from_format("%s/Files/Bitacoras",puntoDeMontaje),0777);
}

void leerFileSystem(){
	ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
//superBocke	
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r");
	
	fread(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);
	
	fread(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
	fflush(superBloque);
//bitmap
	crearBitMap();
 	fread(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);

//Blocks
	mapearBlocks();
}

void inicializarFileSystem(int valorRespuesta){
	struct stat st = {0};
	switch (valorRespuesta)
	{	
	case 0:
		borrarFileSystem(puntoDeMontaje);
		crearFileSystem();
		log_info(loggerImongoStore, "---------CREO FILESYSTEM----------");
		printf("Se creo el FileSystem\n");
		break;
	default:
		if(stat(puntoDeMontaje,&st) == -1){
			crearFileSystem();
			log_info(loggerImongoStore, "---------CREO FILESYSTEM----------");
			printf("Se creo el FileSystem\n");
		} else{	
			leerFileSystem();
			log_info(loggerImongoStore, "---------LEYO FILESYSTEM----------");
			printf("Se leyo el FileSystem existente\n");
		}
		break;
	}
}

void borrarFileSystem(const char *path){

 struct dirent *de;
        char fname[300];
        DIR *dr = opendir(path);
        if(dr == NULL)
        {
            printf("La carpeta FileSystem no existe\n");
            return;
        }
        while((de = readdir(dr)) != NULL)
        {
            int ret = -1;
            struct stat statbuf;
            sprintf(fname,"%s/%s",path,de->d_name);
            if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
                        continue;
            if(!stat(fname, &statbuf))
            {
                if(S_ISDIR(statbuf.st_mode))
                {
                   ret = unlinkat(dirfd(dr),fname,AT_REMOVEDIR);

                    if(ret != 0)
                    {
                       borrarFileSystem(fname);
                       ret = unlinkat(dirfd(dr),fname,AT_REMOVEDIR);
                    }
                }
                else
                {
                   unlink(fname);
                }
            }
        }
        closedir(dr);
		rmdir(path);
}
//----------------------------------------------------------------------------------------------------//
//------------------------------------------ USO DEL BITMAP ------------------------------------------//
void crearBitMap(){
	size_t sizeBitMap = cantidadDeBloques / 8; 
	char * bitMap = (char *) malloc(sizeBitMap);
	punteroBitmap = bitarray_create(bitMap, sizeBitMap);
	
	for(int i = 0; i<cantidadDeBloques; i++){
		bitarray_clean_bit(punteroBitmap,i);
	}
}

void guardarBitMap(){
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");
	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);
	fclose(superBloque);
}

void leerBitMap(){
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");

	fseek(superBloque, sizeof(uint32_t) * 2, SEEK_SET);
	crearBitMap();
 	fread(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);

	fclose(superBloque);
}

int bloqueLibreBitMap(){
	sem_wait(&semaforoBloques);
	int proximoBlock = 0;
	while(bitarray_test_bit(punteroBitmap,proximoBlock))
			proximoBlock++;
	bitarray_set_bit(punteroBitmap,proximoBlock);
	
	guardarBitMap();
	sem_post(&semaforoBloques);

	return proximoBlock;
}

void liberarBloqueBitMap(int bloquePorLiberar){
	sem_wait(&semaforoBloques);
	bitarray_clean_bit(punteroBitmap,bloquePorLiberar);
	sem_post(&semaforoBloques);
}

void liberarBitMap(){
	free(punteroBitmap->bitarray);
 	bitarray_destroy(punteroBitmap);
}
//---------------------------------------------------------------------------------------------------//
//--------------------------------- CODIGO DE LOS FILES Y BITACORAS ----------------------------------//
int leerUltimoBloque(t_config * configGeneral){

	char *listaBlocks = config_get_string_value(configGeneral,"BLOCKS");
	int proximoBlock;
	
	if(listaBlocks[1]!=']'){ 
		int indiceFinal = strlen(listaBlocks);
		while(listaBlocks[indiceFinal]!=',' && indiceFinal>0)
			indiceFinal--;
		proximoBlock = atoi(string_substring_from(listaBlocks,indiceFinal+1)) - 1;
	}else{
		proximoBlock = bloqueLibreBitMap();
		agregarBloqueArchivo(proximoBlock, configGeneral);
	}
	return proximoBlock;
}

void agregarBloqueArchivo(int nuevoBloque, t_config * configGeneral){
	if(config_has_property(configGeneral,"BLOCK_COUNT")){
		int nuevaCantidadBloques = config_get_int_value(configGeneral,"BLOCK_COUNT");
		nuevaCantidadBloques++;
		config_set_value(configGeneral,"BLOCK_COUNT",string_itoa(nuevaCantidadBloques));
	}

	char *listaBlocks = config_get_string_value(configGeneral,"BLOCKS");
	char *nuevaListablocks = string_substring_until(listaBlocks,strlen(listaBlocks)-1);

	if(listaBlocks[1]!=']')
		string_append(&nuevaListablocks, ",");
	
	string_append(&nuevaListablocks, string_itoa(nuevoBloque + 1));
	string_append(&nuevaListablocks, "]");
	config_set_value(configGeneral,"BLOCKS",nuevaListablocks);
	config_save(configGeneral);
}

void agregarSizeArchivo(t_config * configGeneral, int nuevaSize){
	int archivoSize = config_get_int_value(configGeneral,"SIZE");
	archivoSize+=nuevaSize;
	config_set_value(configGeneral, "SIZE", string_itoa(archivoSize));
	config_save(configGeneral);
}
//---------------------------------------------------------------------------------------------------//
//------------------------------------ CREARCION ARCHIVO BITACORA -----------------------------------//
t_config *crarArchivoBitacora(int idTripulante){
	char * ubicacionArchivoBitacora = string_from_format("%s/Files/Bitacoras/Tripulante%d.ims",puntoDeMontaje,idTripulante);
	t_config * configBitacora;
	
	if(access(ubicacionArchivoBitacora, F_OK )){
		FILE *fileBitacora= fopen(ubicacionArchivoBitacora,"w");
		fclose(fileBitacora);	
		
		configBitacora = config_create(ubicacionArchivoBitacora);
	 	config_set_value(configBitacora,"SIZE",string_itoa(0));
	 	config_set_value(configBitacora,"BLOCKS","[]");
	 	config_save_in_file(configBitacora,ubicacionArchivoBitacora);
	} else {
		configBitacora = config_create(ubicacionArchivoBitacora);
	}

	return configBitacora;
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------- CODIGO DE LOS BLOCKS DE BITACORAS --------------------------------//
void llenarBlocksBitcoras(char * informacionDeLenado, t_config * configBitacora){
	int cantLlenado = 0;
	int cantFaltante = strlen(informacionDeLenado);
	char *copiaInformacion = informacionDeLenado;

	int proximoBlock = leerUltimoBloque(configBitacora);

	int marcador = config_get_int_value(configBitacora,"SIZE") % tamanioDeBloque;
	int cantidadLibreBlock = tamanioDeBloque-marcador;
	marcador += proximoBlock * tamanioDeBloque;

	agregarSizeArchivo(configBitacora, cantFaltante);

	while(cantFaltante>0){
		if(cantidadLibreBlock>cantFaltante){
			cantidadLibreBlock=cantFaltante;
			memcpy(&(mapBlocksCopia[marcador]), copiaInformacion, cantidadLibreBlock);
		} else {
			memcpy(&(mapBlocksCopia[marcador]), string_substring_until(copiaInformacion,cantidadLibreBlock), cantidadLibreBlock);
			copiaInformacion = string_substring_from(copiaInformacion,cantidadLibreBlock);
		}
		
		cantFaltante-=cantidadLibreBlock;

		if(cantFaltante>0){
			proximoBlock = bloqueLibreBitMap();
			agregarBloqueArchivo(proximoBlock, configBitacora);
			marcador = proximoBlock * tamanioDeBloque;
			cantidadLibreBlock=tamanioDeBloque;
		}
	}

	free(copiaInformacion);
}
//--------------------------------------------------------------------------------------------------//
//------------------------------ SEMAFOROS PARA LOS FILES DE RECURSOS ------------------------------//
void semaforoEsperaRecurso(char *recurso){
	if(!strcmp(recurso,"Oxigeno")){
		sem_wait(&semaforoOxigeno);
	} else if(!strcmp(recurso,"Basura")){
		sem_wait(&semaforoBasura);
	} else {
		sem_wait(&semaforoComida);
	}
}

void semaforoListoRecurso(char *recurso){
	if(!strcmp(recurso,"Oxigeno")){
		sem_post(&semaforoOxigeno);
	} else if(!strcmp(recurso,"Basura")){
		sem_post(&semaforoBasura);
	} else {
		sem_post(&semaforoComida);
	}
}
//---------------------------------------------------------------------------------------------------//
//--------------------------------- CODIGO DE LOS FILES DE RECURSOS ---------------------------------//
void creacionFileRecurso(char caracterLlenado, char *ubicacionArchivoRecurso){
	FILE *fileRecurso = fopen(ubicacionArchivoRecurso,"w");
	fclose(fileRecurso);

	t_config *configRecurso = config_create(ubicacionArchivoRecurso);
	 config_set_value(configRecurso,"SIZE",string_itoa(0));
	 config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(0));
	 config_set_value(configRecurso,"BLOCKS","[]");
	 config_set_value(configRecurso,"CARACTER_LLENADO",string_repeat(caracterLlenado,1));
	 config_set_value(configRecurso,"MD5_ARCHIVO","");
	 config_save_in_file(configRecurso,ubicacionArchivoRecurso);
	 config_destroy(configRecurso);
}

void agregarBloqueFile(int nuevoBloque, char *recurso){
	semaforoEsperaRecurso(recurso);

	t_config *configRecurso = config_create(string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso));

	agregarBloqueArchivo(nuevoBloque, configRecurso);

	config_destroy(configRecurso);
	semaforoListoRecurso(recurso);
}

void agregarSizeFile(char *recurso, int nuevaSize){
	semaforoEsperaRecurso(recurso);
	
	t_config *configRecurso = config_create(string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso));

	agregarSizeArchivo(configRecurso, nuevaSize);
	
	config_destroy(configRecurso);

	semaforoListoRecurso(recurso);
}

int ultimoBloqueFile(char *recurso){
	semaforoEsperaRecurso(recurso);

	t_config *configRecurso = config_create(string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso));

	int proximoBlock = leerUltimoBloque(configRecurso);

	config_destroy(configRecurso);
	semaforoListoRecurso(recurso);

	return proximoBlock;
}

int conseguirSizeBlocks(char *recurso){
	semaforoEsperaRecurso(recurso);

	t_config *configRecurso = config_create(string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso));

	int sizeDeLosBlocks = config_get_int_value(configRecurso,"SIZE");

	config_destroy(configRecurso);
	semaforoListoRecurso(recurso);

	return sizeDeLosBlocks;
}

void borrarBloqueFile(char *recurso, int cantidadBorrar){
	semaforoEsperaRecurso(recurso);
	t_config *configRecurso = config_create(string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso));

	int nuevaCantidadBloques = config_get_int_value(configRecurso,"BLOCK_COUNT");
	nuevaCantidadBloques--;
	config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(nuevaCantidadBloques));

	char *listaBlocks = config_get_string_value(configRecurso,"BLOCKS");
	int indiceFinal = strlen(listaBlocks)-1;
	while(listaBlocks[indiceFinal]!=',' && indiceFinal>0)
			indiceFinal--;
	
	if(indiceFinal>0){
		listaBlocks = string_substring_until(listaBlocks, indiceFinal);
		string_append(&listaBlocks, "]");
	} else {
		listaBlocks = "[]";
	}
	config_set_value(configRecurso,"BLOCKS",listaBlocks);

	int sizeAcutal = config_get_int_value(configRecurso, "SIZE");
	sizeAcutal-=cantidadBorrar;
	config_set_value(configRecurso,"SIZE",string_itoa(sizeAcutal));

	config_save(configRecurso);
	config_destroy(configRecurso);
	free(listaBlocks);
	semaforoListoRecurso(recurso);
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------- CODIGO DE LOS BLOCKS DE RECURSOS ---------------------------------//
void llenarBlocksRecursos(char *recurso, int cantALlenar){
	int cantFaltante = cantALlenar;
	int proximoBlock = ultimoBloqueFile(recurso);

	int marcador = conseguirSizeBlocks(recurso) % tamanioDeBloque;
	int cantidadLibreBlock = tamanioDeBloque-marcador;
	marcador += proximoBlock * tamanioDeBloque;

	agregarSizeFile(recurso, cantALlenar);
	
	while(cantFaltante>0){
		char *caracteresLlenado;

		if(cantidadLibreBlock>cantFaltante)
			cantidadLibreBlock=cantFaltante;

		caracteresLlenado = string_repeat(recurso[0],cantidadLibreBlock);
		memcpy(&(mapBlocksCopia[marcador]), caracteresLlenado, cantidadLibreBlock);
		cantFaltante-=cantidadLibreBlock;

		if(cantFaltante>0){
			proximoBlock = bloqueLibreBitMap();
			agregarBloqueFile(proximoBlock, recurso);
			marcador = proximoBlock * tamanioDeBloque;
			cantidadLibreBlock=tamanioDeBloque;
		}
	}
}

void vaciarBlocksRecursos(char *recurso, int cantAVaciar){
	int cantFaltante = cantAVaciar;
	int proximoBlock;

	int cantidadEliminar= conseguirSizeBlocks(recurso) % tamanioDeBloque;
	
	while(cantFaltante>0){
		proximoBlock = ultimoBloqueFile(recurso);
		int marcador = proximoBlock * tamanioDeBloque;

		if(cantidadEliminar == 0){
			cantidadEliminar = tamanioDeBloque;
		}

		if(cantidadEliminar>cantFaltante){
			cantidadEliminar = cantFaltante;
			cantFaltante*=(-1);
			agregarSizeFile(recurso, cantFaltante);
		} else {
			cantFaltante-=cantidadEliminar;
			borrarBloqueFile(recurso, cantidadEliminar);
			liberarBloqueBitMap(proximoBlock);
		}

		char *stringVacio = NULL;
		stringVacio = (char*)calloc(cantidadEliminar, sizeof(char));
		memcpy(&(mapBlocksCopia[marcador]), stringVacio, cantidadEliminar);
		cantidadEliminar=0;
		free(stringVacio);
	}
}
//---------------------------------------------------------------------------------------------------//
//--------------------------------------- MENSAJES QUE RECIBE ---------------------------------------//
void generarRecurso(char *recurso, int cantidadALlenar, uint32_t idTripulante){
	log_info(loggerImongoStore,string_from_format("Tripulante %d genera %d de %s",idTripulante, cantidadALlenar, recurso));
	
	t_config *configBitacora = crarArchivoBitacora(idTripulante);
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea Generar %s\n",recurso),configBitacora);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);

	sem_wait(&semaforoArchivoRecurso);
	if(access(ubicacionArchivoRecurso, F_OK ))
		creacionFileRecurso(recurso[0], ubicacionArchivoRecurso);		
	sem_post(&semaforoArchivoRecurso);
	
	llenarBlocksRecursos(recurso, cantidadALlenar);
	llenarBlocksBitcoras(string_from_format("Se finaliza la tarea Generar %s\n",recurso),configBitacora);

	config_destroy(configBitacora);
	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,string_from_format("Se genero %d de %s",cantidadALlenar, recurso));
}

bool consumirRecurso(char *recurso, int cantidadAConsumir, uint32_t idTripulante){
	log_info(loggerImongoStore,string_from_format("Tripulante %d consume %d de %s",idTripulante,cantidadAConsumir,recurso));
	
	t_config *configBitacora = crarArchivoBitacora(idTripulante);
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea Consumir %s\n",recurso),configBitacora);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);
	t_config *configRecurso;

	if(access(ubicacionArchivoRecurso, F_OK )){
		log_info(loggerImongoStore,"No hay recurso que consumir");
		config_destroy(configBitacora);
		free(ubicacionArchivoRecurso);
		return false;
	}

	configRecurso = config_create(ubicacionArchivoRecurso);
	vaciarBlocksRecursos(recurso, cantidadAConsumir);

	llenarBlocksBitcoras(string_from_format("Se finaliza la tarea Consumir %s\n",recurso),configBitacora);

	config_destroy(configBitacora);
	config_destroy(configRecurso);
	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,string_from_format("Se consumio %d de %s",cantidadAConsumir,recurso));
	return true;
}

void descartarBasura(uint32_t idTripulante){
	log_info(loggerImongoStore,string_from_format("Tripulante %d descarta Basura",idTripulante));
	
	t_config *configBitacora = crarArchivoBitacora(idTripulante);
	llenarBlocksBitcoras("Comienza la ejecucion de la tarea Descartar Basura\n",configBitacora);

	char *ubicacionArchivoBasura = string_from_format("%s/Files/Basura.ims",puntoDeMontaje);

	t_config *configRecurso = config_create(ubicacionArchivoBasura);
	int cantidadBasura = config_get_int_value(configRecurso, "SIZE");

	vaciarBlocksRecursos("Basura", cantidadBasura);
	
	config_destroy(configRecurso);
	remove(ubicacionArchivoBasura);

	llenarBlocksBitcoras("Se finaliza la tarea Descartar Basura\n",configBitacora);

	free(ubicacionArchivoBasura);

	log_info(loggerImongoStore,"Se descarto la Basura");
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------------- SINCRONIZACION BLOCKS --------------------------------------//
void sincronizacionMapBlocks(){
	while(1){
		sleep(tiempoDeSinc);
		memcpy(mapBlocks, mapBlocksCopia, tamanioBlocks);
		msync(mapBlocks, tamanioBlocks, MS_SYNC);
	}
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------------- ATENDER DISCORDIADOR ---------------------------------------//
void servidorPrincipal() {
	int listeningSocket = crear_conexionServer(puertoImongoStore);
	int socketCliente;
		
	printf("Conectado con el servidor, esperando tripulantes\n");

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	pthread_t receptorDiscordiador;

	while(1){
		socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
		if(socketCliente == -1){
			printf("Error en la conexión");
		} else {
			pthread_create(&receptorDiscordiador, NULL, atenderDiscordiador, socketCliente);
		}
	}

	close(socketCliente);
	close(listeningSocket);   
}

void atenderDiscordiador(int socketCliente){
	printf("Esperando mensaje del Discordiador\n");
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	t_parametro * parametroS = malloc(sizeof(int));  

	int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);

	int tamanioPAQUETE_RECV = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	int PAQUETE_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size,0);

	memcpy(&(parametroS->parametro), paquete->buffer->stream, sizeof(int));
	paquete->buffer->stream += sizeof(int);

	memcpy(&(parametroS->tid), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	uint32_t *tid = malloc(sizeof(uint32_t));
	*tid = parametroS->tid;   // ID TRIPULANTE

	log_info(loggerImongoStore,string_from_format("cantidad de Parametros %d",parametroS->parametro));
	log_info(loggerImongoStore,string_from_format("Llego el tripulante %d",parametroS->tid));

	switch (paquete->header)
	{
	case GENERAR_OXIGENO:
		generarRecurso("Oxigeno",parametroS->parametro,*tid);
		break;
	case CONSUMIR_OXIGENO:
		consumirRecurso("Oxigeno",parametroS->parametro,*tid);
		break;
	case GENERAR_COMIDA:
		generarRecurso("Comida",parametroS->parametro,*tid);
		break;
	case CONSUMIR_COMIDA:
		consumirRecurso("Comida", parametroS->parametro,*tid);
		break;
	case GENERAR_BASURA:
		generarRecurso("Basura",parametroS->parametro,*tid);
		break;
	case DESCARTAR_BASURA:
		descartarBasura(*tid);
		break;

	default:
		break;
	}

	memcpy(mapBlocks, mapBlocksCopia, tamanioBlocks);
	msync(mapBlocks, tamanioBlocks, MS_SYNC);
}