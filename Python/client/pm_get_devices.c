#include "pmlib.h"
#define BUFFSIZE 1024

int pm_get_devices(server_t pm_server, char ** devices_list [], int *num_devices ){

/*! This function obtains the name of the available devices.

    To achieve this, the function opens a conection with the server and then closes it.

	 Protocol:                                                                 
        
                 Client ------------------------- Server                           
                                                                                    
                         >-->-->-->-->-->-->-->-->                                  
                
                                int   7 (MSG TYPE)                                
                                                                                          
                        <--<--<--<--<--<--<--<--<                                  
                
                                int size of devices list  //if <0 error
				for each device:
					int dev_size
					char device[dev_size]
				
 */


  char               buffer[BUFFSIZE], **ptr_list;
  void              *ptr_buffer;
  int                bytes;
  int                server_response = 0, disp_size=0;
  int                type_mark, msg_size;
  int                i, sock= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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
  /* message mark int 7 */
  type_mark = 7;
  memcpy((void *)buffer, (void *)&type_mark, sizeof(type_mark));
  ptr_buffer = (void *) (((long) buffer) + sizeof(type_mark));
  msg_size=sizeof(type_mark);
 
  /* Send the message */
  if (send(sock, buffer, msg_size, 0) != msg_size) {
    perror("Mismatch in number of sent bytes in get devices");
    exit(1);
  }

  /* Start to receive the message */
  ptr_buffer=(void *)buffer;

  /* Size of devices string */
  if ((bytes = recv(sock, ptr_buffer, sizeof(server_response), 0)) != sizeof(server_response) ) {
        perror("Failed to receive bytes form server in get devices");
        exit(1);
  }

  memcpy((void *)&server_response, (void *)buffer, sizeof(server_response));
  if (server_response < 0 ){
      perror("PM server unable to get information of devices");
      exit(1);
  }

  /*Store size of name*/
  *num_devices= server_response;
  ptr_list= (char **)malloc(*num_devices*sizeof(char*));

  /*Receive devices*/
  for(i=0;i<*num_devices;i++){
	//Receive size of device
  	if ((bytes = recv(sock, (void *)buffer, sizeof(disp_size), 0)) != sizeof(disp_size) ) {
        	perror("Failed to receive bytes form server in get devices");
        	exit(1);
   	}
	memcpy((void *)&disp_size, (void *)buffer, sizeof(disp_size));
  	if ((bytes = recv(sock, ptr_buffer, (disp_size)*sizeof(char), 0)) != (disp_size)*sizeof(char) ) {
        	perror("Failed to receive bytes form server in get devices");
        	exit(1);
   	}

	//Reserve memory
	ptr_list[i]= (char *)malloc((disp_size+1)*sizeof(char));
	//Store device
	memcpy(ptr_list[i], buffer, (disp_size)*sizeof(char));
	ptr_list[i][disp_size]= '\0';
  }	

  *devices_list= ptr_list;
  if (close(sock)<0)  //Close connection from the server
  {
     perror("Server cannot be closed properly!\n");
  }


  return 0;

}
