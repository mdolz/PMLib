#include "pmlib.h"
//#include "uthash.h"
#define BUFFSIZE 1024
#define TIMELEN 10000

int pm_create_counter( char *pm_id, line_t lines, int aggregate, int interval, server_t pm_server, counter_t *pm_counter ){
/******************************************************************************
 *  Sends a request to the PM_server in order to create a new power counter.  *
 *  Each power meter is identified by a string pm_id (e.g. "PM0", "PM1").     *
 *                                                                            * 
 *  A message is sended to PM_server. PM_server sends back a message with an  *
 *  integer that is the id of the counter in the server.                      *
 *                                                                            * 
 *  Returns values are:                                                       *
 *                                                                            * 
 *   <0 : there has been an error and is not possible to create a new power   *
 *        counter.                                                            *
 *   >0 : the id of the power counter.                                        *
 *                                                                            * 
 *  Protocol                                                                  *
 *  ========                                                                  *
 *         Client ------------------------- Server                            *
 *                                                                            * 
 *                >-->-->-->-->-->-->-->-->                                   *
 *                         int   0 (MSG TYPE)                                 *
 *                         int   size of pm_id                                *
 *                         char *pm_id                                        *
 *                                                                            * 
 *                <--<--<--<--<--<--<--<--<                                   *
 *                         long int   power counter id                             *
 ******************************************************************************/

/*!  Opens the server connetion and sends a request to the PM_server in order to create a new power counter. 
  Each power meter is identified by a string pm_id (e.g. "PM0", "PM1").    
                                                                            
  A message is sended to PM_server. PM_server sends back a message with an 
  integer that is the id of the counter in the server.       

  The lines parameter defines the lines that we want to measure. This parameter is a bit map.
 
  The agregate parameter can be:

  0 : If we want the value of each line separated
  
  1 : If we want only one data with the value of addition all line data

  The interval parameter is the frecuency of the powermeter.

  The pm_server is the struct that stores data of the server.

  The counter struct is initialized with the values of the function parmeters.
  
  The counter struct has an attribute named num_lines that stores the number of lines of the device.               
                                                                            
  Returns values are:                                                      
                                                                            
   <0 : there has been an error and is not possible to create a new power counter.                                                           
 
   >0 : the id of the power counter.                                       
                                                                            
	 Protocol:                                                                 
        
		 Client ------------------------- Server                           
                	                                                            
			 >-->-->-->-->-->-->-->-->                                  
		
				int   0 (MSG TYPE)                                
		
				int   interval
				
				int   aggregate                              
		
				int   len of lines
                            
				char* lines                                       
                                                                            
			<--<--<--<--<--<--<--<--<                                  
		
				int <0 if error or 0                             */

 
  char               buffer[BUFFSIZE];
  void               *ptr_buffer;
  int                bytes, buffer_len, type_mark, msg_size, pm_lines_len, server_response, pm_id_len;
  int                sock=  socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in server;
  device_t			 device;
  
  
  /* Construct the server sockaddr_in structure */
  memset(&server, 0, sizeof(server)); 		        		/* Clear struct */
  server.sin_family = AF_INET;                   			/* Internet/IP */
  server.sin_addr.s_addr = inet_addr(pm_server.server_ip);  /* IP address */
  server.sin_port = htons(pm_server.port);          		/* server port */

  
  
  /* Establish connection */
  if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
    perror("Failed to connect with server");
    exit(1);
  }
  
  /* Construct message */
  /* message mark int 0 */
  type_mark = 0;
  memcpy((void *)buffer, (void *)&type_mark, sizeof(type_mark));
  ptr_buffer = (void *) (((long) buffer) + sizeof(type_mark));
  msg_size=sizeof(type_mark);

  /* len of pm_id */
  pm_id_len=strlen(pm_id);
  memcpy(ptr_buffer, (void *)&pm_id_len, sizeof(pm_id_len));
  ptr_buffer = (void *) (((long) ptr_buffer) + sizeof(pm_id_len));
  msg_size += sizeof(pm_id_len);

  /* pm_id */
  memcpy(ptr_buffer, (void *)pm_id, pm_id_len);
  ptr_buffer = (void *) (((long) ptr_buffer) + pm_id_len);
  msg_size += pm_id_len;
    
  /* interval */
  memcpy(ptr_buffer, (void *)&interval, sizeof(interval));
  ptr_buffer = (void *) (((long) ptr_buffer) + sizeof(interval));
  msg_size += sizeof(interval);
  
  /* aggregate */
  memcpy(ptr_buffer, (void *)&aggregate, sizeof(aggregate));
  ptr_buffer = (void *) (((long) ptr_buffer) + sizeof(aggregate));
  msg_size += sizeof(aggregate);
  
  /* len of lines */
  pm_lines_len=__LINE_SETSIZE;
  memcpy(ptr_buffer, (void *)&pm_lines_len, sizeof(pm_lines_len));
  ptr_buffer = (void *) (((long) ptr_buffer) + sizeof(pm_lines_len));
  msg_size += sizeof(pm_lines_len);
  
  /* lines */
  memcpy(ptr_buffer, (void *)&lines, pm_lines_len);
  ptr_buffer = (void *) (((long) ptr_buffer) + pm_lines_len);
  msg_size += pm_lines_len;

/* Send the message */
  if (send(sock, buffer, msg_size, 0) != msg_size) {
    perror("Mismatch in number of sent bytes in create counter");
    exit(1);
  }

  
/* Response from server */

  ptr_buffer=(void *)buffer;
  buffer_len = sizeof(buffer);

  if ((bytes = recv(sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive bytes from server in create counter");
     exit(1);
   }
 
  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));
  if ( server_response < 0 ){
      perror("PM sever unable to assign a new counter");
      exit(1);
  }
   
  /* Store information in counter structure */
  pm_get_device_info(pm_server, pm_id, &device);
  pm_counter->sock = sock;
  pm_counter->aggregate = aggregate;
  memcpy((void *)&pm_counter->lines, (void *)&lines, pm_lines_len);
  pm_counter->num_lines = device.n_lines;
  pm_counter->interval = interval;

  pm_counter->measures = (pm_measures_wt *) malloc(sizeof(pm_measures_wt));

  (pm_counter->measures)->next_timing = 0;
  (pm_counter->measures)->timing = (double *) malloc(TIMELEN * sizeof(double));
  (pm_counter->measures)->energy.watts_size = 0;
  (pm_counter->measures)->energy.watts_sets_size = 0;
  (pm_counter->measures)->energy.watts_sets = NULL;
  (pm_counter->measures)->energy.watts = NULL;
  (pm_counter->measures)->energy.lines_len = 0;

  return(0);
}
