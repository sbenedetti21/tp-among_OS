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

	//pruebaDeSabotaje();
	//sabotajeSuperBloque();
	//conseguirBitacora(1);
	//Inicio de servidor
	pthread_t servidor;
    pthread_create(&servidor, NULL, servidorPrincipal, NULL);
    pthread_join(servidor, NULL);

	//Necesario al finalizar
	munmap(mapSuperBloque,tamanioBlocks);
	close(archivoSuperBloque);
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
}

void mapearBlocks(){
	char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	archivoBlocks = open(ubicacionBlocks, O_RDWR , S_IRUSR | S_IWUSR);
	mapBlocks = mmap(NULL, tamanioBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBlocks,0);
	free(ubicacionBlocks);
}

void crearSuperBloque(){	
	char *ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE *superBloque = fopen(ubicacionSuperBloque, "w");
	ftruncate(fileno(superBloque), tamanioSuperBloqueBlocks);
	fclose(superBloque);
	
	mapearSuperBloque();

	int marcador = 0;
	memcpy(&(mapSuperBloque[marcador]),&tamanioDeBloque, sizeof(uint32_t));
	marcador+=sizeof(uint32_t);
	
	memcpy(&(mapSuperBloque[marcador]), &cantidadDeBloques, sizeof(uint32_t));
	marcador+=sizeof(uint32_t);
	
	size_t tamanioBitMap = cantidadDeBloques / 8;
	crearBitMap();
	memcpy(&(mapSuperBloque[marcador]), punteroBitmap->bitarray, tamanioBitMap);

	msync(mapSuperBloque, tamanioSuperBloqueBlocks, MS_SYNC);

	tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
}

void leerSuperBloque(){
	int marcador = 0;
	memcpy(&tamanioDeBloque, &(mapSuperBloque[marcador]), sizeof(uint32_t));
	marcador+=sizeof(uint32_t);
	
	memcpy(&cantidadDeBloques, &(mapSuperBloque[marcador]), sizeof(uint32_t));
	marcador+=sizeof(uint32_t);
	
	crearBitMap();
	memcpy(punteroBitmap->bitarray, &(mapSuperBloque[marcador]), tamanioBitMap);

	tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
}

void mapearSuperBloque(){
	char *ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);

	archivoSuperBloque = open(ubicacionSuperBloque, O_RDWR, S_IRUSR | S_IWUSR);
	mapSuperBloque = mmap(NULL, tamanioSuperBloqueBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoSuperBloque,0);

	free(ubicacionSuperBloque);
}

//---------------------------------------------------------------------------------------------------//
//---------------------------------------- USO DEL FILESYSTEM ---------------------------------------//
void crearFileSystem(){
	mkdir(puntoDeMontaje,0777);

	tamanioBitMap = cantidadDeBloques / 8;
	tamanioSuperBloqueBlocks = sizeof(uint32_t)*2 + tamanioBitMap;
	crearSuperBloque();

	crearBlocks();
	mkdir(string_from_format("%s/Files",puntoDeMontaje),0777);
	mkdir(string_from_format("%s/Files/Bitacoras",puntoDeMontaje),0777);
}	

void leerFileSystem(){
//superBocke	
	tamanioBitMap = cantidadDeBloques / 8;
	tamanioSuperBloqueBlocks = sizeof(uint32_t)*2 + tamanioBitMap;

	mapearSuperBloque();
	leerSuperBloque();
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

	mapearBlocks();
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
	char * bitMap = (char *) malloc(tamanioBitMap);
	punteroBitmap = bitarray_create(bitMap, tamanioBitMap);
	
	for(int i = 0; i<cantidadDeBloques; i++){
		bitarray_clean_bit(punteroBitmap,i);
	}
}

void guardarBitMap(){
	sem_wait(&semaforoBloques);

	int marcador = sizeof(uint32_t) * 2;
	memcpy(&(mapSuperBloque[marcador]), punteroBitmap->bitarray, punteroBitmap->size);
	msync(mapSuperBloque, tamanioBlocks, MS_SYNC);
	
	sem_post(&semaforoBloques);
}

