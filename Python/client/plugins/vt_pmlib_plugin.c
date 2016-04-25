/*
libVtPmlibPlugin.so,
*/

/* some standard functions */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>

#include <vampirtrace/vt_plugin_cntr.h> /* VT plugin interface definition */
#include <pmlib.h>

#define PL_SUCCESS      0
#define E_NO_INIT      -1
#define E_INIT_FAILURE -2
#define E_INVALID_ARG  -3
#define E_NOT_FOUND    -4

//#define DEBUG
#define DEFAULT_PM_PORT 6526

/* counters are solely identified by their name (== name of table in DB) */
typedef struct counter {
  char* address;
  int   port;
  char* device;
  char* lines;
  char* name;
  uint64_t timer_start;
  uint64_t timer_stop;
  line_t lines_mask;
  counter_t counter;
  server_t server;
} pm_counter_t;

static pm_counter_t *counters = NULL;
static int init_failed = E_NO_INIT;  //TODO return error if any function is called and init_failed!=0
static int num_counters = 0;

/**
 * internal helper functions
 */
char* get_ip_from_hostname(const char *hostname)
{
    struct addrinfo hints, *res;
    struct in_addr addr;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        printf("getaddrinfo error: %s could not be resolved\n", hostname);
        exit(-1);
    }

    addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(res);
    return strdup(inet_ntoa(addr));
}

/*
 * check if a string matches a pattern that can include "*" and "?" as wildcards
 * @param ptrn[in] pattern
 * @param name[in] string that is checked if it matches the given pattern
 * @return 0 if name matches pattern, -1 elsewise
 */
static int strwldcmp(char *** parts, char * name)
{
    char * pos=NULL, * str_pos=NULL;
    int num_parts=0;
    int i, j;
    int ret= 0;
    char **a;

    if ( ( parts == NULL ) || ( name == NULL ) ) 
        return E_INVALID_ARG;
  
    *parts= (char**) malloc(sizeof(char*));
    /* divide pattern into substrings divided by "*" */
    pos= strdup(name);
    do
    {
        num_parts++;
	(*parts)[ num_parts - 1 ]= pos;
        *parts= (char**) realloc(*parts, num_parts*sizeof(char*));
        pos= strstr(pos, "_");
        if ( pos != NULL )
        {
            *pos='\0'; pos++;
        }
    } while ( pos != NULL );

        printf("num_parts: %s\n", name); fflush(0);
    /* check whether the string contains appropriate nr of parameters */
    if ( num_parts != 3 ) // && num_parts != 4 )
    {
	fprintf(stderr, "Incorrect format: IP_DEV_LIN_[HOST]");
	return E_INVALID_ARG;
    }
    return PL_SUCCESS;
}

/**
 * plugin interface implementation
 */

/* VT timer function */
static uint64_t (*vt_timer_function)(void) = NULL;

/* set the reference timer used by VampirTrace */
void set_timer_function(uint64_t(*timestamp)(void))
{
    vt_timer_function=timestamp;
}

/* initialize plugin 
 * @return PL_SUCCSESS if plugin is initialized successfully, E_INIT_FAILURE elsewise
 */
int32_t init(void)
{

    char ** events=NULL;
    uint64_t tmp1,tmp2;
    int i, count;

    if (vt_timer_function==NULL)
    {
        init_failed= E_INIT_FAILURE;
        fprintf(stderr,"received invalid timer function from VampirTrace\n");
    }
    else
    {
        init_failed= PL_SUCCESS;
#ifdef DEBUG
        printf("%d different metrics available\n", num_counters);
#endif
        counters= calloc(num_counters, sizeof(pm_counter_t));

    }
    return init_failed;
}

/* get info about counters that match the specified pattern
 * @param event_name[in] pattern ("*" and "?" wildcards supported)
 * @return array of events that match the given pattern
 */
vt_plugin_cntr_metric_info * get_event_info(char * event_name)
{
    int count, selected= 0, status= 0;
    char **params;
    vt_plugin_cntr_metric_info * return_values;
    server_t server;
    line_t lines;
    counter_t counter;

    #ifdef DEBUG
    printf("get information about events for pattern \"%s\"\n", event_name);
    fflush(stdout);
    #endif
   
    return_values= malloc(sizeof(vt_plugin_cntr_metric_info));

    if ( !strwldcmp(&params, event_name) )
    { 
        /* check whether the given parameters can create a counter */

        status= status | pm_set_server(get_ip_from_hostname(params[0]), DEFAULT_PM_PORT, &server);
        status= status | pm_set_lines(params[2], &lines);
        status= status | pm_create_counter(params[1], lines, 1, 0, server, &counter);
        status= status | pm_finalize_counter(&counter);

        if ( ! status )
	{
            selected++;
            return_values = realloc(return_values, (selected+1) * sizeof(vt_plugin_cntr_metric_info));
            return_values[selected-1].name = strdup(event_name);
          //  return_values[selected-1].name = strdup(params[3]);
            return_values[selected-1].unit = strdup("Watts");
            return_values[selected-1].cntr_property = VT_PLUGIN_CNTR_ABS|VT_PLUGIN_CNTR_DOUBLE|VT_PLUGIN_CNTR_LAST;
  
            num_counters++;
            counters = realloc(counters, num_counters * sizeof(pm_counter_t));
            counters[num_counters-1].name   = strdup(event_name);
          //  counters[num_counters-1].address= strdup(params[0]);
            counters[num_counters-1].address= get_ip_from_hostname(params[0]);
            counters[num_counters-1].port   = DEFAULT_PM_PORT;
            counters[num_counters-1].device = strdup(params[1]);
            counters[num_counters-1].lines  = strdup(params[2]);
        }

	free(params);
    }

    /* Last element empty */
    return_values[selected].name = NULL;

    return return_values;
}                                                    

