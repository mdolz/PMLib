#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pmlib.h"


int pm_fprint_counter_info(char *out, counter_t pm_counter){

/*! Print the parameters of counter struct. 

    The out parameter indicates the output where the data will be printed.

*/

  FILE    *file_data;
  int                i;

  file_data = fopen(out,"a");
 
  fprintf(file_data," socket: %d\n", pm_counter.sock );
  fprintf(file_data," aggregate: %d\n", pm_counter.aggregate );
  fprintf(file_data," numero de lineas del dispositivo: %d\n", pm_counter.num_lines );
  fprintf(file_data," lineas activadas: ");
  for (i=0; i<pm_counter.num_lines;i++){
  	if(LINE_ISSET( i, &pm_counter.lines ))
		fprintf(file_data,"%d\t", i+1);
  }
  fprintf(file_data," intervalo de muestreo: %d\n", pm_counter.interval);
 
  fclose(file_data);

  return(1);
}


