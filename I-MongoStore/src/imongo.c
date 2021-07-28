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
	sem_init(&semaforoPruebaBitacora,0,1);

	//SABOTAJE
	//pruebaDeSabotaje();
	//sabotajeImongo();
	
	signal(SIGUSR1, llegoElSignal);
	// sleep(20);
	// printf("SIGUSR1");
    // raise(SIGUSR1);

	//Inicio de servidor
	pthread_t servidor;
	pthread_t sincronizador;
    pthread_create(&servidor, NULL, servidorPrincipal, NULL);
     pthread_create(&sincronizador, NULL, sincronizacionMapBlocks, NULL);
    pthread_join(servidor, NULL);
	 	pthread_join(sincronizador, NULL);

  
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
 
//------------------------------------------------------------------- ----------------------------------//
//-------------------------------------------- LEE CONFIG --------------------------------------------//
void leerConfig(){ 
	t_config * config = config_create("./cfg/imongo.config");
	
	puntoDeMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	puertoImongoStore = config_get_string_value(config, "PUERTO");
	tiempoDeSinc = config_get_int_value(config, "TIEMPO_SINCRONIZACION");
	tamanioDeBloque = config_get_int_value(config, "BLOCK_SIZE");
    cantidadDeBloques = config_get_int_value(config, "CANTIDAD_BLOCKS");
	posicionesSabotaje = config_get_array_value(config,"POSICIONES_SABOTAJE");
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
	
	memcpy(punteroBitmap->bitarray, &(mapSuperBloque[marcador]), tamanioBitMap);
}

void mapearSuperBloque(){
	char *ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);

	archivoSuperBloque = open(ubicacionSuperBloque, O_RDWR, S_IRUSR | S_IWUSR);
	mapSuperBloque = mmap(NULL, tamanioSuperBloqueBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoSuperBloque,0);

	free(ubicacionSuperBloque);
}

void desmapearSuperbloque(){
	munmap(mapSuperBloque,tamanioSuperBloqueBlocks); 
	close(archivoSuperBloque);
}
//---------------------------------------------------------------------------------------------------//
//---------------------------------------- USO DEL FILESYSTEM ---------------------------------------//
void crearFileSystem(){
	mkdir(puntoDeMontaje,0777);

	tamanioBitMap = cantidadDeBloques / 8;
	tamanioSuperBloqueBlocks = sizeof(uint32_t)*2 + tamanioBitMap;
	tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	crearSuperBloque();

	crearBlocks();
	mkdir(string_from_format("%s/Files",puntoDeMontaje),0777);
	mkdir(string_from_format("%s/Files/Bitacoras",puntoDeMontaje),0777);
}	

