#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <getopt.h>
#include "pmlib.h"

#define MAXLEN	50

void get_ip_from_hostname(char *ip, const char *hostname)
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
	strcpy(ip, inet_ntoa(addr));
}

int main (int argc, char *argv[])
{
	server_t server;
	counter_t counter;
	line_t lines;
	int c, frequency, aggregate, port, size, newsize, set;
	char ip[MAXLEN]= {0}, device[MAXLEN]= {0}, trace, filename[MAXLEN]= {0};
	static int help_flag= 0;
	char *cmd;

	/* Default values */	
	port= 6526;
	LINE_SET_ALL(&lines);
	aggregate= 1;
	frequency= 0;
	trace= 'e';
	sprintf(filename, "out.");
	set= -1;

	/* Parse options */
	while (1)
	{
		static struct option long_options[] =
             	{
	               	{"help",         no_argument,       &help_flag, 1},
	               	{"server",       required_argument, 0, 's'},
	               	{"port",         required_argument, 0, 'p'},
	               	{"device",       required_argument, 0, 'd'},
	               	{"lines",        required_argument, 0, 'l'},
	               	{"nonaggregate", no_argument,       0, 'n'},
	               	{"frequency",    required_argument, 0, 'f'},
	               	{"trace",        required_argument, 0, 't'},
			{"out",          required_argument, 0, 'o'},
	               	{0, 0, 0, 0}
		};
		int option_index = 0;
     
		c = getopt_long (argc, argv, "hs:p:d:l:nf:t:o:", long_options, &option_index);
     
		/* Detect the end of the options. */
		if (c == -1)
			break;
     
		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg)
				printf(" with arg %s", optarg);
				printf("\n");
				exit(1);
				break;

                        case 'h':
				/* Show this help and exit */
				printf("Help for pm_measure\n");
                                break;
     
			case 's':
				get_ip_from_hostname(ip, optarg);
				break;
     
			case 'p':
				port= atoi(optarg);
                                if ( port < 0 ) {
                                        printf("Port should be a positive integer!\n");
                                        exit(-1);
                                }
				break;

			case 'd':
                                strcpy(device, optarg);
                                break;

			case 'l':
				pm_set_lines(optarg, &lines);
				break;
     
			case 'n':
				aggregate= 0;
				break;
     
			case 'f':
				frequency= atoi(optarg);
				if ( frequency < 0 ) {
                                        printf("Frequency should be a positive integer!\n");
					exit(-1);
				}
				break;

                        case 't':
				if      ( strcmp(optarg, "txt" ) == 0 ) { trace= 't'; strcat(filename, optarg); }
				else if ( strcmp(optarg, "prv" ) == 0 ) { trace= 'p'; strcat(filename, optarg); }
				else if ( strcmp(optarg, "csv" ) == 0 ) { trace= 'c'; strcat(filename, optarg); }
			//	else if ( strcmp(optarg, "otf" ) == 0 ) { trace= 'o'; strcat(filename, optarg); }
				else if ( strcmp(optarg, "std" ) == 0 ) { trace= 's'; strcat(filename, optarg); }
				else { printf("Trace Format '%s' not recognized!\n", optarg); exit(-1); }				
                                break;

                        case 'o':
                                sprintf(filename, "%s", optarg);
                                break;

			case '?':
				/* getopt_long already printed an error message. */
				break;
     
			default:
			abort();
		}
	}
     
	if ( *ip==0 || *device==0) {
		printf("A server and measurement device are required!\n");
		exit(-1);
	}
		
	size= MAXLEN;
	cmd= malloc( sizeof(char) * size );
	cmd[0]= '\0';

	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		// printf ("non-option ARGV-elements: ");
		while (optind < argc){
			newsize= strlen(cmd) + strlen(argv[optind]) + 2;
			if (newsize > size) {
				cmd= realloc(cmd, newsize);
				newsize= size;
			}
			strcat(cmd, argv[optind]);
			optind++;
			if (optind < argc)
				strcat(cmd, " ");
			
		}
		//printf("%s\n", cmd);
	}

    	/*	
	printf("IP: %s\n", ip);
	printf("Port: %d\n", port);
	printf("Device: %s\n", device);
	printf("Aggregate: %d\n", aggregate);
	printf("Frequency: %d\n", frequency);
	printf("Trace: %c\n", trace);
	*/

	pm_set_server(ip, port, &server);
	pm_create_counter(device, lines, aggregate, frequency, server, &counter);

	pm_start_counter(&counter);
	system(cmd);
	pm_stop_counter(&counter);

	pm_get_counter_data(&counter);
	switch ( trace )
	{	
		case 't':
			pm_print_data_text(filename, counter, lines, set);
			break;
		case 'p':
			pm_print_data_paraver(filename, counter, lines, set, "us");
			break;
		case 'c':
			pm_print_data_csv(filename, counter, lines, set);
			break;
	//	acase 'o':
	//		pm_print_data_otf(filename, counter, lines, set);
	//		break;
                case 's':
                        pm_print_data_stdout(counter, lines, set);
                        break;
                case 'e':
                        pm_calculate_energy(counter, lines, set);
                        break;
	}
	pm_finalize_counter(&counter);	

	return 0;
}
