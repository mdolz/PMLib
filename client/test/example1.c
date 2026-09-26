#include <stdio.h>
#include "pmlib.h"

int main (int argc, char *argv[])
{
	server_t server;
	device_t disp;
	char **list;
	int i, num_devices;

        printf("Empieza pm_set_server\n");
	pm_set_server("136.172.10.1", 6526, &server);
	
	printf("Empieza pm_get_devices\n");
	pm_get_devices(server, &list, &num_devices);  

	printf("Numero de dispositivos: %d\n", num_devices);      
	for(i=0; i<num_devices; i++)
		printf("Disp%d: %s\n",i, list[i]);

	printf("Empieza pm_get_device_info\n");
	pm_get_device_info(server, list[0], &disp);
	printf("Nombre: %s\n", disp.name);
	printf("Frecuencia maxima: %d\n", disp.max_frecuency);
	printf("Numero de lineas: %d\n", disp.n_lines);
        return 0;
}