void leerFileSystem(){
//superBocke	
	tamanioBitMap = cantidadDeBloques / 8;
	tamanioSuperBloqueBlocks = sizeof(uint32_t)*2 + tamanioBitMap;
    tamanioBlocks = tamanioDeBloque*cantidadDeBloques;
	mapearSuperBloque();

	crearBitMap();
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

	for(int i = 0; i<tamanioBitMap*8; i++){
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
	sem_wait(&semaforoPruebaBitacora);
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
sem_post(&semaforoPruebaBitacora);
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

char *generarMD5(char * ubicacionArchivo){
    char *md5_sum = malloc(32);
    FILE *p = popen("md5sum FileSystem/archivoMD5.ims", "r");

    int i;
    for (i = 0; i < 32; i++) {
        md5_sum[i] = fgetc(p);
	}
	
	md5_sum[i] = '\0';
    pclose(p);
	remove(ubicacionArchivo);

	return md5_sum;
}

char *conseguirMD5(t_config *configRecurso){
	int sizeRecurso = config_get_int_value(configRecurso,"SIZE");
	char ** listaBloques= config_get_array_value(configRecurso,"BLOCKS");

	char * ubicacionArchivoMD5 = string_from_format("%s/archivoMD5.ims",puntoDeMontaje);
	FILE * archivoMD5 = fopen( ubicacionArchivoMD5,"w");
	
	for(int i = 0; sizeRecurso>0; i++){
		int bloqueABuscar = atoi(listaBloques[i]) - 1;
		int marcador =  bloqueABuscar * tamanioDeBloque;
		int marcador2 = i * tamanioDeBloque;
		
		int cantidadACopiar;
		if(sizeRecurso>tamanioDeBloque)
			cantidadACopiar=tamanioDeBloque;
		else
			cantidadACopiar = sizeRecurso;
		
		char *bloqueACopiar = malloc(cantidadACopiar);
		memcpy(bloqueACopiar, &(mapBlocksCopia[marcador]), cantidadACopiar);
		fwrite(bloqueACopiar,1,cantidadACopiar,archivoMD5);

		sizeRecurso-=cantidadACopiar;
		free(listaBloques[i]);
	}
	
	fclose(archivoMD5);
	
	char *md5_sum = generarMD5(ubicacionArchivoMD5);

	free(ubicacionArchivoMD5);
	free(listaBloques);

	return md5_sum;
}

void actualizarMD5(char *recurso){
	semaforoEsperaRecurso(recurso);
	char * ubicacionArchivo = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);
	t_config *  configRecurso = config_create(ubicacionArchivo);

	char *md5_sum = conseguirMD5(configRecurso);

	config_set_value(configRecurso,"MD5_ARCHIVO",md5_sum);
	log_info(loggerImongoStore, "Se guardo el MD5 %s", md5_sum);

	config_save(configRecurso);
	config_destroy(configRecurso);
	free(md5_sum);
	free(ubicacionArchivo);
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

	actualizarMD5(recurso);
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

	actualizarMD5(recurso);
}
//---------------------------------------------------------------------------------------------------//
//--------------------------------------- MENSAJES QUE RECIBE ---------------------------------------//
void generarRecurso(char *recurso, int cantidadALlenar, uint32_t idTripulante){
	log_info(loggerImongoStore,"Tripulante %d genera %d de %s",idTripulante, cantidadALlenar, recurso);
	
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea Generar %s\n",recurso),idTripulante);

	char *ubicacionArchivoRecurso = string_from_format("%s/Files/%s.ims",puntoDeMontaje,recurso);

	sem_wait(&semaforoArchivoRecurso);
	if(access(ubicacionArchivoRecurso, F_OK ))
		creacionFileRecurso(recurso[0], ubicacionArchivoRecurso);		
	sem_post(&semaforoArchivoRecurso);
	
	llenarBlocksRecursos(recurso, cantidadALlenar);

	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,"Tripulante %d termino de generar %d de %s",idTripulante,cantidadALlenar, recurso);
}

bool consumirRecurso(char *recurso, int cantidadAConsumir, uint32_t idTripulante){
	log_info(loggerImongoStore,"Tripulante %d consume %d de %s",idTripulante,cantidadAConsumir,recurso);


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


	free(ubicacionArchivoRecurso);

	log_info(loggerImongoStore,"Se consumio %d de %s",cantidadAConsumir,recurso);
	return true;
}

void descartarBasura(uint32_t idTripulante){
	log_info(loggerImongoStore,"Tripulante %d descarta Basura",idTripulante);
	
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

		free(ubicacionArchivoBasura);

		log_info(loggerImongoStore,"Se descarto la Basura");
	}
}

char *conseguirStringBlocks(int bloqueBuscado){
	char *bloqueStringBuscado = malloc(tamanioDeBloque);
	int marcador = bloqueBuscado * tamanioDeBloque;
	memcpy(bloqueStringBuscado,&(mapBlocksCopia[marcador]), tamanioDeBloque);

	return bloqueStringBuscado;
}

char * conseguirBitacora(uint32_t idTripulante){

	log_info(loggerImongoStore,"Consiguiendo bitacora del Tripulante %d",idTripulante);
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
		log_info(loggerImongoStore,"No tiene informacion en la bitacora del Tripulante %d",idTripulante);
	}
	log_info(loggerImongoStore,"%s",stringBitacora);
	free(ubicacionBitacoraID);


	return stringBitacora;
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------------------- SABOTAJES --------------------------------------------//