/* activate a certain counter 
 * @param event_name[in] name of counter that should be recorded
 * @return a unique ID used by VampiTrace to collect the data later on, E_NOT_FOUND in case of an error
 */
int32_t add_counter(char * event_name)
{
    int i;
    #ifdef DEBUG
    printf("Adding counter \"%s\" - %d\n",event_name, num_counters);fflush(stdout);
    #endif

    for ( i= 0; i < num_counters; i++)
    {
    /* data collection in db is allways running, thus no starting of counters required
     * only assign event name to returned ID
     */
       #ifdef DEBUG
       printf("  EVENT: %s - %s - %d:%d\n", event_name, counters[i].name, i, num_counters);
       if ( ! strcmp(counters[i].name, event_name) ) 
          printf("  ID: %i\n",i);
       #endif
       if ( ! strcmp(counters[i].name, event_name) ) 
       {
          pm_set_server(counters[i].address, counters[i].port, &counters[i].server);
          pm_set_lines(counters[i].lines, &counters[i].lines_mask);
          pm_create_counter(counters[i].device, counters[i].lines_mask, 1, 0, counters[i].server, &counters[i].counter);
	  #ifdef DEBUG
	  printf("Start pmlib counter %d\n", i); fflush(0);
	  #endif
	  pm_start_counter(&counters[i].counter);
	  counters[i].timer_start= vt_timer_function();
          return i;
       }
       
    }
    fprintf(stderr,"event %s not found\n", event_name);

    return E_NOT_FOUND; 
}

/* return all values for a counter
 * @param counter[in] ID of counter that should be read from db
 * @param result_vector[out] array of (timestamp,value) pairs in vt_plugin format
 * @return number of returned array elements, E_NO_INIT or E_INIT_FAILURE if called uninitialized
 */
uint64_t get_all_values(int32_t counter, vt_plugin_cntr_timevalue ** result_vector)
{
    vt_plugin_cntr_timevalue * results;
    counter_t *c;
    int i, count;
    int s, set_init, set_last, ini, fin, watts_size;
    uint64_t tmp1, tmp2;
    double time, t;
    uint64_t inc_time;
 
    union{
        uint64_t u64;
        double dbl;
    } value;
  
    if ( init_failed != PL_SUCCESS ) 
        return init_failed;
 
    c= &counters[counter].counter;

    pm_stop_counter(&counters[counter].counter);
    counters[counter].timer_stop= vt_timer_function();
    pm_get_counter_data(&counters[counter].counter);

    set_init= 0;
    set_last= c->measures->energy.watts_sets_size - 1;
    count=    c->measures->energy.watts_size;

    if ( count == 0 )
    { 
        results= (vt_plugin_cntr_timevalue*)malloc(sizeof(vt_plugin_cntr_timevalue));
        results[0].timestamp = 0;
        results[0].value = 0;

        *result_vector= results;    
        return 0;
    } 
    else if ( count > 0 )
    {
        /* convert results in format required by VampirTrace */
        results= (vt_plugin_cntr_timevalue*) malloc( (count+1) *sizeof(vt_plugin_cntr_timevalue));
        results[count].timestamp = 0;
        results[count].value = 0;

        for ( s= set_init; s < set_last; s++ )
        {
            ini= c->measures->energy.watts_sets[s];
            fin= c->measures->energy.watts_sets[s+1];

            watts_size= c->measures->energy.watts_size;
            //time= c->measures->timing[ ( s * 2 ) + 1 ] - c->measures->timing[ s * 2 ];
            inc_time= ( counters[counter].timer_stop - counters[counter].timer_start ) / ( fin - ini );

            t= 0.0;
            for ( i= ini; i < fin; i++ )
            {
                #ifdef DEBUG
                printf("%lu - %f\n", counters[counter].timer_start + inc_time * (i - ini), c->measures->energy.watts[i]);
                #endif
                results[i].timestamp= counters[counter].timer_start + inc_time * (i - ini);
                value.dbl= c->measures->energy.watts[i];
                results[i].value= value.u64;
            }
        }
    }

    *result_vector=results;    
    return count;
}

/*
int32_t enable_counter(int32_t counter)
{
    return res;
}
*/

/* free resources */
void finalize(void)
{  
    int i;
    for ( i = 0; i < num_counters; i++ ) 
    {
        #ifdef DEBUG
	printf("DESTROY %i\n", i); fflush(0);
        #endif
        pm_finalize_counter(&counters[i].counter);
    }
}

/* Plugin description needed by VampirTrace
 * @return plugin property struct 
 */
vt_plugin_cntr_info get_info()
{
    vt_plugin_cntr_info info;
    memset(&info,0,sizeof(vt_plugin_cntr_info));
  
    info.set_pform_wtime_function = set_timer_function;
    info.init                     = init;
    info.add_counter              = add_counter;
                   
    info.vt_plugin_cntr_version   = VT_PLUGIN_CNTR_VERSION;
    info.run_per                  = VT_PLUGIN_CNTR_ONCE;                // not process or thread specific
    info.synch                    = VT_PLUGIN_CNTR_ASYNCH_POST_MORTEM;  // merge values post mortem
    info.get_event_info           = get_event_info;
    info.get_all_values           = get_all_values;
    info.finalize                 = finalize;
 //   info.enable_counter           = enable_counter;
    return info;
}
                                                
