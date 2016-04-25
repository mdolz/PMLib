/*
libDataheapPlugin.so,
a library to count dataheap events in vampir trace
Copyright (C) 2010 TU Dresden, ZIH

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */

#include <vampirtrace/vt_plugin_cntr.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define MAX_EXPEC_COUNTER 20

typedef struct expec_counter_t {
    uint64_t expec_id;
    union{
      double dbl;
      uint64_t uint64;
    } last_value;
} expec_counter;

static int used_expec_counters = 0;
static int init_libexpec_failed = 0;

int init()
{
    char*    ext_metric_server;
    char*    colon_pos;
    uint32_t port;

        return 0;
}

vt_plugin_cntr_metric_info * get_event_info(char * event_name)
{
    vt_plugin_cntr_metric_info * return_values;
    /* Wildcards are not supported yet, so we need two entries */
    return_values = malloc(2 * sizeof(vt_plugin_cntr_metric_info));

    /* if the description is null it should be considered the end */
    return_values[0].name = strdup(event_name);
    return_values[0].unit = strdup("aaa");
    return_values[0].cntr_property = VT_PLUGIN_CNTR_ABS
            | VT_PLUGIN_CNTR_DOUBLE | VT_PLUGIN_CNTR_LAST;
    /* Last element empty */
    return_values[1].name = NULL;
    return return_values;
}

void fini()
{
}

int add_counter(char * event_name)
{
    uint64_t expec_id;
    uint64_t local_id;
    printf("Adding counter \"%s\"\n",event_name);fflush(stdout);
    return 0;
}

int is_thread_registered()
{
    uint8_t *flag;
    return 0;
      return 1;
}

uint64_t get_current_value(int32_t ID){
  uint64_t value;
  return 1;
}
uint64_t get_all_values(int32_t counter, vt_plugin_cntr_timevalue ** result_vector)
{
    vt_plugin_cntr_timevalue * results;
    return 0;
}

/* VT timer function */
static uint64_t (*vt_timer_function)(void) = NULL;

/* set the reference timer used by VampirTrace */
void set_timer_function(uint64_t(*timestamp)(void))
{
    vt_timer_function=timestamp;
}


vt_plugin_cntr_info get_info()
{
    vt_plugin_cntr_info info;
    memset(&info,0,sizeof(vt_plugin_cntr_info));

    info.set_pform_wtime_function = set_timer_function;

    info.init                     = init;
//    info.is_thread_registered     = is_thread_registered;
    info.add_counter              = add_counter;

    info.vt_plugin_cntr_version   = VT_PLUGIN_CNTR_VERSION;
    info.run_per                  = VT_PLUGIN_CNTR_ONCE;
    info.synch                    = VT_PLUGIN_CNTR_ASYNCH_POST_MORTEM;
    info.get_event_info           = get_event_info;
    info.get_all_values           = get_all_values;
 //   info.get_current_value        = get_current_value;
    info.finalize                 = fini;
    return info;
}
