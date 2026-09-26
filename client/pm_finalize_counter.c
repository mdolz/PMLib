#include "pmlib.h"

#define BUFFSIZE 1024

int pm_finalize_counter( counter_t *pm_counter ){
/******************************************************************************
 * Sends a request to the PM_Server in order to destroy the                   *
 * pm_counter counter.                                                        *
 *                                                                            *
 * On exit, An error is reported if a negative value is returned. If          *
 * the returned value is 0 all has been done correctly.                       *
 *                                                                            *
 *  Protocol                                                                  *
 *  ========                                                                  *
 *         Client ------------------------- Server                            *
 *                                                                            *
 *                >-->-->-->-->-->-->-->-->                                   *
 *                         int   5    MESSAG TYPE)                            *
 *                         long int   power counter id                        *
 *                                                                            *
 *                <--<--<--<--<--<--<--<--<                                   *
 *                         int   0 or <0 (error code)                         *
 *                                                                            *
 ******************************************************************************/

 /*!Sends a request to the PM_Server in order to destroy the                   
  pm_counter counter. 

  Closes the conexion with the server.

  Frees the data array.

  Deletes the counter struct from the global variable.                                                       
                                                                            
  On exit, An error is reported if a negative value is returned. If         
  the returned value is 0 all has been done correctly.                      
                                                                            
	Protocol:                                                                 
        
		Client ------------------------- Server                           
                                                                            
			 >-->-->-->-->-->-->-->-->                                  
		
				int   5    MESSAG TYPE)                           
		       
			<--<--<--<--<--<--<--<--<                                  
		
				int   0 or <0 (error code)       
 */


  char               buffer[BUFFSIZE];
  void              *ptr_buffer;
  int                server_response = 0;
  int                bytes, buffer_len, type_mark, msg_size;
  /* Construct message */
  /* message mark int 5 */
  type_mark = 5;
  memcpy((void *)buffer, (void *)&type_mark, sizeof(type_mark));
  ptr_buffer = buffer + sizeof(type_mark);
  msg_size=sizeof(type_mark);



/* Send the message */
  if (send(pm_counter->sock, buffer, msg_size, 0) != msg_size) {
    perror("Mismatch in number of sent bytes in destroy counter");
    exit(1);
  }

  ptr_buffer=(void *)buffer;
  buffer_len = sizeof(buffer);
  if ((bytes = recv(pm_counter->sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive bytes from server destroy counter");
     exit(1);
   }


  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));

  if (server_response < 0 ){
      perror("PM sever unable destroy counter");
      exit(1);
  }
         
  if (close(pm_counter->sock) < 0){  //Cerrar la conexión del cliente
	perror ("PM client unable destroy counter");
	exit(-1);
  }
  
  free(pm_counter->measures->timing);
  free((pm_counter->measures)->energy.watts_sets);
  free((pm_counter->measures)->energy.watts);

/*
  else{
//	HASH_DEL( pm_counter_list, pm_counter); //Eliminar el counter de la  variable global
  	free(pm_counter);  //Liberar el contador
*/
  	return(0);
 // }
}
