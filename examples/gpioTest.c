#include <stdio.h>
#include <fcntl.h>

int fd;
int aux;
ssize_t ret;

char buffer[]="Escritura";

void main() {
	if ((fd=open("/dev/charDev_readWrite",O_RDWR))==-1) {
		printf("Error al abrir comunicación con el dispositivo\n");
		exit(-1);
	}
	ret=write(fd,&buffer,9,&aux);
	if (ret==-1) {
		printf("Error en escritura al dispositivo\n");
	}
	ret=read(fd,&buffer,7,&aux);
	if (ret==-1) {
		printf("Error en lectura al dispositivo\n");
	} else {
		printf("buffer: %s\n",buffer);
	}
	close(fd);
}
