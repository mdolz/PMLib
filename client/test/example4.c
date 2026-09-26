#include <stdio.h>
#include "pmlib.h"

int main (int argc, char *argv[]){
	server_t servidor;
	counter_t counter, counter2;
	line_t lines, lines2;	
        int frequency= 0, aggregate= 1;

	pm_set_server("150.128.83.55", 6526, &servidor);
	pm_set_lines("1,3-5", &lines);

	pm_create_counter("DCMeter1", lines, !aggregate, frequency, servidor, &counter);
	pm_create_counter("DCMeter1", lines,  aggregate, frequency, servidor, &counter2);

	pm_start_counter(&counter);
	pm_start_counter(&counter2);
       	sleep(1);
	pm_stop_counter(&counter);
	pm_stop_counter(&counter2);
	pm_get_counter_data(&counter);
	pm_get_counter_data(&counter2);

	pm_set_lines("3,4", &lines2);
	pm_print_data_text("out_non_aggregate.txt", counter, lines2, -1);
	pm_print_data_text("out_aggregate.txt", counter2, lines, -1);

	pm_finalize_counter(&counter);	
	pm_finalize_counter(&counter2);	
	return 0;
}
