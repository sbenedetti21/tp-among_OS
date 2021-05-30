#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int main(int argc, const char *argv[])
{    
    const char *filepath = "/home/utnso/TPCUATRI/tp-2021-1c-Pascusa/I-MongoStore/src/prueba/texto2.ims"; 
    int valor = 64;
    char *text = "SIZE_ARCHIVO=";    
    
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    
    size_t numero = sizeof(int);
    size_t texto = strlen(text);
    size_t textsize = numero; //+ texto +1; // + \0 null character

    

    lseek(fd, textsize-1, SEEK_SET);    
    write(fd, "", 1);

    

    // Now the file is ready to be mmapped.
    void *map = mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
 
    
    // for (size_t i = 0; i < texto; i++)
    // {
    //     printf("Writing character %c at %zu\n", text[i], i);
    //     map[i] = text[i];
    // }
    // map[texto] = valor;
    //  printf("Writing character %c at %zu\n", valor, texto);

    map = &valor;
    printf("Writing character %d", valor);

    msync(map, textsize, MS_SYNC);
    
    
    // Don't forget to free the mmapped memory
   munmap(map, textsize);

    // Un-mmaping doesn't close the file, so we still need to do that.
    close(fd);
    
    return 0;
}
