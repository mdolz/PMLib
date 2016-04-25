#include "pmlib.h"
#define BUFFSIZE 1024

int pm_get_device_info(server_t pm_server, char * pm_id, device_t *d){

/*! This function obtains the information of the device with the specified pm_id and insert it to de device data structure.

    To achieve this, the function opens a conection with the server and then closes it.


	 Protocol:                                                                 
        
                 Client ------------------------- Server                           
                                                                                    
                         >-->-->-->-->-->-->-->-->                                  
                
                                int   6 (MSG TYPE)                                
                
                                int   size of pm_id                               
                
                                char *pm_id                                       
                                                                            
                        <--<--<--<--<--<--<--<--<                                  
                
                                int max_frecuency
				int num_lines
				int size of name
				char * name 
 */


  char               buffer[BUFFSIZE];
  void              *ptr_buffer;
  int                bytes;
  int                server_response = 0;
//  int                buffer_len;
  int                type_mark, msg_size, pm_id_len;
  int                name_size;
  int                sock=  socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in server;


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

  type_mark = 6;
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

/* Send the message */
  if (send(sock, buffer, msg_size, 0) != msg_size) {
    perror("Mismatch in number of sent bytes in create counter");
    exit(1);
  }

  /* Start to receive the message */
  ptr_buffer=(void *)buffer;

  if ((bytes = recv(sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive bytes from server in get counter");
     exit(1);
   }

  /* Receive max_frecuency, if <0 exit */
  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));

  if (server_response < 0 ){
      perror("Failed to receive max frecuency from server in get device info");
      exit(1);
  }

  /* Store max_frecuency */
  d->max_frecuency = server_response;

  /* Receive n_lines */
  ptr_buffer=(void *)buffer;
  /*buffer_len = sizeof(buffer);*/
  if ((bytes = recv(sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
     perror("Failed to receive bytes from server in get device info");
     exit(1);
  }
  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));

  if (server_response < 0 ){
      perror("Failed to receive n_lines from server in get device info");
      exit(1);
  }
  
  /* Store n_lines */
  d->n_lines = server_response;

  /*Receive size of name*/
  ptr_buffer=(void *)buffer;
  if ((bytes = recv(sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
        perror("Failed to receive bytes form server in get device info");
         exit(1);
   }
  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));

  if (server_response < 0 ){
      perror("Failed to receive size of name from server in get device info");
      exit(1);
  }
  /*Store size of name*/
  name_size=server_response;

  /*Receive name*/
  ptr_buffer=(void *)buffer;
  if ((bytes = recv(sock, ptr_buffer, name_size, 0)) != name_size ) {
        perror("Failed to receive bytes form server in get device info");
         exit(1);
   }

  /*Store name*/
  d->name = (char *) malloc( (name_size+1) * sizeof(char));
  memcpy(d->name, buffer, name_size);
  d->name[name_size]= '\0';

  if (close(sock)<0)  //Close connection from the server
  {
     perror("Server cannot be closed properly!\n");
  }

  return 0;
}