void pruebaDeSabotaje(){
    generarRecurso("Oxigeno",100,0);
    generarRecurso("Basura",160,0);
    generarRecurso("Comida",200,0);
	
    memcpy(mapBlocks, mapBlocksCopia, tamanioBlocks);
    msync(mapBlocks, tamanioBlocks, MS_SYNC);
	/*
    int marcador = sizeof(uint32_t);
    int nuevaCantidadDeBloques = cantidadDeBloques * 10;

    memcpy(&(mapSuperBloque[marcador]), &nuevaCantidadDeBloques, sizeof(uint32_t));
    marcador+=sizeof(uint32_t);

     crearBitMap();
	 
    memcpy(&(mapSuperBloque[marcador]), punteroBitmap->bitarray, tamanioBitMap);
	*/
	guardarBitMap();
    msync(mapSuperBloque, tamanioSuperBloqueBlocks, MS_SYNC);
	
}

bool verificarBlocksBitMap(char *ubicacionArchivo){
    t_config *configGeneral;
    configGeneral = config_create(ubicacionArchivo);
	bool cumpleVerificacion = false;
    if(configGeneral!=NULL){
        char **listaBlocks = config_get_array_value(configGeneral,"BLOCKS");
        

        for(int i=0;listaBlocks[i]!=NULL; i++){ 
        
				
                int blockOcupado = atoi(listaBlocks[i]) - 1;
				if(!bitarray_test_bit(punteroBitmap, blockOcupado)){
                	bitarray_set_bit(punteroBitmap, blockOcupado);
					log_info(loggerImongoStore, "hola como estas");
					cumpleVerificacion = true;
				}

                free(listaBlocks[i]);
        }
        free(listaBlocks);
		free(ubicacionArchivo);
    	config_destroy(configGeneral);
    } 
   
	return cumpleVerificacion;
}

bool verificarBitacoraBitMap(){
	
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;
    dirp = opendir(string_from_format("%s/Files/Bitacoras/",puntoDeMontaje));
    
	if(dirp == NULL){
		return false;
	}
	
	while ((entry = readdir(dirp)) != NULL) {
		
        if (entry->d_type == DT_REG) {
            file_count++;
        }
    }
    closedir(dirp);

    int i = 0;
	bool cumpleVerificacion = false;
	log_info(loggerImongoStore, "file count %d",file_count);
    while(file_count>i){
        cumpleVerificacion = verificarBlocksBitMap(string_from_format("%s/Files/Bitacoras/Tripulante%d.ims",puntoDeMontaje,i));
	
		log_info(loggerImongoStore, "cumpleVerificacion %d",cumpleVerificacion);
        i++;
   }

   return cumpleVerificacion;
}

bool sabotajeBitMap(){
	int cumpleVerificacion = 0;

	char *ubicacionArchivoBasura = string_from_format("%s/Files/Basura.ims",puntoDeMontaje);
    cumpleVerificacion +=  verificarBlocksBitMap(ubicacionArchivoBasura);
log_info(loggerImongoStore, "basura cumpleVerificacion %d",cumpleVerificacion);

    char *ubicacionArchivoComida = string_from_format("%s/Files/Comida.ims",puntoDeMontaje);
    cumpleVerificacion += verificarBlocksBitMap(ubicacionArchivoComida);
log_info(loggerImongoStore, "comida cumpleVerificacion %d",cumpleVerificacion);

    char *ubicacionArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims",puntoDeMontaje);
   	cumpleVerificacion += verificarBlocksBitMap(ubicacionArchivoOxigeno);
log_info(loggerImongoStore, "oxigeno cumpleVerificacion %d",cumpleVerificacion);

    cumpleVerificacion += verificarBitacoraBitMap();
log_info(loggerImongoStore, "bitacora cumpleVerificacion %d",cumpleVerificacion);

	if(cumpleVerificacion > 0 ){
    int marcador = sizeof(uint32_t) * 2;

    memcpy(&(mapSuperBloque[marcador]), punteroBitmap->bitarray, tamanioBitMap);
    msync(mapSuperBloque, tamanioSuperBloqueBlocks, MS_SYNC);
	return true;
	} else 
	return false;
}

