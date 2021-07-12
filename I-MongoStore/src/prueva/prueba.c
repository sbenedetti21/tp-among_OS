
#include <commons/txt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

//para borrar fileSistem
	#include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <signal.h>




void rutina (int n) {
			printf("LLEGO SIGUSR1\n");
}


void main () {

   
    signal(SIGUSR1, rutina);
    raise(SIGUSR1);
}

/* void signal_catch(int);

int main () {
	signal(SIGINT, signal_catch);

   printf("Se termina la charla??\n");
   raise(SIGINT);

   printf("Y… se nos termino la charla\n");
   return(0);
}

void signal_catch(int signal) {
   printf("No se termina maaaaas\n");
}

*/