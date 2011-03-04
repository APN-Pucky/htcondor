#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "frames.h"
#include "server_sm_lib.h"


// For command line arguments
#include <argp.h>

const char *argp_program_version =
	"cftp-server v0.01";
const char *argp_program_bug_address =
	"<nmitchel@cs.wisc.edu>";

/* Program documentation. */
static char doc[] =
	"Cluster File Transfer Server - A simple file transfer server designed to be run in parallel across a cluster";

/* A description of the arguments we accept. */
static char args_doc[] = "[-i TIMEOUT] [-q QUOTA] [-a HOST[:PORT]] [-l HOST[:PORT]]";

	/* The options we understand. */
static struct argp_option options[] = {
	{"verbose",
	 'v',
	 0,
	 0,
	 "Produce verbose output" },

	{"debug",
	 'd',
	 0,
	 0,
	 "Produce debug output" },

	{"initial-timeout",
	 'i',
	 "TIMEOUT",
	 0,
	 "Do not wait forever for first client connection. Exit after TIMEOUT if no client connects."},

	{"disk-quota",
	 'q',	
	 "QUOTA",
	 0,
	 "Only allow files to be transfered that fit within this disk space quota (KB - 1024 Bytes)" },

	{"announce-host" ,
	 'a',
	 "HOST[:PORT]",
	 0,
	 "Upon startup, announce presence this collector host." },

	{"listen-host",
	 'l',
	 "HOST[:PORT]",
	 0,
	 "Bind to this address instead of the default." },

	{ 0 }
};
     

     
	/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
		/* Get the input argument from argp_parse, which we
		   know is a pointer to our arguments structure. */
	ServerArguments *arguments = state->input;
	char* port_start;


	switch (key)
		{
		case 'i':
			sscanf( arg, "%d", &arguments->itimeout );
			break;

		case 'v':
			arguments->verbose = 1;
			break;

		case 'd':
			arguments->debug = 1;
			arguments->verbose = 1;
			break;

		case 'q':
			sscanf( arg, "%d", &arguments->quota );
			break;

		case 'a':
			port_start = strrchr( arg, ':' );
			if( port_start)
				{
					(*port_start) = 0;
					strcpy( arguments->aport, port_start+1 );
				}
			strcpy( arguments->ahost, arg );
			
			arguments->announce = 1;
			break;

		case 'l':
			port_start = strrchr( arg, ':' );
			if(port_start)
				{
					(*port_start) = 0;
					strcpy( arguments->lport, port_start+1 );
				}

			strcpy( arguments->lhost, arg );
			
			break;		

		case ARGP_KEY_ARG:
			if (state->arg_num > 0)
					/* Too many arguments. */
				argp_usage (state);
			break;
     
		case ARGP_KEY_END:
			if (state->arg_num < 0)
					/* Not enough arguments. */
				argp_usage (state);
			break;
     
		default:
			return ARGP_ERR_UNKNOWN;
		}
	return 0;
}

	/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };



int main( int argc, char** argv )
{

	ServerArguments arguments;
     
		/* Default values. */
	arguments.debug = 0;
	arguments.verbose = 0;
	arguments.itimeout = -1;
	arguments.quota = -1;
	arguments.announce = 0;

	arguments.aport = (char*)malloc( 16 );
	memset( arguments.aport, 0, 16 );
	arguments.ahost = (char*)malloc( 256 );
	memset( arguments.ahost, 0, 256 );

	arguments.lport = (char*)malloc( 16 );
	memset( arguments.lport, 0, 16 );
	sprintf( arguments.lport, "0" ); // By default pick random port
	arguments.lhost = (char*)malloc( 256 );
	memset( arguments.lhost, 0, 256 );
	sprintf( arguments.lhost, "localhost" );

		/* Parse our arguments; every option seen by parse_opt will
		   be reflected in arguments. */
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	if( arguments.debug )
		{
			printf ("\n--ARGUMENTS--\n"
					"\tLISTEN PORT      = %s\n"
					"\tLISTEN INTERFACE = %s"
					"\n\n--OPTIONS--\n"
					"\tQUOTA            = %d KB\n"
					"\tINITIAL TIMEOUT  = %d sec\n"
					"\tVERBOSE          = %s\n"
					"\tANNOUNCE HOST    = %s\n"
					"\tANNOUNCE PORT    = %s\n\n\n",
					arguments.lport,
					arguments.lhost,
					arguments.quota,
					arguments.itimeout,
					arguments.verbose ? "yes" : "no",
					arguments.ahost,
					arguments.aport);
		}

		//TODO: Write server main loop and associated handling code.
	run_server(&arguments);

	free( arguments.aport );
	free( arguments.ahost );
	free( arguments.lport );
	free( arguments.lhost );

	return 0;
}
