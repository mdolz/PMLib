#include <stdio.h>
#include "pmlib.h"

int main (int argc, char *argv[])
{
	server_t server;
	counter_t contador;
	counter_t contador2;
	line_t lines;	
	line_t lines2;	
        int set= -1, frequency= 0, aggregate= 0;

	//pm_set_server("150.128.83.55", 6526, &server);
	pm_set_server("10.0.0.100", 6526, &server);
	LINE_SET_ALL(&lines);
	LINE_SET_ALL(&lines2);
	pm_set_lines("0", &lines);
	pm_create_counter("LMG450-2", lines, !aggregate, frequency, server, &contador);
	pm_create_counter("ArduPowerDevice", lines2, aggregate, frequency, server, &contador2);

	pm_start_counter(&contador);
	pm_start_counter(&contador2);
       	sleep(10);
	pm_stop_counter(&contador);
	pm_stop_counter(&contador2);

	pm_get_counter_data(&contador);
	pm_get_counter_data(&contador2);

//	set= 0;
	pm_print_data_text("ext.txt", contador, lines, set);
	pm_print_data_text("ard.txt", contador2, lines2, set);
//	pm_print_data_csv("out.csv", contador, lines, set);
//	pm_print_data_paraver("out.prv", contador, lines, set, "us");
//	pm_print_data_stdout(contador, lines, 0);

	pm_finalize_counter(&contador);	
	pm_finalize_counter(&contador2);	
	return 0;
}
