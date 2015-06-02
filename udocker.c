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

/* Flag set by ‘--verbose’. */
static int verbose_flag;

void help(char *name);

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
	char** myargv = malloc( (argc+10)*sizeof(void*));
	/* int myargc = argc+1;  */
	int myidx = 0;
	int len = 0;

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
			{"volumes-from",	required_argument,	0, 		'v'},
			{"user",    		required_argument,	0, 		'u'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		/* set opstring[0] = '-'                     */
		c = getopt_long (argc, argv, "-hitc:u:w:",
				long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				/* getopt_long() set a flag as found in the long option table. */
				/* If this option set a flag, do nothing else now.
					 if (long_options[option_index].flag != 0)
					 break;
					 */
				if (strcmp(long_options[option_index].name, "verbose") == 0)
					break;
				len = strlen(long_options[option_index].name);
				myargv[myidx] = malloc(len+2+1);
				memcpy(myargv[myidx], "--", 2);
				memcpy(myargv[myidx++]+2, long_options[option_index].name, len+1);

				if (optarg) {
					len = strlen(optarg);
					myargv[myidx] = malloc(len+1);
					memcpy(myargv[myidx++], optarg, len+1);
				}
				break;

			case 1:
				/* optarg points at a plain command-line argument. */
				/* verify first argument */
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
						/* */ 
						len = strlen("echo");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "echo", len+1);
						/* */

						len = strlen("docker");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "docker", len+1);

						len = strlen(argv[1]);
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], argv[1], len+1);

						/* This is the place to override docker command */
						/* -it                                          */
						len = strlen("-it");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "-it", len+1);
						/* --rm                                         */
						len = strlen("--rm");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "--rm", len+1);
						/* -u                                           */
						len = strlen("-u");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "-u", len+1);
						
            sprintf(id, "%d:%d", uid, gid);
						len = strlen(id);
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], id, len+1);

						/* -v                                           */
						len = strlen("-v");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "-v", len+1);

						len = strlen(homedir) + 1 + strlen(homedir);
						homeopt = malloc(len+1);
						sprintf(homeopt, "%s:%s", homedir, homedir);
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], homeopt, len+1);

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
						len = strlen(argv[optind-1]);
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], argv[optind-1], len+1);
				}	
				break;

			case 'h':
				help(argv[0]);
				break;

			case 'i':
				/* ignore
				len = strlen("-i");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-i", len+1);
				*/
				break;

			case 't':
				/* ignore
				len = strlen("-t");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-t", len+1);
				*/
				break;

			case 'v':
				/* ignore
				len = strlen("-v");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-v", len+1);

				len = strlen(optarg);
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], optarg, len+1);
				*/
				break;

			case 'u':
				/* ignore
				len = strlen("-u");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-u", len+1);

				len = strlen(optarg);
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], optarg, len+1);
				*/
				break;

			case 'c':
				len = strlen("-c");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-c", len+1);

				len = strlen(optarg);
				myargv[myidx] = malloc(len+1);
				/* memcpy(myargv[myidx++], optarg, len+1); */
				sprintf(myargv[myidx++], "%s", optarg);
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
				abort ();
		}
	}

	/* Print any remaining command line arguments (not options). */
  /* The following code is used only when getop_long was called
	 * without the '-' option                                    */
	if (optind < argc) {
		while (optind < argc) {
			len = strlen(argv[optind]);
			myargv[myidx] = malloc(len+1);
			memcpy(myargv[myidx++], argv[optind], len+1);
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
				"  -it                run in interactive mode with a pseudo terminal\n"
				"  --rm               Container to be removed at exit\n"
				"  --name [container-name]\n"
				"                     Name to be given to the container\n"
				"  -v|--volumes-from [host-dir]:[container-dir]:[rw|ro]\n"
				"                     Map host directories\n"
				"  -u|--user [Username|UID]\n"
				"  -w [dir]           Working directory inside the container\n"
				"\n"
				" image:              docker image to be loaded\n"
				" command:            command to be started upon container launch\n"
				"\n"
				, name);
}
