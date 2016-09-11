#include "pmlib.h"

int pm_get_counter_data( counter_t *pm_counter ){
/******************************************************************************
 *  Sends a request to the PM_Server in order to  obtain the current measures *
 *  of the pm_counter counter.                                                *
 *                                                                            *
 *  On exit, An error is reported if a negative value is returned. If         *
 *  the returned value is 0 all has been done correctly.                      *
 *                                                                            *
 *  Protocol                                                                  *
 *  ========                                                                  *
 *         Client ------------------------- Server                            *
 *                                                                            *
 *                >-->-->-->-->-->-->-->-->                                   *
 *                         int   4 (MSG TYPE)                                 *
 *                         long int   power counter id                        *
 *                                                                            *
 *                <--<--<--<--<--<--<--<--<                                   *
 *                         int   watts_size                                   *
 *            watts_size <=0  reports an error and no more data are sended    *
 *                         int   watts_sets_size                              *
 *                         int   watts_sets[watts_sets_size]                  *
 *                         double watts[watts_size]                            *
 *                         double watts_per_hour                               *
 *                                                                            *
 ******************************************************************************/

/*! Sends a request to the PM_Server in order to  obtain the current measures 
  of the pm_counter counter.                                                
                                                                            
  On exit, An error is reported if a negative value is returned. If         
  the returned value is 0 all has been done correctly.                      
                                                                            
	Protocol:                                                                  
                                                                  
		Client ------------------------- Server                            
                                                                            
			>-->-->-->-->-->-->-->-->                                   
		
			int   4 (MSG TYPE)                                                   
                                                                            
			<--<--<--<--<--<--<--<--<                                   
		
			int   watts_size                            
		
			watts_size <=0  reports an error and no more data are sended    
		
			int   watts_sets_size                              
		
			int   watts_sets[watts_sets_size]                  
		
			double watts[watts_size]                            
		
			double watts_per_hour                               
                                                                            
*/


  /*char               buffer[BUFFSIZE];*/
  char               buffer2[1024];
  void              *ptr_buffer;
  char              *string;
  int                bytes;
  int                server_response = 0;
//  int                buffer_len;
  int                type_mark, msg_size;
  /*int                *watts_sets;*/
  int                elem_watts_sets, watts_sets_len, watts_len, message_len;
  int                i;
  /*double              *watts;*/
  double              elem_watts;

  
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
  /*buffer_len = sizeof(buffer);
  printf("tamano buffer %d \n",buffer_len);*/
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
  /* *data = (pm_measures_t *) malloc(sizeof(pm_measures_t));*/

  /* Store watts_size */
  (pm_counter->measures)->energy.watts_size = server_response;

  /* Receive watts_sets_size */
  ptr_buffer=(void *)buffer2;
  /*buffer_len = sizeof(buffer);*/
  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive watts_sets_size from server in get counter");
     exit(1);
  }
  /* Receive watts_sets_size */
  memcpy((void *)&server_response, (void *)buffer2, sizeof(server_response));

/* Store watts_sets_size */
  (pm_counter->measures)->energy.watts_sets_size = server_response;

  /* Receive watts_sets_size */
  ptr_buffer=(void *)buffer2;
  /*buffer_len = sizeof(buffer);*/
  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive watts_sets_size from server in get counter");
     exit(1);
  }
  /* Receive watts_sets_size */
  memcpy((void *)&server_response, (void *)buffer2, sizeof(server_response));

/* Store watts_sets_size */
  (pm_counter->measures)->energy.lines_len = server_response;


  /* Reserve memory for watts_sets  and watts */
  watts_sets_len = (pm_counter->measures)->energy.watts_sets_size*sizeof(int);
  (pm_counter->measures)->energy.watts_sets = (int *)malloc(watts_sets_len);
  watts_len = (pm_counter->measures)->energy.watts_size*sizeof(double);
  (pm_counter->measures)->energy.watts = (double *)malloc(watts_len);


  /* Receive the rest of message */
  message_len = watts_sets_len + watts_len;

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
    //  printf("%d - ", elem_watts_sets);
      (pm_counter->measures)->energy.watts_sets[i] = elem_watts_sets;
      ptr_buffer = (void *) (((long) ptr_buffer) + sizeof(elem_watts_sets));
      }

  /* Store the values of array watts */
  for (i=0; i<(pm_counter->measures)->energy.watts_size; i++){
      memcpy((void *)&elem_watts, (void *)ptr_buffer, sizeof(elem_watts));
    //  printf("%f - ", elem_watts);
      (pm_counter->measures)->energy.watts[i] = elem_watts;
      ptr_buffer = (void *) (((long) ptr_buffer) + sizeof(elem_watts));
 }

  free(string);

  return(1);
}
