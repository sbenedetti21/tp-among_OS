#include "imongo.h"



void crearSuperBloque(){
char * ubicacionSuperBloque = string_from_format("%s/SuperBloque.ims",puntoDeMontaje);
FILE * superBloque; 

superBloque = fopen(ubicacionSuperBloque,"w+");
fwrite(&tamanioDeBloque, sizeof(uint32_t), 1, superBloque);
fwrite(&cantidadDeBloques, sizeof(uint32_t), 1, superBloque);
fclose(superBloque);

superBloque = fopen(ubicacionSuperBloque,"r");
int valor;
fread(&valor, sizeof(uint32_t), 1, superBloque);
fclose(superBloque);

printf("%d",valor);
}


void crearFileSystem(){
crearSuperBloque();
/* char * ubicacionBlocks = string_from_format("%s/Blocks.ims",puntoDeMontaje);
FILE * blocks = fopen(ubicacionBlocks,"w");
*/
}


void conectarAlCliente(){
	t_config * config = config_create("./cfg/imongo.config");
	int listening_socket = crear_conexionServer(config_get_string_value(config, "PUERTO"));

	int socketCliente;
	struct sockaddr_in addr;

	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr, &addrlen);
	if(socketCliente == -1){
		printf("Error en la conexion");
	}else{
		printf("Conexion establecida con el Discordiador");
	}

	close(listening_socket);

	close(socketCliente);
}


int main(int argc, char ** argv){

	leerConfig();
	crearFileSystem();
	
	//conectarAlCliente();


	return 0;
}


