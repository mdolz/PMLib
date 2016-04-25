#include "pmlib.h"

/******************************************************************************/

int pm_set_server( char *ip, int port, server_t *pm_server){
/******************************************************************************
 * Initializes the server's IP address and port to be used for the            *
 * communication with the PM_server.                                          *
 ******************************************************************************/
/*!  Initializes the server's IP address and port to be used for the communication with the PM_server.   */
 /* if(arm_signal){
  	signal(SIGINT, handler);
	arm_signal=0;
  }
 */
  pm_server->server_ip[SERVER_IP_LEN-1]='\0';
  strncpy(pm_server->server_ip, ip, SERVER_IP_LEN-1);
  pm_server->port = port;

  /* Create the TCP socket */
/*
  if ((pm_server->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Failed to create socket");
    exit(1);
  }
*/

//  printf("termino de crear el socket \n");
  return(0);
}
