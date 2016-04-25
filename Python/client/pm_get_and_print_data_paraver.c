#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pmlib.h"

#define SIZE 50


int pm_get_and_print_data_paraver(char *file_name, counter_t *pm_counter, line_t lines, int set, unit_t unit){

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
  int	i, j, ii, init, last;
  int	ini, fin, interval, watts_size;
//  double	sum;
  long double tm, tot_time, inc_time;
  int	*ind_print, *ind_lines, n_lines_print, n_lines_counter;
  long int line;
  time_t curtime;
  struct tm *loctime;
  char date[SIZE];



  char               buffer2[1024];
  void              *ptr_buffer;
  char              *string;
  int                bytes;
  int                server_response = 0;
  int                type_mark, msg_size;
  int                elem_watts_sets, watts_sets_len, watts_len, message_len;
  double              elem_watts;
  unsigned long long int read, current_line, read_from_line, print_current_line, current_line_pos;
  
  /* Construct message */
  /* message mark int 4 */
  type_mark = 4;
  memcpy((void *)buffer2, (void *)&type_mark, sizeof(type_mark));
  ptr_buffer = buffer2 + sizeof(type_mark);
  msg_size=sizeof(type_mark);


/* Send the message */
  if (send(pm_counter->sock, buffer2, msg_size, 0) != msg_size) {
    perror("Mismatch in number of sent bytes in get counter");
    exit(1);
  }

  /* Start to receive the message */
  ptr_buffer=(void *)buffer2;

  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive bytes from server in get counter");
     exit(1);
   }

  /* Receive watts_size, if <0 exit */
  memcpy((void *)&server_response, (void *)buffer2, sizeof(server_response));

  if (server_response < 0 ){
      perror("Failed to receive watts_size from server in get counter");
      exit(1);
  }

  /* create struct to store message*/
 
  /* Store watts_size */
  (pm_counter->measures)->energy.watts_size = server_response;

  /* Receive watts_sets_size */
  ptr_buffer=(void *)buffer2;

  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive watts_sets_size from server in get counter");
     exit(1);
  }
  /* Receive watts_sets_size */
  memcpy((void *)&server_response, (void *)buffer2, sizeof(server_response));


