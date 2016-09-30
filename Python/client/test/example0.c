#include <stdio.h>
#include "pmlib.h"

int main (int argc, char *argv[])
{
	server_t servidor;
//	counter_t contador;
	counter_t contador2;
	line_t lineas;
	device_t disp;
	char **lista;
	int i, num_devices;

        int frequency= 400;
        int aggregate= 0;

	LINE_CLR_ALL(&lineas);
        pm_set_lines("0-7", &lineas);

        printf("Empieza pm_set_server\n");
	pm_set_server("127.0.0.1", 6526, &servidor);
	
	pm_create_counter("APCape8L", lineas, aggregate, frequency, servidor, &contador2);

        pm_start_counter(&contador2);
        sleep(1);
        pm_stop_counter(&contador2);

	pm_get_counter_data(&contador2);

	pm_print_data_text("out.txt", contador2, lineas, -1);
	pm_finalize_counter(&contador2);	

	return 0;
}
