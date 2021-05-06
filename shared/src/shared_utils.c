#include "shared_utils.h"

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error conectando");
	else printf("conectado");

	freeaddrinfo(server_info);

	return socket_cliente;

}

int crear_conexionServer(char *puerto){

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //PREGUNTAR EL TEMA DE LA IP

	getaddrinfo(NULL, puerto, &hints, &server_info);

	int listening_socket= socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error Linkenado puerto");

	if(listen(listening_socket,5) == -1)
		printf("error Listening");

	freeaddrinfo(server_info);

	return listening_socket;

}
