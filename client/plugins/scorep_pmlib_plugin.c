/*
*/

/* some standard functions */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

#include <scorep/SCOREP_MetricPlugins.h>
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
  line_t lines_mask;
  counter_t counter;
  server_t server;
} pm_counter_t;

static pm_counter_t *counters = NULL;
static int init_failed = E_NO_INIT;  //TODO return error if any function is called and init_failed!=0
static int num_counters = 0;

/* time in usec*/
GTimeVal timestamp;

/* nedded to calculate VampirTrace timestamp from internal measurement */
static uint64_t scorep_timer_start;
static uint64_t scorep_timer_stop;
static uint64_t internal_timer_start;
static uint64_t internal_timer_stop;
double factor=0.0,additional=0.0;
uint64_t diff;

/**
 * internal helper functions
 */

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

    /* check whether the string contains appropriate nr of parameters */
    if ( num_parts != 3 )
    {
	fprintf(stderr, "Incorrect format: IP_DEV_LIN");
	return E_INVALID_ARG;
    }
    return PL_SUCCESS;
}

/**
 * plugin interface implementation
 */

/* VT timer function */
static uint64_t (*scorep_clock_function)(void) = NULL;

/* set the reference timer used by VampirTrace */
void set_clock_function(uint64_t(*clock_time)(void))
{
    scorep_clock_function=clock_time;
}

/* initialize plugin 
 * @return PL_SUCCSESS if plugin is initialized successfully, E_INIT_FAILURE elsewise
 */
