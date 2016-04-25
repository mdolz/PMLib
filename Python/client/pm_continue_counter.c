#include "pmlib.h"

#define BUFFSIZE 1024

int pm_continue_counter( counter_t *pm_counter ){
/******************************************************************************
 *  Sends a request to the PM_Server in order to restart counting with the    *
 *  pm_counter counter. Values from now are added to a previous counting.     *
 *                                                                            *
 *  On exit, An error is reported if a negative value is returned. If         *
 *  the returned value is 0 all has been done correctly.                      *
 *                                                                            *
 *  Protocol                                                                  *
 *  ========                                                                  *
 *         Client ------------------------- Server                            *
 *                                                                            *
 *                >-->-->-->-->-->-->-->-->                                   *
 *                         int   2 (MSG TYPE)                                 *
 *                         long int   power counter id                        *
 *                                                                            *
 *                <--<--<--<--<--<--<--<--<                                   *
 *                         int   0 or <0 (error code)                         *
 *                                                                            *
 ******************************************************************************/

/*!  Sends a request to the PM_Server in order to restart counting with the
  pm_counter counter. Values from now are added to a previous counting.    
                                                                           
  On exit, An error is reported if a negative value is returned. If        
  the returned value is 0 all has been done correctly.                     
                                                                           
	Protocol:                                                                 
        
		 Client ------------------------- Server                           
                                                                           
            		>-->-->-->-->-->-->-->-->                                  
		
			 int   2 (MSG TYPE)                                
                    
                                                                           
			<--<--<--<--<--<--<--<--<                                  
		
			 int   0 or <0 (error code)         */

  char               buffer[BUFFSIZE];
  void              *ptr_buffer;
  int                server_response = 0;
  int                buffer_len, type_mark, msg_size, bytes;
  double             temp;
  struct timeval     temp_start;

  /* message mark int 2 */
  type_mark = 2;
  memcpy((void *)buffer, (void *)&type_mark, sizeof(type_mark));
  ptr_buffer = buffer + sizeof(type_mark);
  msg_size=sizeof(type_mark);


/* Send the message */
  if (send(pm_counter->sock, buffer, msg_size, 0) != msg_size) {
    perror("Mismatch in number of sent bytes in restart counter");
    exit(1);
  }

  ptr_buffer=(void *)buffer;
  buffer_len = sizeof(buffer);

  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive bytes from server in restart counter");
     exit(1);
   }


  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));

  if (server_response < 0 ){
      perror("PM sever unable to restart counter");
      exit(1);
  }

 /* start time counting */


  gettimeofday(&temp_start,0);
  temp = (double) (temp_start.tv_sec + (0.000001 * temp_start.tv_usec));
  (pm_counter->measures)->timing[((pm_counter->measures)->next_timing)*2] = temp;

  return(server_response);
}
