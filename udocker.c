#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <getopt.h>

/*********************************************
 * udocker - docker run wrapper              *
 *                                           *
 * 2015 - Quan Nguyen <quan.nguyen@clumeq.ca *
 *                                           *
 * Usage: udocker <ps|images|run> options    *
 *                                           *
 *********************************************/

/* Docker run syntax: https://docs.docker.com/reference/run/ */

/* Define global variables */

int uid,gid;
int myidx = 0;
int len = 0;


/* Flag set by ‘--verbose’. */
static int verbose_flag;

void help(char *name);
void cpyarg(char **myargv, char *myarg);

/* main(int argc, char **argv) - main process loop */

int main(int argc, char **argv) {

	/* Check argc count only at this point */

	if ( argc == 1 ) {
		help(argv[0]);
		exit(1);
	}

	/* Get euid and egid of actual user */

	uid = getuid();
	gid = getgid();
	struct passwd *pw = getpwuid(uid);
	const char *homedir = pw->pw_dir;
  char id[100] = ""; 
	char *homeopt;
	char *wopt;
	char sudoers[] = "/dev/null:/etc/sudoers";

	/* printf("uid:gid %d:%d %s %d\n", getuid(), getgid(), homedir, argc); */

	/* Set uid, gid, euid and egid to root */
	setegid(0);
	seteuid(0);
	setgid(0);
	setuid(0);

	/*------------------------------------------------------*/

	int c;
	extern int opterr;
	opterr = 1;
	int optname;
	/* add 20 to for the number of option parameters thhat we will add */
  #define MAXMYIDX 30
	char** myargv = malloc( (argc+MAXMYIDX)*sizeof(void*));
	/* int myargc = argc+1;  */
	while (1) {
		struct option long_options[] =
		{
			/* These options set a flag. */
			{"verbose", 		no_argument,       	&verbose_flag, 	  1},
			{"brief",   		no_argument,       	&verbose_flag,    0},
			/* These options don’t set a flag.       */
			/* We distinguish them by their indices. */
			{"help",    		no_argument,	  	0,	 	'h'},
			{"name",    		required_argument, 	&optname,	  1},
			{"rm",      		no_argument,	  	&optname, 	  1},
			{"memory",  		required_argument,	&optname, 	  1},
			{"memory-swap", 	required_argument,	&optname, 	  1},
			/*
				 {"volumes-from",	required_argument,	&optname, 	  1},
				 */
			{"volumes",     required_argument,	0, 		'v'},
			{"user",    		required_argument,	0, 		'u'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		/* set opstring[0] = '-' to process every arguments */
		c = getopt_long (argc, argv, "-hitdc:p:u:v:w:",
				long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				/* getopt_long() set a flag as found in the long option table. */
				/* don't do anything if --verbose                              */
				if (strcmp(long_options[option_index].name, "verbose") == 0)
					break;
				
				/* copy the option name preprended by --                       */
				if (myidx == MAXMYIDX) {
				  fprintf(stderr, "MAXMYIDX not large enough...\n");
				  exit(EXIT_FAILURE);
				}
				len = strlen(long_options[option_index].name);
				myargv[myidx] = malloc(len+2+1);
				memcpy(myargv[myidx], "--", 2);
				memcpy(myargv[myidx++]+2, long_options[option_index].name, len+1);

				/* copy the option if specified                                */
				if (optarg) {
				  cpyarg(myargv, optarg);
				}
				break;

			case 1:
				/* optarg points at a plain command-line argument. */
				/* verify first argument is ps|images|run          */
				if (optind == 2) {
					if ( strncmp(argv[1], "ps", 2) == 0 ) {
						if (execl("/usr/bin/docker", "docker", "ps", "-a", NULL) < 0) {
							perror("Execl:");
						}
						exit(0);
					} else if ( strncmp(argv[1], "images", 6) == 0 ) {
						if (execl("/usr/bin/docker", "docker", "images", NULL) < 0) {
							perror("Execl:");
						}
						exit(0);
					} else if ( strncmp(argv[1], "run", 3) == 0 ) {
						/* echo */ 
				    cpyarg(myargv, "echo");

						/* docker */
				    cpyarg(myargv, "docker");

						/* run */
				    cpyarg(myargv, argv[1]);

						/* -it --rm */
				    cpyarg(myargv, "-i");
				    cpyarg(myargv, "--rm");

						/* This is the place to override docker command */
						/* -u                                           */
				    cpyarg(myargv, "-u");
						
            sprintf(id, "%d:%d", uid, gid);
				    cpyarg(myargv, id);

						/* -v homedir:homedir                           */
				    cpyarg(myargv, "-v");

						len = strlen(homedir) + 1 + strlen(homedir);
						homeopt = malloc(len+1);
						sprintf(homeopt, "%s:%s", homedir, homedir);
						cpyarg(myargv, homeopt);

						/* -v /dev/null:/etc/sudoers                   */
				    cpyarg(myargv, "-v");
						cpyarg(myargv, sudoers);

						/* -w homedir                                   */
				    cpyarg(myargv, "-w");

						len = strlen(homedir) + 1;
						wopt = malloc(len+1);
						sprintf(wopt, "%s", homedir);
						cpyarg(myargv, wopt);

						/* print help if less than 3 arguments given */
						if (argc < 3) {
							help(argv[0]);
							exit(1);
						}
					} else {
						help(argv[0]);
						exit(1);
					}
				} else if (optind <= argc) {
						/* The following code is used when "-" is 
						 * specified in the getop_long options       */
				    cpyarg(myargv,  argv[optind-1]);
				}	
				break;

			case 'h':
				help(argv[0]);
		    exit(1);
				break;

			case 'i':
				/* ingore
				cpyarg(myargv, "-i");
				*/
		    /* force -rm if -i                              */
				/*
				cpyarg(myargv, "--rm");
				*/

				break;

			case 't':
				cpyarg(myargv, "-t");

				break;

			case 'd':
				/* ignore
				cpyarg(myargv, "-d");
				*/

				break;

			case 'v':
				cpyarg(myargv, "-v");
				cpyarg(myargv, optarg);
				break;

			case 'u':
				/* ignore
				cpyarg(myargv, "-u");
				cpyarg(myargv, optarg);
				*/
				break;

			case 'c':
				cpyarg(myargv, "-c");
				cpyarg(myargv, optarg);
				break;

			case 'p':
				cpyarg(myargv, "-p");
				cpyarg(myargv, optarg);
				break;

			case 'w':
				cpyarg(myargv, "-w");
				cpyarg(myargv, optarg);
				break;

			case '?':
				/* Invalid option. */
				/* getopt_long already printed an error message. */
				printf ("Invalid long option %c\n", optopt);
				if (optopt)
					printf ("Value %s", optarg);
				break;

			default:
				/* break; */
				fprintf(stderr, "Unprocessed option...\n");
				exit(EXIT_FAILURE);
				/* abort (); */
		}
	}

	/* Print any remaining command line arguments (not options). */
  /* The following code is used only when getop_long was called
	 * without the '-' option                                    */
	if (optind < argc) {
		while (optind < argc) {
			cpyarg(myargv, argv[optind++]);
		}
	}
	/* Terminate myargv vector */
	myargv[myidx] = 0;

	if (verbose_flag) {
		puts ("verbose flag is set");
	  if (execv("/bin/echo", myargv) < 0) {
		  perror("Echo:");
	  }
	} else {
	  if (execv("/usr/bin/docker", &myargv[1]) < 0) {
		  perror("docker: ");
	  }
	}
	return 0;
}

void help(char *name) {
	printf(
				"\n"
				" Usage: %s [ps|images|run] options image command\n"
				"\n"
				" Minimumm wrapper for docker run command to run as non-root\n"
				" Options are mainly for the 'docker run` command.\n"
				"\n"
				"  -h|--help          print this help text\n"
				"  --verbose          just echo the command string\n"
				"  -it|d              run in interactive mode with a pseudo terminal\n"
				"                     or in background detached mode\n"
				"  --rm               Container to be removed at exit\n"
				"  --name [container-name]\n"
				"                     Name to be given to the container\n"
				"  -v|--volumes-from [host-dir]:[container-dir]:[rw|ro]\n"
				"                     Map host directories\n"
				"  -u|--user [Username|UID]\n"
				"  -p nnnn            port number to expose\n"
				"  -w [dir]           Working directory inside the container\n"
				"\n"
				" image:              docker image to be loaded\n"
				" command:            command to be started upon container launch\n"
				"\n"
				, name);
}

void cpyarg(char **myargv, char *myarg) {
  if (myidx == MAXMYIDX) {
		fprintf(stderr, "MAXMYIDX not large enough...\n");
		exit(EXIT_FAILURE);
	}
	len = strlen(myarg);
	myargv[myidx] = malloc(len+1);
	memcpy(myargv[myidx++], myarg, len+1);
}