int bloqueLibreBitMap(){
	sem_wait(&semaforoBloques);
	int proximoBlock = 0;
	while(bitarray_test_bit(punteroBitmap,proximoBlock))
			proximoBlock++;
	bitarray_set_bit(punteroBitmap,proximoBlock);
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
	config_set_value(configGeneral, "SIZE", string_itoa(nuevaSize));
	config_save(configGeneral);
}
//---------------------------------------------------------------------------------------------------//
//------------------------------------ CREARCION ARCHIVO BITACORA -----------------------------------//
t_config *crearArchivoBitacora(int idTripulante){
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
void llenarBlocksBitcoras(char *informacionDeLenado, uint32_t idTripulante){
	t_config *configBitacora = crearArchivoBitacora(idTripulante);
	int cantLlenado = 0;
	int cantFaltante = strlen(informacionDeLenado);
	
	int proximoBlock = leerUltimoBloque(configBitacora);
	int sizeBitacora = config_get_int_value(configBitacora,"SIZE");
	
	int marcador = sizeBitacora % tamanioDeBloque;
	int cantidadLibreBlock = tamanioDeBloque-marcador;
	
	marcador += proximoBlock * tamanioDeBloque;
	sizeBitacora += cantFaltante;
	while(cantFaltante>0){
		char *stringPorEscribir;

		if(cantidadLibreBlock>cantFaltante){
			cantidadLibreBlock=cantFaltante;
			stringPorEscribir=string_substring_from(informacionDeLenado,cantLlenado);
		} else {
			stringPorEscribir = string_substring(informacionDeLenado,cantLlenado,cantidadLibreBlock);
		}
		
		memcpy(&(mapBlocksCopia[marcador]), stringPorEscribir, cantidadLibreBlock);
		cantLlenado+=cantidadLibreBlock;

		cantFaltante-=cantidadLibreBlock;

		if(cantFaltante>0){
			proximoBlock = bloqueLibreBitMap();
			agregarBloqueArchivo(proximoBlock, configBitacora);
			marcador = proximoBlock * tamanioDeBloque;
			cantidadLibreBlock=tamanioDeBloque;
		}

		free(stringPorEscribir);
	}
	agregarSizeArchivo(configBitacora, sizeBitacora);
	config_save(configBitacora);
	config_destroy(configBitacora);
}
//--------------------------------------------------------------------------------------------------//
//------------------------------ SEMAFOROS PARA LOS FILES DE RECURSOS ------------------------------//
void semaforoEsperaRecurso(char *recurso){
	if(!strcmp(recurso,"Oxigeno")){
		sem_wait(&semaforoOxigeno);
	} else if(!strcmp(recurso,"Basura")){
		sem_wait(&semaforoBasura);
	} else if(!strcmp(recurso,"Comida")){
		sem_wait(&semaforoComida);
	}
}

void semaforoListoRecurso(char *recurso){
	if(!strcmp(recurso,"Oxigeno")){
		sem_post(&semaforoOxigeno);
	} else if(!strcmp(recurso,"Basura")){
		sem_post(&semaforoBasura);
	} else if(!strcmp(recurso,"Comida")){
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
	while(listaBlocks[indiceFinal]!=',' && indiceFinal>1)
			indiceFinal--;
	
	
	listaBlocks = string_substring_until(listaBlocks, indiceFinal);
	string_append(&listaBlocks, "]");
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

	int sizeRecurso = conseguirSizeBlocks(recurso);

	int marcador = sizeRecurso % tamanioDeBloque;
	int cantidadLibreBlock = tamanioDeBloque-marcador;
	
	if(cantidadLibreBlock==tamanioDeBloque && sizeRecurso>0){
		proximoBlock = bloqueLibreBitMap();
		agregarBloqueFile(proximoBlock, recurso);
	}

	marcador += proximoBlock * tamanioDeBloque;

	sizeRecurso+=cantALlenar;
	agregarSizeFile(recurso, sizeRecurso);
	
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
	int sizeRecurso = conseguirSizeBlocks(recurso);
	int cantidadEliminar= sizeRecurso % tamanioDeBloque;
	
	while(cantFaltante>0){
		proximoBlock = ultimoBloqueFile(recurso);
		int marcador = proximoBlock * tamanioDeBloque;
		

		if(cantidadEliminar == 0){
			cantidadEliminar = tamanioDeBloque;
		}

		marcador += cantidadEliminar;
		
		if(cantidadEliminar>cantFaltante){
			cantidadEliminar = cantFaltante;
			sizeRecurso-=cantFaltante;
			agregarSizeFile(recurso, sizeRecurso);
		} else {
			borrarBloqueFile(recurso, cantidadEliminar);
			liberarBloqueBitMap(proximoBlock);
		}

		marcador -= cantidadEliminar;
		cantFaltante-=cantidadEliminar;

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
	mandarMensajeEnLog(string_from_format("Tripulante %d genera %d de %s",idTripulante, cantidadALlenar, recurso));
	
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea Generar %s\n",recurso),idTripulante);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);

	sem_wait(&semaforoArchivoRecurso);
	if(access(ubicacionArchivoRecurso, F_OK ))
		creacionFileRecurso(recurso[0], ubicacionArchivoRecurso);		
	sem_post(&semaforoArchivoRecurso);
	
	llenarBlocksRecursos(recurso, cantidadALlenar);
	llenarBlocksBitcoras(string_from_format("Se finaliza la tarea Generar %s\n",recurso),idTripulante);

	free(ubicacionArchivoRecurso);

	mandarMensajeEnLog(string_from_format("Tripulante %d termino de generar %d de %s",idTripulante,cantidadALlenar, recurso));
}

bool consumirRecurso(char *recurso, int cantidadAConsumir, uint32_t idTripulante){
	mandarMensajeEnLog(string_from_format("Tripulante %d consume %d de %s",idTripulante,cantidadAConsumir,recurso));


	char *comienzaLaTarea = string_from_format("Comienza la ejecucion de la tarea Consumir %s\n",recurso);
	llenarBlocksBitcoras(comienzaLaTarea,idTripulante);
	free(comienzaLaTarea);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);

	if(access(ubicacionArchivoRecurso, F_OK )){
		log_info(loggerImongoStore,"No hay recurso que consumir");
		free(ubicacionArchivoRecurso);
		return false;
	}

	vaciarBlocksRecursos(recurso, cantidadAConsumir);

	char *terminaLaTarea = string_from_format("Se finaliza la tarea Consumir %s\n",recurso);
	llenarBlocksBitcoras(terminaLaTarea,idTripulante);
	free(terminaLaTarea);

	free(ubicacionArchivoRecurso);

	mandarMensajeEnLog(string_from_format("Se consumio %d de %s",cantidadAConsumir,recurso));
	return true;
}

void descartarBasura(uint32_t idTripulante){
	mandarMensajeEnLog(string_from_format("Tripulante %d descarta Basura",idTripulante));
	
	llenarBlocksBitcoras("Comienza la ejecucion de la tarea Descartar Basura\n",idTripulante);
	char *ubicacionArchivoBasura = string_from_format("%s/Files/Basura.ims",puntoDeMontaje);

	if(access(ubicacionArchivoBasura, F_OK )){
		log_info(loggerImongoStore,"No hay recurso que consumir");
		free(ubicacionArchivoBasura);
	} else {
		t_config *configRecurso = config_create(ubicacionArchivoBasura);
		int cantidadBasura = config_get_int_value(configRecurso, "SIZE");

		vaciarBlocksRecursos("Basura", cantidadBasura);

		config_destroy(configRecurso);
		remove(ubicacionArchivoBasura);

		llenarBlocksBitcoras("Se finaliza la tarea Descartar Basura\n",idTripulante);

		free(ubicacionArchivoBasura);

		log_info(loggerImongoStore,"Se descarto la Basura");
	}
}

void tareaTripulante(char *tarea, uint32_t idTripulante){	
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea %s\n",tarea),idTripulante);
	
	llenarBlocksBitcoras(string_from_format("Se finaliza la tarea %s\n",tarea),idTripulante);
}

void movimientoTripulante(uint32_t idTripulante){
	t_config *configBitacora = crearArchivoBitacora(idTripulante);
	llenarBlocksBitcoras(string_from_format("Se mueve de X|Y a X’|Y’"),configBitacora);
	config_destroy(configBitacora);
}

char *conseguirStringBlocks(int bloqueBuscado){
	char *bloqueStringBuscado = malloc(tamanioDeBloque);
	int marcador = bloqueBuscado * tamanioDeBloque;
	memcpy(bloqueStringBuscado,&(mapBlocksCopia[marcador]), tamanioDeBloque);

	return bloqueStringBuscado;
}

void conseguirBitacora(uint32_t idTripulante){
	mandarMensajeEnLog(string_from_format("Consiguiendo bitacora del Tripulante %d",idTripulante));
	char *ubicacionBitacoraID = string_from_format("%s/Files/Bitacoras/Tripulante%d.ims",puntoDeMontaje,idTripulante);

	t_config *configBitacora = config_create(ubicacionBitacoraID);
	int sizeAcutal = config_get_int_value(configBitacora, "SIZE");

	char *stringBitacora = malloc(sizeAcutal);
	int marcador = 0;
	char *listaBlocks = config_get_string_value(configBitacora,"BLOCKS");
	
	if(listaBlocks[1]!=']'){
		int inicio = 1;
		while(inicio<strlen(listaBlocks)){
			int tamanio = 0;	
			while(listaBlocks[inicio+tamanio]!=',' && listaBlocks[inicio+tamanio]!=']')
				tamanio++;

			char *proximoBlockString = string_substring(listaBlocks,inicio,tamanio);
			int proximoBlock = atoi(proximoBlockString) - 1;

			char *stringBuscado = conseguirStringBlocks(proximoBlock);

			memcpy(&(stringBitacora[marcador]),stringBuscado, tamanioDeBloque);


			marcador+=tamanioDeBloque;
			inicio+=tamanio;
			inicio++;

			//free(proximoBlockString);
			//free(stringBuscado);
		}
	}else{
		mandarMensajeEnLog(string_from_format("No tiene informacion en la bitacora del Tripulante %d",idTripulante));
	}
	mandarMensajeEnLog(string_from_format("%s",stringBitacora));
	free(ubicacionBitacoraID);
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------------------- SABOTAJES --------------------------------------------//
void pruebaDeSabotaje(){
	generarRecurso("Oxigeno",30,0);
	generarRecurso("Basura",40,0);
	generarRecurso("Comida",50,0);

	memcpy(mapBlocks, mapBlocksCopia, tamanioBlocks);
	msync(mapBlocks, tamanioBlocks, MS_SYNC);

	bitarray_set_bit(punteroBitmap,8);
	bitarray_set_bit(punteroBitmap,9);
	guardarBitMap();
	
	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");
	fseek(superBloque, sizeof(uint32_t), SEEK_SET);
	uint32_t prueba = cantidadDeBloques * 10;
	fwrite(prueba,sizeof(uint32_t),1,superBloque);
	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);
	fclose(superBloque);
}

bool verificarBlocksBitMap(char *ubicacionArchivo){
	t_config *configGeneral;
	configGeneral = config_create(ubicacionArchivo);
	if(configGeneral!=NULL){
		char *listaBlocks = config_get_string_value(configGeneral,"BLOCKS");
		int blockOcupado;

		if(listaBlocks[1]!=']'){ 
			int inicio = 1;
			int marcador = 1;

			while(marcador<strlen(listaBlocks)){

				while(listaBlocks[marcador]!=',' && listaBlocks[marcador]!=']')
					marcador++;
				
				
				blockOcupado = atoi(string_substring(listaBlocks,inicio,marcador+inicio)) - 1;

				if(!bitarray_test_bit(punteroBitmap, blockOcupado)){
					bitarray_set_bit(punteroBitmap, blockOcupado);
				}

				inicio = marcador+1;
			}
			
		}
		return true;
	} else {
		return false;
	}
}

void verificarBitacoraBitMap(){
	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;
	dirp = opendir(string_from_format("%s/Files/Bitacoras/",puntoDeMontaje));
	while ((entry = readdir(dirp)) != NULL) {
    	if (entry->d_type == DT_REG) {
        	file_count++;
    	}
	}
	closedir(dirp);


	int i = 0;
	while(file_count>0){
		if(verificarBlocksBitMap(string_from_format("%s/Files/Tripulante%d",puntoDeMontaje,i)))
			file_count--;
		i++;
	}
}

void sabotajeSuperBloque(){
	leerSuperBloque();

	FILE * archivoBlocks; 
	archivoBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
	struct stat buf;
	fstat(archivoBlocks, &buf);
	off_t size = buf.st_size;
	int cantidadBloquesReal = size / tamanioDeBloque;

	char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
	FILE * superBloque; 
	superBloque = fopen(ubicacionSuperBloque,"r+");
	fseek(superBloque, sizeof(uint32_t), SEEK_SET);
	fwrite(cantidadBloquesReal,sizeof(uint32_t),1,superBloque);

	char *ubicacionArchivoBasura = string_from_format("%s/Files/Basura.ims",puntoDeMontaje);
	verificarBlocksBitMap(ubicacionArchivoBasura);

	char *ubicacionArchivoComida = string_from_format("%s/Files/Comida.ims",puntoDeMontaje);
	verificarBlocksBitMap(ubicacionArchivoComida);

	char *ubicacionArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims",puntoDeMontaje);
	verificarBlocksBitMap(ubicacionArchivoOxigeno);

	verificarBitacoraBitMap();

	fwrite(punteroBitmap->bitarray,punteroBitmap->size,1,superBloque);	
	fclose(superBloque);
}

void sabotajeFile(){

}
//---------------------------------------------------------------------------------------------------//
//--------------------------------------- MENSAJES DEL LOG ---------------------------------------//
void mandarMensajeEnLog(char *mensaje){
	log_info(loggerImongoStore, mensaje);
	free(mensaje);
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

void deserealizarPosicion(t_paquete* paquete){
	//id
	uint32_t tid;
	memcpy(&tid, paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	uint32_t posViejaX;
	memcpy(&posViejaX, paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);
	
	uint32_t posViejaY;
	memcpy(&posViejaY, paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	uint32_t posNuevaX;
	memcpy(&posNuevaX, paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);
	
	uint32_t posNuevaY;
	memcpy(&posNuevaY, paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

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

//--poner en un switch
	memcpy(&(parametroS->parametro), paquete->buffer->stream, sizeof(int));
	paquete->buffer->stream += sizeof(int);

	memcpy(&(parametroS->tid), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	uint32_t *tid = malloc(sizeof(uint32_t));
	*tid = parametroS->tid;   // ID TRIPULANTE

	log_info(loggerImongoStore,string_from_format("cantidad de Parametros %d",parametroS->parametro));
	log_info(loggerImongoStore,string_from_format("Llego el tripulante %d",parametroS->tid));
//--

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
	case NUEVA_POSICION:
		break;

	default:
		tareaTripulante("",*tid);
		break;
	}
	memcpy(mapBlocks, mapBlocksCopia, tamanioBlocks);
	msync(mapBlocks, tamanioBlocks, MS_SYNC);
	guardarBitMap();
}

