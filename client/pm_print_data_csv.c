#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pmlib.h"


int pm_print_data_csv(char *file_name, counter_t pm_counter, line_t lines, int set){

/*! Print the structure data in a file in csv format. 

  The lines and set parameters define which data will be printed.
 
  If parameter set is 0 all sets will be printed.

  The format of the file is:
  
	Set_id;Time;Value_Line1;Value_Line2;...;Value_aggregate

*/

 FILE    *file_data;
  int	i, j, ii, s, init, last;
  int	ini, fin, watts_size, interval;
  double	time, inc_time, t, sum;
  int	*ind_print, *ind_lines, n_lines_print, n_lines_counter;

  file_data = fopen(file_name,"w");

  if ( pm_counter.aggregate )
  {	//Only aggregate value will be printed

	if (set > pm_counter.measures->energy.watts_sets_size-1 || set <-1)
		return -1;
	if (set == -1)
	{
		init= 0;
		last= pm_counter.measures->energy.watts_sets_size-1;
 	}
	else
	{
		init= set;
		last= set+1;		
	}
	fprintf(file_data,"Set_id;Time;Value_aggregate\n");
	for( s= init; s < last; s++ )
	{
		ini=pm_counter.measures->energy.watts_sets[s];
		fin=pm_counter.measures->energy.watts_sets[s+1];

		watts_size=pm_counter.measures->energy.watts_size;
		time=pm_counter.measures->timing[(s*2)+1]-pm_counter.measures->timing[s*2];
		inc_time=time/(fin-ini-1);	

		t=0.0;	 					
		for(i=ini; i<fin; i++)
		{
			fprintf(file_data,"%d;%f;%f\n", s, t, pm_counter.measures->energy.watts[i]);
			t+=inc_time;
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

	fprintf(file_data,"Set_id;Time;");
	for (j=0;j<n_lines_print;j++)
		fprintf(file_data,"Line %d;", ind_lines[j]);
	fprintf(file_data,"Sum\n");

	interval=pm_counter.measures->energy.watts_sets[pm_counter.measures->energy.watts_sets_size-1]-pm_counter.measures->energy.watts_sets[0];

	if (set == -1)
	{
		init= 0;
		last= pm_counter.measures->energy.watts_sets_size-1;
 	}
	else
	{
		init= set;
		last= set+1;		
	}

	for( s= init; s < last; s++ )
	{
		ini=pm_counter.measures->energy.watts_sets[s];
		fin=pm_counter.measures->energy.watts_sets[s+1];

		watts_size=pm_counter.measures->energy.watts_size;
		time=pm_counter.measures->timing[(s*2)+1]-pm_counter.measures->timing[s*2];
		inc_time=time/(fin-ini-1);	

		t=0.0;
		for(i=ini; i<fin; i++)
		{
			sum=0.0;
			fprintf(file_data,"%d;%f;", s, t);
	
			for(j=0;j<n_lines_print;j++)
			{
				fprintf(file_data,"%f;", pm_counter.measures->energy.watts[i+interval*ind_print[j]]);
				sum+=pm_counter.measures->energy.watts[i+interval*ind_print[j]];
			}
			fprintf(file_data,"%f\n", sum);
			t+=inc_time;
		}
	}
	free(ind_print);
	free(ind_lines);
  }
  
  fclose(file_data);
  return(1);

}