bool sabotajeSuperBloque(){
	log_info(loggerImongoStore, "entre al sabotaje del SuperBloque");
//Verificar Cantidad de Bloques
    char * archivoBlocks; 
    archivoBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
    struct stat buf;
    stat(archivoBlocks, &buf);
    off_t blockSize = buf.st_size;
    int cantidadDeBloquesAuxiliar = blockSize / tamanioDeBloque;
	desmapearSuperbloque();
	mapearSuperBloque();
	leerSuperBloque();
	
	if(cantidadDeBloquesAuxiliar != cantidadDeBloques){ 
	 log_info(loggerImongoStore, "cantidadDEBloquesAuxiliar %d, cantidadDeBloques %d",cantidadDeBloquesAuxiliar,cantidadDeBloques);
   	 int marcador = sizeof(uint32_t);
	 cantidadDeBloques = cantidadDeBloquesAuxiliar;
	 memcpy(&(mapSuperBloque[marcador]), &cantidadDeBloques, sizeof(uint32_t));
	 msync(mapSuperBloque, tamanioSuperBloqueBlocks, MS_SYNC);
	 log_info(loggerImongoStore, "sabotaje en cantidad de Bloques ");
	 return true;
	}	else if(sabotajeBitMap()) { 
			log_info(loggerImongoStore, "Sabotaje BitMap");
		//Verificar BitMap
			return true;
	} else 
		return false;
//Termino
    log_info(loggerImongoStore,"Termina el proceso de verificacion en el SuperBloque");
}

int llenarBloqueConRecurso(char caracterLlenado, int bloqueALlenar, int sizeALlenar){
	int cantFaltante = sizeALlenar;
	int cantidadLibreBlock = tamanioDeBloque;
	int marcador = bloqueALlenar * tamanioDeBloque;

	char *caracteresVaciado = string_repeat('\0',tamanioDeBloque);
	memcpy(&(mapBlocksCopia[marcador]), caracteresVaciado, tamanioDeBloque);

	if(cantFaltante<=cantidadLibreBlock){
		cantidadLibreBlock = cantFaltante;
	}
	char *caracteresLlenado = string_repeat(caracterLlenado,cantidadLibreBlock);
	memcpy(&(mapBlocksCopia[marcador]), caracteresLlenado, cantidadLibreBlock);
	
	cantFaltante-=cantidadLibreBlock;

	return cantFaltante;
}

bool verificarListBlocks(char *ubicacionRecurso){
	log_info(loggerImongoStore,"Verificando Lista de Bloques de %s",ubicacionRecurso);
	t_config *configRecurso = config_create(ubicacionRecurso);	
	if(configRecurso!=NULL){
		char *archivoMD5 = config_get_string_value(configRecurso, "MD5_ARCHIVO");
		char *recursoMD5Real = conseguirMD5(configRecurso);
		log_info(loggerImongoStore,"MD5\n%s\n%s",archivoMD5,recursoMD5Real);
	
		if(strcmp(archivoMD5,recursoMD5Real)==0){
			log_info(loggerImongoStore,"COMPARO EL MD5");
			//MD5 iguales
			return false;
		}
	
    	char **listaBlocks = config_get_array_value(configRecurso, "BLOCKS");
		char *caracterLlenado = config_get_string_value(configRecurso, "CARACTER_LLENADO");
		int sizeALlenar = config_get_int_value(configRecurso, "SIZE");

        for(int i = 0;listaBlocks[i]!=NULL; i++){
            int blockOcupado = atoi(listaBlocks[i]) - 1;

			sizeALlenar = llenarBloqueConRecurso(caracterLlenado[0],blockOcupado,sizeALlenar);
			
			free(listaBlocks[i]);
        }
        
		config_destroy(configRecurso);
		free(listaBlocks);
		return true;
    }
	return false;
}

bool verificarBlockCount(char *ubicacionRecurso){
	log_info(loggerImongoStore,"Verificando BlockCount de %s",ubicacionRecurso);
	t_config *configRecurso = config_create(ubicacionRecurso);
	
	if(configRecurso!=NULL){
        char **listaBlocks = config_get_array_value(configRecurso, "BLOCKS");
        int blockCountAuxiliar = config_get_int_value(configRecurso,"BLOCK_COUNT");
		int blockCount = 0;

        for(int i = 0;listaBlocks[i]!=NULL; i++){
            int blockOcupado = atoi(listaBlocks[i]) - 1;

			blockCount++;
			free(listaBlocks[i]);
        }
		
		if(blockCountAuxiliar != blockCount){
			config_set_value(configRecurso,"BLOCK_COUNT",string_itoa(blockCount));
			config_save(configRecurso);
		
			log_info(loggerImongoStore,"se modifico el blockCount archivo de  %s",ubicacionRecurso);
			config_destroy(configRecurso);
			free(listaBlocks);
			return true;
		}

		config_destroy(configRecurso);
		free(listaBlocks);
    }
	return false;
}

