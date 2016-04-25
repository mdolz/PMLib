#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pmlib.h"

#define SIZE 50

int pm_print_data_paraver(char *file_name, counter_t pm_counter, line_t lines, int set, unit_t unit){

/*! Print the structure data in a file in paraver format. 

  The lines and set parameters define which data will be printed.

  If parameter set is 0 all sets will be printed.

  The unit parameter defines the unit of the written data.

  The format of the file is:

	2:0:1:1:1:Time:500000009:Value_aggregate 
  
	2:0:1:1:1:Time:5000000010:Value   //Value of line 0
	
	2:0:1:1:1:Time:5000000011:Value   //Value of line 1

*/

 
  FILE    *file_data;
  int	i, j, ii, s, init, last;
  int	ini, fin, interval, watts_size;
  double	sum;
  long double tm, tot_time, inc_time;
  int	*ind_print, *ind_lines, n_lines_print, n_lines_counter;
  long int line;
  time_t curtime;
  struct tm *loctime;
  char date[SIZE];

  file_data = fopen(file_name,"w");
  curtime = time(NULL);
  loctime = localtime (&curtime);


  // Calculate increment of time
  tot_time=0;

  if(set == -1){
  	for(i=0;i<pm_counter.measures->next_timing;i++){
  		tot_time+=pm_counter.measures->timing[(i*2)+1]-pm_counter.measures->timing[i*2]; 
        }
        watts_size=pm_counter.measures->energy.watts_sets[pm_counter.measures->energy.watts_sets_size-1]-pm_counter.measures->energy.watts_sets[0];
  }
  else{
	tot_time=pm_counter.measures->timing[(set*2)+1]-pm_counter.measures->timing[set*2];
        watts_size=pm_counter.measures->energy.watts_sets[set+1]-pm_counter.measures->energy.watts_sets[set];
  }
  if(strcmp(unit, "")==0 || strcmp(unit, "ns")==0)
  	tot_time=tot_time*1e9;
  else if (strcmp(unit, "us")==0)
	tot_time=tot_time*1e6;
  else if (strcmp(unit, "ms")==0)
	tot_time=tot_time*1e3;
  else
	printf("Debe especificar una unidad válida (ns, us, ms)\n");

  strftime(date, SIZE, "%d/%m/%Y at %H:%M", loctime);
  fprintf(file_data,"#Paraver (%s):%0Lf_%s:1(1):1:1:(1:1)\n",date,tot_time, unit);
  
  inc_time=tot_time/(watts_size-1);  

  if ( pm_counter.aggregate )
  {	//Only aggregate value will be printed
	
	if (set > pm_counter.measures->energy.watts_sets_size-1 || set <-1)
		return -1;
	if (set == -1)
	{
		init= 0;
		last= pm_counter.measures->energy.watts_sets_size-1;
		tm = 0.0;
 	}
	else
	{
		init= set;
		last= set+1;
                tm=0.0;
                for(i=0;i<set*2;i++)
                      tm+=pm_counter.measures->timing[(i*2)+1]-pm_counter.measures->timing[i*2];
	}
	for( s= init; s < last; s++ )
	{
		ini=pm_counter.measures->energy.watts_sets[s];
		fin=pm_counter.measures->energy.watts_sets[s+1];

		for(i=ini; i<fin; i++)
		{
			fprintf(file_data, "2:0:1:1:1:%0Lf:500000009:%f\n", tm, pm_counter.measures->energy.watts[i]);
			tm += inc_time;
		}
	}
  }
  else
  {	//If all lines will be printed

	if (set > pm_counter.measures->energy.watts_sets_size-1 || set <-1){
		return -1;
	}
	line_t p_lines;
	LINE_AND(&p_lines, lines, pm_counter.lines);
	n_lines_counter= 0;
	n_lines_print= 0;

	for (i=0; i<__NLINEBITS && n_lines_print < pm_counter.measures->energy.lines_len; i++)
	{
		if(LINE_ISSET( i, &p_lines ))
			n_lines_print++;
		if(LINE_ISSET( i, &pm_counter.lines ))
			n_lines_counter++;
	}

	ind_print=(int *)malloc( n_lines_print*sizeof(int));
	ind_lines=(int *)malloc( n_lines_print*sizeof(int));
	
	j= 0; ii= 0;
	for (i=0; i<__NLINEBITS && j < pm_counter.measures->energy.lines_len; i++)
	{
		if(LINE_ISSET( i, &p_lines ) && LINE_ISSET( i, &pm_counter.lines ))
		{
			ind_print[ii]= j;
			ind_lines[ii]= i;
	 		ii++;
			j++;														
		}
		else if(!LINE_ISSET( i, &p_lines ) && LINE_ISSET( i, &pm_counter.lines ))
			j++;
	}	

        interval=pm_counter.measures->energy.watts_sets[pm_counter.measures->energy.watts_sets_size-1]-pm_counter.measures->energy.watts_sets[0];

	if (set == -1)
	{
		init= 0;
		last= pm_counter.measures->energy.watts_sets_size-1;
		tm=0.0;
 	}
	else
	{
		init= set;
		last= set+1;	
                tm=0.0;
                for(i=0;i<set*2;i++)
                      tm+=pm_counter.measures->timing[(i*2)+1]-pm_counter.measures->timing[i*2];
	}

	for( s= init; s < last; s++ )
	{
		ini=pm_counter.measures->energy.watts_sets[s];
		fin=pm_counter.measures->energy.watts_sets[s+1];

		//watts_size=pm_counter.measures->energy.watts_size;
		//time=pm_counter.measures->timing[((s-1)*2)+1]-pm_counter.measures->timing[(s-1)*2];
		//inc_time=time/(fin-ini-1);	

		for(i=ini; i<fin; i++)
		{
                        sum= 0;
			for(j=0;j<n_lines_print;j++)
			{
				//line=500000010+j;
				line=500000010+ind_lines[j];
				fprintf(file_data, "2:0:1:1:1:%0Lf:%ld:%f\n", tm, line,pm_counter.measures->energy.watts[i+interval*ind_print[j]]); //print each line
				sum+=pm_counter.measures->energy.watts[i+interval*ind_print[j]];
			}
			fprintf(file_data, "2:0:1:1:1:%0Lf:500000009:%f\n", tm, sum);
			tm+=inc_time;
		}
	}
	free(ind_print);
	free(ind_lines);
  }
  
  fclose(file_data);
  return(1);

}

