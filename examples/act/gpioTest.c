#include <stdio.h>
#include <fcntl.h>

int fd;
int aux;
ssize_t ret;

int buffer[1]={15};

void main() {
	if ((fd=open("/dev/charDev_KernelModule",O_RDWR))==-1) { //open for reading and writing
		printf("Error al abrir comunicación con el dispositivo\n");
		exit(-1);
	}
	ret=write(fd,&buffer,1,&aux);
	if (ret==-1) {
		printf("Error en escritura al dispositivo\n");
	}
	ret=read(fd,&buffer,1,&aux);
	if (ret==-1) {
		printf("Error en lectura al dispositivo\n");
	} else {
		printf("Valor del sensor: %s\n",buffer);
	}
	close(fd);
}	