int reemplazarSizeBloque(int bloqueAVerificar){
	int sizeRecurso = 0;
	int marcador = bloqueAVerificar * tamanioDeBloque;
	int limite = marcador + tamanioDeBloque;
	while(mapBlocksCopia[marcador]!='\0' && marcador<limite){
		sizeRecurso++;
		marcador++;
	}
	return sizeRecurso;
}

bool verificarSize(char *ubicacionRecurso){
    log_info(loggerImongoStore,"Verificando Size de %s",ubicacionRecurso);
	t_config *configRecurso = config_create(ubicacionRecurso);

	if(configRecurso!=NULL){
        char **listaBlocks = config_get_array_value(configRecurso, "BLOCKS");
		int sizeAuxiliar = config_get_int_value(configRecurso, "SIZE");
		int sizeReal = 0;

        for(int i = 0;listaBlocks[i]!=NULL; i++){
            int blockOcupado = atoi(listaBlocks[i]) - 1;

			sizeReal += reemplazarSizeBloque(blockOcupado);
			free(listaBlocks[i]);
        }
        
		if(sizeAuxiliar != sizeReal){
			config_set_value(configRecurso,"SIZE",string_itoa(sizeReal));
			config_save(configRecurso);
			log_info(loggerImongoStore,"se modifico el Size del archivo de  %s",ubicacionRecurso);
			
			return true;
		}

		config_destroy(configRecurso);
		free(listaBlocks);
    }
	
	return false;
}

bool sabotajeFile(){
    log_info(loggerImongoStore,"Comienza el proceso de verificacion en los Files");

	char *ubicacionArchivoBasura = string_from_format("%s/Files/Basura.ims",puntoDeMontaje);
    char *ubicacionArchivoComida = string_from_format("%s/Files/Comida.ims",puntoDeMontaje);
	char *ubicacionArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims",puntoDeMontaje);

	bool cumpleVerificacion = false;


	if(!cumpleVerificacion){
		cumpleVerificacion +=verificarBlockCount(ubicacionArchivoBasura);
		cumpleVerificacion +=verificarBlockCount(ubicacionArchivoComida);
		cumpleVerificacion +=verificarBlockCount(ubicacionArchivoOxigeno);
	}  

	if(!cumpleVerificacion){
		cumpleVerificacion +=verificarSize(ubicacionArchivoBasura);
		cumpleVerificacion +=verificarSize(ubicacionArchivoComida);
		cumpleVerificacion +=verificarSize(ubicacionArchivoOxigeno);
	} 
	

	if(!cumpleVerificacion) {
		cumpleVerificacion +=verificarListBlocks(ubicacionArchivoBasura);
		cumpleVerificacion +=verificarListBlocks(ubicacionArchivoComida);
		cumpleVerificacion +=verificarListBlocks(ubicacionArchivoOxigeno);
	} 
	
    log_info(loggerImongoStore,"Termina el proceso de verificacion en los Files %d",cumpleVerificacion);

	memcpy(mapBlocks, mapBlocksCopia, tamanioBlocks);
	msync(mapBlocks, tamanioBlocks, MS_SYNC);

	free(ubicacionArchivoBasura);
	free(ubicacionArchivoComida);
	free(ubicacionArchivoOxigeno);

	return cumpleVerificacion;
}


void sabotajeImongo(){

	if(sabotajeSuperBloque()){
		log_info(loggerImongoStore,"Se arreglo el Sabotaje en el Superbloque");
	} else if (sabotajeFile()){
		log_info(loggerImongoStore,"Se arreglo el Sabotaje en File");
	}

	log_info(loggerImongoStore,"paso sabotaje");
}


