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

        int frequency= 0;
        int aggregate= 1;

	LINE_CLR_ALL(&lineas);
        pm_set_lines("0-128", &lineas);
//        pm_set_lines("", &lineas);
//	for (i=0; i<24;i+=1){
//		LINE_SET(i, &lineas);
//	}
	//LINE_SET(23, &lineas);
//	LINE_SET(0,  &lineas);

	for (i=0; i<24;i++){
		if (LINE_ISSET(i, &lineas)) printf("1");
		else printf("0");
	}
	LINE_CLR_ALL(&lineas);
        printf("\n");

        printf("Empieza pm_set_server\n");
	pm_set_server("150.128.83.55", 6526, &servidor);
	
	printf("Empieza pm_get_devices\n");
	pm_get_devices(servidor, &lista, &num_devices);  //Lista será una lista de nombres de dispositivos separados por comas

	printf("Number of devices: %d\n", num_devices);      
	for(i=0; i<num_devices; i++)
		printf("%s\n", lista[i]);

	printf("Empieza pm_get_device_info\n");
	pm_get_device_info(servidor, lista[0], &disp);

/*
	printf("%s\n", disp.name);
	printf("%d\n", disp.max_frecuency);
	printf("%d\n", disp.n_lines);
*/

/*	printf("Empieza pm_create_counter\n");
	pm_create_counter(disp.name, lineas, aggregate, frequency, servidor, &contador);

	printf("Empieza pm_start_counter\n");
	pm_start_counter(&contador);
        sleep(5);
	printf("Empieza pm_stop_counter\n");
	pm_stop_counter(&contador);

//	printf("Empieza pm_finalize_counter\n");
//        pm_finalize_counter(&contador);	

	pm_continue_counter(&contador);
        sleep(3);
	pm_stop_counter(&contador);

	pm_continue_counter(&contador);
        sleep(10);
	pm_stop_counter(&contador);
*/

/*	pm_get_counter_data(&contador);
	pm_print_data_stdout(contador, lineas, 1);
	pm_finalize_counter(&contador);	
*/
       // sleep(1);

	LINE_SET_ALL(&lineas);

	pm_create_counter("DCMeter1", lineas, aggregate, frequency, servidor, &contador2);
/*
	pm_start_counter(&contador2);
        sleep(1);
	pm_stop_counter(&contador2);
*/
        pm_continue_counter(&contador2);
        sleep(1);
        pm_stop_counter(&contador2);

	pm_get_counter_data(&contador2);

	pm_print_data_stdout(contador2, lineas, -1);
	pm_print_data_text("out.txt", contador2, lineas, -1);
	pm_print_data_csv("out.csv", contador2, lineas, -1);
	pm_print_data_paraver("out.prv", contador2, lineas, -1, "us");
	pm_finalize_counter(&contador2);	

/*

	pm_create_counter("DCMeter2", lineas, !aggregate, frequency, servidor, &contador2);
	pm_start_counter(&contador2);
        sleep(1);
	pm_stop_counter(&contador2);
	pm_get_counter_data(&contador2);

	pm_print_data_stdout(contador2, lineas, 1);
	pm_finalize_counter(&contador2);	


	pm_create_counter("PDU", lineas, !aggregate, frequency, servidor, &contador2);
	pm_start_counter(&contador2);
        sleep(10);
	pm_stop_counter(&contador2);
	pm_get_counter_data(&contador2);

	pm_print_data_stdout(contador2, lineas, 1);
	pm_finalize_counter(&contador2);	


	pm_create_counter("WattsUp1", lineas, !aggregate, frequency, servidor, &contador2);
	pm_start_counter(&contador2);
        sleep(10);
	pm_stop_counter(&contador2);
	pm_get_counter_data(&contador2);

	pm_print_data_stdout(contador2, lineas, 1);
	pm_finalize_counter(&contador2);	

        pm_create_counter("WattsUp2", lineas, !aggregate, frequency, servidor, &contador2);
        pm_start_counter(&contador2);
        sleep(10);
        pm_stop_counter(&contador2);
        pm_get_counter_data(&contador2);

        pm_print_data_stdout(contador2, lineas, 1);
        pm_finalize_counter(&contador2);

*/

	return 0;
}
