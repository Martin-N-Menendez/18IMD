
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 32
static char receive[BUFFER_LENGTH];

int main(){
	int ret,fd,cant_lectura;
	char texto[BUFFER_LENGTH];
	
	printf("Abriendo el dispositivo.\n");
	fd = open("/dev/i2c_gonza", O_RDWR);
	if (fd < 0){
		printf("Fallo la apertura del dispositivo...");
		return -1;
	}

	while(1)
	{
		printf("Ingrese el modo de operacion:\r\n");
		printf("1 > Ver todo\r\n");
		printf("2 > Ver hora\r\n");
		printf("3 > Ver dia de la semana\r\n");
		printf("4 > Ver fecha\r\n");
		printf("5 > Ver temperatura\r\n");
		printf("0 > Salida\r\n");
		scanf("%s", texto);
		
		if(strlen(texto) > BUFFER_LENGTH)
			texto[BUFFER_LENGTH-1] = '\0';
			
		if(texto[0]=='0')
			break;	
		//printf("Se escribiran %d bytes, conteniendo \"%s\"\n", strlen(texto),texto);
		ret = write (fd,texto,strlen(texto)+1);
		if (ret < 0){
			printf("Falla al leer el mensaje del dispositivo.");
			close(fd);
			return -1;
		}

	}
	printf("Fin del programa\n");
	close(fd);
	return 0;
}