int32_t init(void)
{

    char ** events=NULL;
    uint64_t tmp1,tmp2;
    int i, count;

    if (scorep_clock_function==NULL)
    {
        init_failed= E_INIT_FAILURE;
        fprintf(stderr,"received invalid timer function from VampirTrace\n");
    }
    else
    {
        init_failed= PL_SUCCESS;
        /* start timestamp synchronisation */
      //  diff=0xffffffffffffffff;
      //  for ( i=0; i < 100; i++ )
      //  {
            tmp1=scorep_clock_function();
      //      g_get_current_time(&timestamp);
      //      tmp2=scorep_clock_function();
      //      if ( tmp2 - tmp1 < diff )
      //      {
      //          diff=tmp2-tmp1;
      //          internal_timer_start=((uint64_t)timestamp.tv_sec)*1000000+(uint64_t)timestamp.tv_usec;
      //          scorep_timer_start=tmp1+diff/2;
                  scorep_timer_start=tmp1;
      //      }
      //  }

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
SCOREP_Metric_Plugin_MetricProperties* 
get_event_info(char * event_name)
{
    int count, selected= 0, status= 0;
    char **params;
    SCOREP_Metric_Plugin_MetricProperties* return_values;
    server_t server;
    line_t lines;
    counter_t counter;

    #ifdef DEBUG
    printf("get information about events for pattern \"%s\"\n", event_name);
    fflush(stdout);
    #endif
   
    return_values= malloc(sizeof(SCOREP_Metric_Plugin_MetricProperties));

    if ( !strwldcmp(&params, event_name) )
    { 
        /* check whether the given parameters can create a counter */
        status= status | pm_set_server(params[0], DEFAULT_PM_PORT, &server);
        status= status | pm_set_lines(params[2], &lines);
        status= status | pm_create_counter(params[1], lines, 1, 0, server, &counter);
        status= status | pm_finalize_counter(&counter);

        if ( ! status )
	{
            selected++;
            return_values = realloc(return_values, (selected+1) * sizeof(SCOREP_Metric_Plugin_MetricProperties));
            return_values[selected-1].name = strdup(event_name);
            return_values[selected-1].unit = strdup("Watts");
            return_values[selected-1].mode = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
            return_values[selected-1].value_type = SCOREP_METRIC_VALUE_DOUBLE;
            return_values[selected-1].base = SCOREP_METRIC_BASE_DECIMAL;
            return_values[selected-1].exponent = 0;
  
            num_counters++;
            counters = realloc(counters, num_counters * sizeof(pm_counter_t));
            counters[num_counters-1].name   = strdup(event_name);
            counters[num_counters-1].address= strdup(params[0]);
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
          pm_start_counter(&counters[i].counter);
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
uint64_t get_all_values(int32_t counter, SCOREP_MetricTimeValuePair** result_vector)
{
    SCOREP_MetricTimeValuePair * results;
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
 
//    printf("%llu - %llu\n", scorep_timer_start, scorep_timer_stop);
    /* stop timestamp synchronisation */
//    diff=0xffffffffffffffff;
//    if (((int)factor)==0) // calculate adjustment parameters only once
//    {
//        for(i=0;i<100;i++)
//        {
            tmp1=scorep_clock_function();
//            g_get_current_time(&timestamp);
//            tmp2=scorep_clock_function();
//	    if ( tmp2 - tmp1 < diff )
//            {
//                diff= tmp2 - tmp1;
//                internal_timer_stop= ((uint64_t)timestamp.tv_sec) * 1000000 + (uint64_t)timestamp.tv_usec;
//                scorep_timer_stop= tmp1 + diff / 2;
                scorep_timer_stop= tmp1;
//            }
//         }
         /* determine timestamp adjustment parameters */
//         factor= ((double) (scorep_timer_stop-scorep_timer_start))/((double)(internal_timer_stop-internal_timer_start));
//         additional= ((double)scorep_timer_start) - factor*((double)internal_timer_start);
//    }

    c= &counters[counter].counter;

    pm_stop_counter(&counters[counter].counter);
    pm_get_counter_data(&counters[counter].counter);
    pm_start_counter(&counters[counter].counter);

    set_init= 0;
    set_last= c->measures->energy.watts_sets_size - 1;
    count=    c->measures->energy.watts_size;
 
    if ( count == 0 )
    { 
        results= (SCOREP_MetricTimeValuePair*)malloc(sizeof(SCOREP_MetricTimeValuePair));
        results[0].timestamp = 0;
        results[0].value = 0;

        *result_vector= results;    
        return 0;
    } 
    else if ( count > 0 )
    {
        /* convert results in format required by VampirTrace */
        results= (SCOREP_MetricTimeValuePair*) malloc( (count+1) *sizeof(SCOREP_MetricTimeValuePair));
        results[count].timestamp = 0;
        results[count].value = 0;

        for ( s= set_init; s < set_last; s++ )
        {
            ini= c->measures->energy.watts_sets[s];
            fin= c->measures->energy.watts_sets[s+1];

            watts_size= c->measures->energy.watts_size;
            //time= c->measures->timing[ ( s * 2 ) + 1 ] - c->measures->timing[ s * 2 ];
            inc_time= ( scorep_timer_stop - scorep_timer_start ) / ( fin - ini );

            t= 0.0;
            for ( i= ini; i < fin; i++ )
            {
                #ifdef DEBUG
                printf("%llu - %f\n", scorep_timer_start + inc_time * (i - ini), c->measures->energy.watts[i]);
                #endif
                results[i].timestamp= scorep_timer_start + inc_time * (i - ini);
                value.dbl= c->measures->energy.watts[i];
                results[i].value= value.u64;
            }
        }
    }

    scorep_timer_start= scorep_timer_stop;
    *result_vector=results;    
    return count;
}

/*
int32_t enable_counter(int32_t counter)
{
    #ifdef DEBUG
    printf("Start pmlib counter %d\n", counter); fflush(0);
    #endif
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
SCOREP_Metric_Plugin_Info
SCOREP_MetricPlugin_PmlibPlugin_get_info()
//get_info()
{
    SCOREP_Metric_Plugin_Info info;
    memset(&info,0,sizeof(SCOREP_Metric_Plugin_Info));
  
    info.set_clock_function       = set_clock_function;
    info.initialize               = init;
    info.add_counter              = add_counter;
                   
    info.plugin_version           = SCOREP_METRIC_PLUGIN_VERSION;
    info.run_per                  = SCOREP_METRIC_PER_PROCESS;   // not process or thread specific
    info.sync                     = SCOREP_METRIC_ASYNC_EVENT;  // merge values post mortem
    info.get_event_info           = get_event_info;
    info.get_all_values           = get_all_values;
    info.finalize                 = finalize;
 //   info.enable_counter           = enable_counter;
    return info;
}
                                                