//---------------------------------------------------------------------------------------------------//
//--------------------------------------------- SIGNAL ----------------------------------------------//
void llegoElSignal (){
	log_info(loggerImongoStore,"llego la signal");
	if(posicionesSabotaje[0]!=NULL){
		char *posicion = posicionesSabotaje[0];

		uint32_t posx, posy;

		posx = posicion[0] - '0';
		posy = posicion[2] - '0';

		posicionesSabotaje = &(posicionesSabotaje[1]);
		free(posicion);

		// ASIGNARLES LA POSX Y LA POSY DEL ARCHIVO DE CONFIG

		log_info(loggerImongoStore,"Sabotaje en la posicion %d|%d",posx,posy);

		serializarYMandarPosicionSabotaje(posx, posy);
	} else {
		log_info(loggerImongoStore,"No hay mas posiciones de sabotaje");
	}
}
//---------------------------------------------------------------------------------------------------//
//-------------------------------------- SINCRONIZACION BLOCKS --------------------------------------//
void sincronizacionMapBlocks(){
	while(1){
		sleep(tiempoDeSinc);
		//log_info(loggerImongoStore,"Sincronizando");
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

	// SE CONECTA POR PRIMERA VEZ PARA TENER LA CONEXION ESTABLECIDA. LA VA AUSAR PARA MANDAR FSCK
	socketParaSabotajes = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);

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
//FUNCION REPETIDA
void deserealizarPosicion(t_paquete* paquete){
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

void deserializarTareaIO(t_paquete * paquete){
	t_parametro * parametroS = malloc(sizeof(int)); 
	int tipoTarea;

	memcpy(&tipoTarea, paquete->buffer->stream, sizeof(int));
	paquete->buffer->stream += sizeof(int);

	memcpy(&(parametroS->parametro), paquete->buffer->stream, sizeof(int));
	paquete->buffer->stream += sizeof(int);

	memcpy(&(parametroS->tid), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	uint32_t *tid = malloc(sizeof(uint32_t));
	*tid = parametroS->tid;   // ID TRIPULANTE

	log_info(loggerImongoStore,string_from_format("Cantidad de Parametros %d",parametroS->parametro));
	log_info(loggerImongoStore,string_from_format("Llego el tripulante %d",parametroS->tid));

	switch (tipoTarea)
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
	}
} 

uint32_t deserializarPedidoBitacora(t_paquete * paquete){
	
	uint32_t tid;
	memcpy(&tid, paquete->buffer->stream, sizeof(uint32_t));

	return tid;
}

void serializarYMandarPosicionSabotaje(uint32_t posx, uint32_t posy){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer-> size = 2 * sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &posx, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	memcpy(stream+offset, &posy, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer-> stream = stream;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	 paquete->buffer = malloc(sizeof(buffer->size));

	paquete->header = ALERTA_DE_SABOTAJE;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t) ); 
	int offset2 = 0;

	memcpy(a_enviar + offset2, &(paquete->header), sizeof(int));
	offset2 += sizeof(int);

	memcpy(a_enviar + offset2, &(paquete->buffer->size), sizeof(uint32_t));
	offset2 += sizeof(uint32_t);

	memcpy(a_enviar + offset2, paquete-> buffer-> stream, paquete->buffer->size);

	send(socketParaSabotajes, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);

}

void serializarYMandarBitacora(char * bitacora, int socket){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	int tamanioBitacora = strlen(bitacora) + 1;

	buffer-> size = sizeof(int) + tamanioBitacora;

	void* stream = malloc(buffer->size);

	int offset = 0;

	memcpy(stream+offset, &tamanioBitacora, sizeof(tamanioBitacora));
	offset += sizeof(int);
	
	memcpy(stream+offset, bitacora, tamanioBitacora);
	offset += tamanioBitacora;

	buffer-> stream = stream;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	 paquete->buffer = malloc(sizeof(buffer->size));

	paquete->header = 3;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t) ); 
	int offset2 = 0;

	memcpy(a_enviar + offset2, &(paquete->header), sizeof(int));
	offset2 += sizeof(int);

	memcpy(a_enviar + offset2, &(paquete->buffer->size), sizeof(uint32_t));
	offset2 += sizeof(uint32_t);

	memcpy(a_enviar + offset2, paquete-> buffer-> stream, paquete->buffer->size);

	send(socket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(int),0);

}

