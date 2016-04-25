#include <stdio.h>
#include "pmlib.h"

int main (int argc, char *argv[])
{
	server_t server;
	counter_t counter;
	line_t lines;	
        int i, frequency= 0, aggregate= 1;

	pm_set_server("150.128.83.55", 6526, &server);
	pm_set_lines("1-3", &lines);
	pm_create_counter("DCMeter1", lines, !aggregate, frequency, server, &counter);
	pm_start_counter(&counter);
	for (i=0; i<3; i++){
        	sleep(1);
		pm_stop_counter(&counter);
		sleep(1);
		pm_continue_counter(&counter);
	}
	pm_stop_counter(&counter);
	pm_get_counter_data(&counter);
	pm_print_data_text("out.txt", counter, lines, -1);
	pm_finalize_counter(&counter);	
	return 0;
}
