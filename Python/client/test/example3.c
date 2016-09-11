#include <stdio.h>
#include "pmlib.h"

int main (int argc, char *argv[])
{
	server_t server;
	counter_t counter;
	line_t lines;	
        int i, frequency= 0, aggregate= 1;

	pm_set_server("127.0.0.1", 6526, &server);
	pm_set_lines("0-3", &lines);
	pm_create_counter("LMG450-1", lines, aggregate, frequency, server, &counter);

	pm_start_counter(&counter);
        sleep(10);
//	for (i=0; i<2; i++){
//		pm_stop_counter(&counter);
//		sleep(2);
//		pm_continue_counter(&counter);
//		sleep(2);
//	}
	pm_stop_counter(&counter);
	pm_get_counter_data(&counter);
	pm_print_data_text("out2.txt", counter, lines, -1);
	pm_finalize_counter(&counter);	

	return 0;
}