void deserializarNuevaPosicion(t_paquete * paquete){
		uint32_t posxN;
		uint32_t posyN;
		uint32_t posyV; 
		uint32_t posxV;
		uint32_t tid;

	memcpy(&(tid), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	memcpy(&(posxV), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	memcpy(&(posyV), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	memcpy(&(posxN), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	memcpy(&(posyN), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	log_info(loggerImongoStore, "Tripulante %d estaba en %d|%d y ahora esta en %d|%d", tid, posxV, posyV, posxN, posyN);

	// METER INFO EN LA BITACORA
	llenarBlocksBitcoras(string_from_format("Se mueve de %d|%d a %d|%d\n",posxV, posyV, posxN, posyN),tid);
}

void deserializarTerminoTarea(t_paquete * paquete){
	uint32_t tid;
	int tamanioNombreTarea;

	memcpy(&(tid), paquete->buffer->stream, sizeof(uint32_t));
	paquete->buffer->stream += sizeof(uint32_t);

	memcpy(&(tamanioNombreTarea), paquete->buffer->stream, sizeof(int));
	paquete->buffer->stream += sizeof(int);

	char * nombreTarea = malloc(tamanioNombreTarea);

	memcpy(nombreTarea, paquete->buffer->stream, tamanioNombreTarea);
	paquete->buffer->stream += sizeof(tamanioNombreTarea);

	log_info(loggerImongoStore,"Trip %d termino %s", tid , nombreTarea);

	// METER INFO EN BITACORA Y LOGS, PARA TODO TIPO DE TAREA (NO SOLO PARA NORMALES)
	llenarBlocksBitcoras(string_from_format("Termina la ejecucion de la tarea %s\n",nombreTarea),tid);
}

void deserializarInicioTareaNormal(t_paquete * paquete){
	uint32_t tid;
	int tamanioNombreTarea;

	void * stream = paquete->buffer->stream;

	memcpy(&(tid), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(tamanioNombreTarea), stream, sizeof(int));
	stream += sizeof(int);

	char * nombreTarea = malloc(tamanioNombreTarea+2);

	memcpy(nombreTarea, stream, tamanioNombreTarea);

	log_info(loggerImongoStore,"Trip %d arranco %s", tid , nombreTarea);

	// METER INFO EN BITACORA Y LOGS
	llenarBlocksBitcoras(string_from_format("Comienza la ejecucion de la tarea %s\n",nombreTarea),tid);
}

void deserializarTripulanteFSCK(t_paquete * paquete){
	uint32_t tid;
	memcpy(&tid, paquete->buffer->stream, sizeof(uint32_t));
	
	//FSCK
	sabotajeImongo();
	
	// METER INFO EN BITACORA Y LOGS
	char *mensajeBitacora = string_from_format("Finalizo el sabotaje y salvo a todos en la nave\n");
	llenarBlocksBitcoras(mensajeBitacora,tid);
	free(mensajeBitacora);
}

void atenderDiscordiador(int socketCliente){
	printf("Esperando mensaje del Discordiador\n");
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	int headerRECV = recv(socketCliente, &(paquete->header) , sizeof(int), 0);

	int tamanioPAQUETE_RECV = recv(socketCliente,&(paquete-> buffer-> size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	int PAQUETE_RECV = recv(socketCliente,paquete->buffer->stream,paquete->buffer->size,0);

	switch (paquete->header)
	{
	case NUEVA_POSICION:
		deserializarNuevaPosicion(paquete);
		break;
	case HACER_TAREA:
		deserializarTareaIO(paquete);
		break;
	case BITACORA:  ;
		log_info(loggerImongoStore, "Entre en bitacoraaaaa");
		uint32_t tid = deserializarPedidoBitacora(paquete);
		char * bitacora = conseguirBitacora(tid);
		serializarYMandarBitacora(bitacora, socketCliente);
		break;
	case INICIO_TAREA_NORMAL:
		deserializarInicioTareaNormal(paquete);
		break;
	case FINALIZO_TAREA:
		deserializarTerminoTarea(paquete);
		break;
	case INICIAR_FSCK:
		deserializarTripulanteFSCK(paquete);
		break;
	case SENIAL:
		raise(SIGUSR1);
		break;
	}

	guardarBitMap();

}

    