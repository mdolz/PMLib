#ifndef _PMLIB
#define _PMLIB

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include "line.h"
//#include "uthash.h"

#define SERVER_IP_LEN 16  /* xxx.xxx.xxx.xxx\0 */
#define MAX_TIMING 10 
#define __LINE_SETSIZE  16
#define __NLINEBITS 128 

typedef struct {
  int    watts_size;     /* number of elements in watts                     */
  int    watts_sets_size;/* number of elements in watts_sets                */
  int   *watts_sets;     /* index of the begining of each set of power data.*/
                         /* Everytime a restart is done a new set begins    */
  double *watts;          /* power  data                                     */
  int    lines_len;
} pm_measures;

typedef struct{
 int next_timing;
 /*double timing[MAX_TIMING][2];*/
 double *timing;
 pm_measures energy;
} pm_measures_wt;

typedef char  __line_mask;

/*! line_t is a bit map where active lines are specified. To manage this bit map we have the next macros:
        Set all bits to 0:  LINE_CLR_ALL( &line_set );
	Set all bits to 1: LINE_SET_ALL( &line_set );
      	Set to 1 the i-bit: LINE_SET( i, &line_set );
        Set to 0 the i-bit: LINE_CLR( i, &line_set );
        Query if the i-bit is active: LINE_ISSET( i, &line_set );
*/

typedef struct
{
  __line_mask __bits[__LINE_SETSIZE];
} line_t;

typedef char* unit_t;

typedef struct{
  char * name;
  int max_frecuency;
  int n_lines;
} device_t;

typedef struct{
  int sock;          /* socket id  */
  int aggregate;     /* define if values are aggregated (1) or not (0) */
  line_t lines;      /* bitmap that indicates which lines will be measured */
  int num_lines;
  int interval;      /* sample rate */
  pm_measures_wt *measures;  
} counter_t;

typedef struct{
  char server_ip[SERVER_IP_LEN];
  int port;
} server_t;


//counter_t *pm_counter_list = NULL;
/*
int arm_signal = 1;
*/

/*
   Example of pm_measures_t
   ========================
  watts_size = 10
  watts_sets = 2
  *watts_sets = {0,5}
  *watts = {20, 21, 23, 24, 25, 22, 29, 27, 28, 26}
  watts_per_hour = 245

 */


/******************************************************************************/



int pm_continue_counter( counter_t *pm_counter );

int pm_create_counter( char *pm_id, line_t lines, int aggregate, int interval, server_t pm_server, counter_t *pm_counter );

int pm_finalize_counter( counter_t *pm_counter );

int pm_fprint_counter_info(char *out, counter_t pm_counter);

int pm_get_counter_data( counter_t *pm_counter);

int pm_get_counter_num_sets( counter_t pm_counter, int *num_sets );

int pm_get_device_info(server_t pm_server, char * pm_id, device_t *d);

int pm_get_devices(server_t pm_server, char ** devices_list [], int *num_devices);

int pm_print_data_csv(char *file_name, counter_t pm_counter, line_t lines, int set);

int pm_print_data_paraver(char *file_name,  counter_t pm_counter, line_t lines, int set, unit_t unit);

int pm_print_data_otf( char *file_name, counter_t pm_counter, line_t lines, int set );

int pm_print_data_stdout( counter_t pm_counter, line_t lines, int set);

int pm_print_data_text(char *file_name,  counter_t pm_counter, line_t lines, int set);

int pm_set_lines( char *lines_string ,line_t *lines );

int pm_set_server( char *ip, int port, server_t *pm_server);

int pm_start_counter( counter_t *pm_counter );

int pm_stop_counter( counter_t *pm_counter );

int pm_calculate_energy( counter_t pm_counter, line_t lines, int set);

#endif
