#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>

#include "pmlib.h"
#include "otf.h"

int pm_print_data_otf( char *file_name, counter_t pm_counter, line_t lines, int set )
{

    int i, j, ii, s, init, last, ini, fin, interval, watts_size;
    int ind_print[__NLINEBITS], ind_label[__NLINEBITS], n_lines_print, n_lines_counter;
    long double tm, tot_time, inc_time;
    double sum;
    OTF_FileManager* manager;
    OTF_Writer* writer;

    manager= OTF_FileManager_open( 100 );
    assert( NULL != manager );

    writer = OTF_Writer_open( file_name, 1, manager );
    OTF_Writer_writeEnter( writer, 0, 1, 0, 0 );

    tot_time= 0;

    if ( set == -1 )
    {
        for( i= 0; i < pm_counter.measures->next_timing; i++ )
        {
            tot_time+= pm_counter.measures->timing[ ( i * 2 ) + 1 ] - pm_counter.measures->timing[ i * 2 ]; 
        }
        watts_size= pm_counter.measures->energy.watts_sets[ pm_counter.measures->energy.watts_sets_size - 1 ] - pm_counter.measures->energy.watts_sets[ 0 ];
    }
    else
    {
        tot_time= pm_counter.measures->timing[ ( set * 2 ) + 1 ] - pm_counter.measures->timing[ set * 2 ];
        watts_size= pm_counter.measures->energy.watts_sets[ set + 1] - pm_counter.measures->energy.watts_sets[ set ];
    }

    inc_time= tot_time / ( watts_size - 1 );  
    n_lines_print= 0;

    if ( pm_counter.aggregate )
    {	
        if ( set > pm_counter.measures->energy.watts_sets_size-1 || set < -1 )
            return -1;

	if ( set == -1 )
	{
            init= 0;
            last= pm_counter.measures->energy.watts_sets_size - 1;
            tm= 0.0;
	}
        else
        {
            init= set;
            last= set + 1;
            tm=0.0;
            for( i=0; i < set * 2; i++ )
                tm+= pm_counter.measures->timing[ ( i * 2 ) + 1 ] - pm_counter.measures->timing[ i * 2 ];
	}
	for ( s= init; s < last; s++ )
	{
            ini=pm_counter.measures->energy.watts_sets[s];
            fin=pm_counter.measures->energy.watts_sets[s+1];

            for ( i= ini; i < fin; i++ )
            {
                OTF_Writer_writeCounter( writer, (tm * 1e6), 0, 0, (uint64_t) round( pm_counter.measures->energy.watts[ i ] * 1e3 ) );
                tm += inc_time;
            }
        }
    }
    else
    {	
	if ( set > pm_counter.measures->energy.watts_sets_size-1 || set < -1 )
        {
            return -1;
        }
        line_t p_lines;
        LINE_AND(&p_lines, lines, pm_counter.lines);
        n_lines_counter= 0;
 
        for ( i=0; i < __NLINEBITS && n_lines_print < pm_counter.measures->energy.lines_len; i++ )
	{
            if ( LINE_ISSET( i, &p_lines ) )
                n_lines_print++;
            if ( LINE_ISSET( i, &pm_counter.lines ) )
                n_lines_counter++;
	}

        j= 0; ii= 0;
 
        for ( i=0; i<__NLINEBITS && j < pm_counter.measures->energy.lines_len; i++ )
        {
            if ( LINE_ISSET( i, &p_lines ) && LINE_ISSET( i, &pm_counter.lines ) )
	    {
                ind_print[ii]= j;
                ind_label[ii]= i;
                ii++;
                j++;														
            }
            else if( !LINE_ISSET( i, &p_lines ) && LINE_ISSET( i, &pm_counter.lines ) )
                j++;
        }	

        interval= pm_counter.measures->energy.watts_sets[ pm_counter.measures->energy.watts_sets_size - 1 ] - pm_counter.measures->energy.watts_sets[ 0 ];

        if ( set == -1 )
        {
            init= 0;
            last= pm_counter.measures->energy.watts_sets_size - 1;
            tm= 0.0;
        }
        else
        {
            init= set;
            last= set + 1;
            tm= 0.0;

            for ( i= 0; i < set * 2; i++ )
                tm+= pm_counter.measures->timing[ ( i * 2) + 1 ] - pm_counter.measures->timing[ i * 2 ];
        }

        for ( s= init; s < last; s++ )
        {
            ini= pm_counter.measures->energy.watts_sets[ s ];
            fin= pm_counter.measures->energy.watts_sets[ s + 1 ];

            for ( i= ini; i < fin; i++ )
            {
                sum= 0;
                for ( j=0; j < n_lines_print; j++ )
		{
                    OTF_Writer_writeCounter( writer, (uint64_t) (tm * 1e6), 0, j+1, (uint64_t) ( round( pm_counter.measures->energy.watts[ i + interval * ind_print[ j ] ] * 1e3 ) ) );
                    sum+= pm_counter.measures->energy.watts[ i + interval * ind_print[ j ] ];
                }
                OTF_Writer_writeCounter( writer, (uint64_t) (tm * 1e6) , 0, 0, (uint64_t) ( round( sum * 1e3 ) ) );
                tm+= inc_time;
            }
	}
    }

    OTF_Writer_writeLeave( writer, tot_time*1e6, 1, 0, 0 );
    OTF_Writer_writeDefTimerResolution( writer, 0, 1e6 );
    OTF_Writer_writeDefProcess( writer, 0, 0, "Power", 0 );
    OTF_Writer_writeDefCounterGroup( writer, 0, 63, "Power" );

    for ( j=0; j < n_lines_print + 1; j++ )
    {
        char name[ 101 ];
	if ( j == 0 )
           snprintf( name, 100, "Sum" );
	else
           snprintf( name, 100, "Line %d", ind_label[j-1] );

        OTF_Writer_writeDefCounter( writer, 0, j, name, OTF_COUNTER_TYPE_ABS | OTF_COUNTER_SCOPE_LAST, 63, "Watts");
    }

    OTF_Writer_close( writer );
    OTF_FileManager_close( manager );

    return 0;

}