/* Store watts_sets_size */
  (pm_counter->measures)->energy.watts_sets_size = server_response;

  /* Receive number of lines */
  ptr_buffer=(void *)buffer2;
  /*buffer_len = sizeof(buffer);*/
  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive watts_sets_size from server in get counter");
     exit(1);
  }
  /* Receive number of lines */
  memcpy((void *)&server_response, (void *)buffer2, sizeof(server_response));

  /* Store number of lines */
  (pm_counter->measures)->energy.lines_len = server_response;


  /* Reserve memory for watts_sets */
  watts_sets_len = (pm_counter->measures)->energy.watts_sets_size*sizeof(int);
  (pm_counter->measures)->energy.watts_sets = (int *)malloc(watts_sets_len);
  watts_len = (pm_counter->measures)->energy.watts_size*sizeof(double);	


 /* Receive the rest of message */
  message_len = watts_sets_len;

  /*reservar espacio para mensaje a recibir */
  string=(char*) malloc(message_len);
  ptr_buffer=(void *)string;


  if ((bytes = recv(pm_counter->sock, ptr_buffer, message_len, MSG_WAITALL)) != message_len ) {
        perror("Failed to receive the rest of the message from server in get counter");
         exit(1);
    }


  /* Store the values of array watts_sets */
  for (i=0; i<(pm_counter->measures)->energy.watts_sets_size; i++){
      memcpy((void *)&elem_watts_sets, (void *)ptr_buffer, sizeof(elem_watts_sets));
      (pm_counter->measures)->energy.watts_sets[i] = elem_watts_sets;
      ptr_buffer +=  sizeof(elem_watts_sets);
      }

 
  free(string);
  


  // Calculate increment of time
 
  file_data = fopen64(file_name,"w");
  curtime = time(NULL);
  loctime = localtime (&curtime);

  tot_time=0;
  if(set == -1){
    for(i=0;i<(pm_counter->measures)->next_timing;i++){
      tot_time+=(pm_counter->measures)->timing[(i*2)+1]-(pm_counter->measures)->timing[i*2]; 
    }
    watts_size=(pm_counter->measures)->energy.watts_sets[(pm_counter->measures)->energy.watts_sets_size-1]-(pm_counter->measures)->energy.watts_sets[0];
  }
  else{
    tot_time=(pm_counter->measures)->timing[(set*2)+1]-(pm_counter->measures)->timing[set*2];
    watts_size=(pm_counter->measures)->energy.watts_sets[set+1]-(pm_counter->measures)->energy.watts_sets[set];
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


  printf("Leido primer bloque watts\n");
  fflush(stdout);
  
 
  if ( pm_counter->aggregate )
  {	//Only aggregate value will be printed
	
	if (set > (pm_counter->measures)->energy.watts_sets_size-1 || set <-1)
		return -1;
	if (set == -1)
	{
		init= 0;
		last= (pm_counter->measures)->energy.watts_sets_size-1;
		tm = 0.0;
 	}
	else
	{
		init= set;
		last= set+1;
                tm=0.0;
                for(i=0;i<set*2;i++)
                      tm+=(pm_counter->measures)->timing[(i*2)+1]-(pm_counter->measures)->timing[i*2];
	}
	
	ini=(pm_counter->measures)->energy.watts_sets[init];
	fin=(pm_counter->measures)->energy.watts_sets[last];

	read=0;
	while(read < (pm_counter->measures)->energy.watts_size){
	  /* Start to receive the message */
	  ptr_buffer=(void *)buffer2;
	  /*buffer_len = sizeof(buffer);
	    printf("tamano buffer %d \n",buffer_len);*/
	  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(elem_watts), 0)) != sizeof(elem_watts) ) {
	    perror("Failed to receive bytes from server in get counter");
	    exit(1);
	  }
	  
	  /* Receive watts_size, if <0 exit */
	  memcpy((void *)&elem_watts, (void *)buffer2, sizeof(elem_watts));
	  read += 1;

	  if(read >= ini && read < fin){
	    fprintf(file_data, "2:0:1:1:1:%0Lf:500000009:%f\n", tm, elem_watts);
	    fflush(file_data);
	    //fsync(fileno(file_data));
	    tm+=inc_time;
	  }
	}
	
  }
  else
  {	//If all lines will be printed

	if (set > (pm_counter->measures)->energy.watts_sets_size-1 || set <-1){
		return -1;
	}
	line_t p_lines;
	LINE_AND(&p_lines, lines, pm_counter->lines);
	n_lines_counter= 0;
	n_lines_print= 0;

	for (i=0; i<__NLINEBITS && n_lines_print < (pm_counter->measures)->energy.lines_len; i++)
	{
		if(LINE_ISSET( i, &p_lines ))
			n_lines_print++;
		if(LINE_ISSET( i, &pm_counter->lines ))
			n_lines_counter++;
	}

	ind_print=(int *)malloc( n_lines_print*sizeof(int));
	ind_lines=(int *)malloc( n_lines_print*sizeof(int));
	


	j= 0; ii= 0;
	for (i=0; i<__NLINEBITS && j < (pm_counter->measures)->energy.lines_len; i++)
	{
		if(LINE_ISSET( i, &p_lines ) && LINE_ISSET( i, &pm_counter->lines ))
		{
			ind_print[ii]= j;
			ind_lines[ii]= i;
	 		ii++;
			j++;														
		}
		else if(!LINE_ISSET( i, &p_lines ) && LINE_ISSET( i, &pm_counter->lines ))
			j++;
	}	


        interval=(pm_counter->measures)->energy.watts_sets[(pm_counter->measures)->energy.watts_sets_size-1]-(pm_counter->measures)->energy.watts_sets[0];

	if (set == -1)
	{
		init= 0;
		last= (pm_counter->measures)->energy.watts_sets_size-1;
		tm=0.0;
 	}
	else
	{
		init= set;
		last= set+1;	
                tm=0.0;
                for(i=0;i<set*2;i++)
                      tm+=(pm_counter->measures)->timing[(i*2)+1]-(pm_counter->measures)->timing[i*2];
	}


	ini=(pm_counter->measures)->energy.watts_sets[init];
	fin=(pm_counter->measures)->energy.watts_sets[last];

	
	read=0;
	current_line=0;
	while(read < (pm_counter->measures)->energy.watts_size){
	  read_from_line = 0;
	  print_current_line = 0;
	  for(i=0; i<n_lines_print; i++)
	    if(current_line == ind_print[i]){
	      current_line_pos = i;
	      print_current_line = 1;
	      break;
	    }
	  while(read_from_line < interval){
	    /* Start to receive the message */
	    ptr_buffer=(void *)buffer2;

	    if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(elem_watts), 0)) != sizeof(elem_watts) ) {
	      perror("Failed to receive bytes from server in get counter");
	      exit(1);
	    }
	    
	    /* Receive watts_size, if <0 exit */
	    memcpy((void *)&elem_watts, (void *)buffer2, sizeof(elem_watts));
	    read += 1;
	    read_from_line += 1;

	    if(print_current_line && read >= ini && read < fin){
	      line=500000010+ind_lines[current_line_pos];
	      fprintf(file_data, "2:0:1:1:1:%0Lf:%ld:%f\n", tm, line, elem_watts);
	      fflush(file_data);
	      //fsync(fileno(file_data));
	      tm+=inc_time;
	    }
	  }
	  current_line +=1;
	  tm=0.0;
	}


	free(ind_print);
	free(ind_lines);
  }
  

  fclose(file_data);
  return(1);

}

