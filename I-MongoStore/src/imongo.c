

#include "imongo.h"


int main(int argc, char ** argv){

	struct addrinfo hints;
	    struct addrinfo *server_info;

	    memset(&hints, 0, sizeof(hints));
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;

	    getaddrinfo(ip, puerto, &hints, &server_info);

	    int socket_server = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	    if(connect(socket_server, server_info->ai_addr, server_info->ai_addrlen) == -1)
	        printf("error conectando");

	    freeaddrinfo(server_info);

}
