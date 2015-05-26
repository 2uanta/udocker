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
	printf("uid:gid %d:%d %s %d\n", getuid(), getgid(), homedir, argc);

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
	char** myargv = malloc( (argc-1)*sizeof(void*));
	int myargc = argc; 
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

		printf("\noptind: %d\n", optind);

		/* getopt_long stores the option index here. */
		int option_index = 0;

		/* set opstring[0] = '-'                     */
		c = getopt_long (argc, argv, "hitu:w:",
				long_options, &option_index);
		printf("option_index: %d\n", option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now.
					 if (long_options[option_index].flag != 0)
					 break;
					 */
				printf ("Long option --%s", long_options[option_index].name);
				len = strlen(long_options[option_index].name);
				myargv[myidx] = malloc(len+2+1);
				memcpy(myargv[myidx], "--", 2);
				memcpy(myargv[myidx++]+2, long_options[option_index].name, len+1);

				if (optarg) {
					printf (" with arg %s", optarg);
					len = strlen(optarg);
					myargv[myidx] = malloc(len+1);
					memcpy(myargv[myidx++], optarg, len+1);
				}
				printf ("\n");
				break;

			case 1:
				/* verify first argument */
				if (optind == 2) {
					if ( strncmp(argv[1], "ps", 3) == 0 ) {
						if (execl("/usr/bin/docker", "docker", "ps", "-a", NULL) < 0) {
							perror("Execl:");
						}
						exit(0);
					} else if ( strncmp(argv[1], "images", 7) == 0 ) {
						if (execl("/usr/bin/docker", "docker", "images", NULL) < 0) {
							perror("Execl:");
						}
						exit(0);
					} else if ( strncmp(argv[1], "run", 4) == 0 ) {

						len = strlen("echo");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "echo", len+1);

						len = strlen("docker");
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], "docker", len+1);

						len = strlen(argv[1]);
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], argv[1], len+1);

						/*
#if (execl("/usr/bin/docker", "docker", "run", "-it", "--rm", "busybox", "sh", NULL) < 0) {
# perror("Execl:");
*/
					} else {
						help(argv[0]);
						exit(1);
					}
				} else if (optind < 2) {{	
						help(argv[0]);
						exit(1);
					}
				} else if (optind < argc) {
						/* The following code is used when "-" is 
						 * specified in the getop_long options       */
						len = strlen(argv[optind]);
						myargv[myidx] = malloc(len+1);
						memcpy(myargv[myidx++], argv[optind], len+1);
				}	
				break;

			case 'h':
				puts ("option -h\n");
				printf("argv[0]: %s\n", argv[0]);
				help(argv[0]);
				break;

			case 'i':
				puts ("option -i\n");
				len = strlen("-i");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-i", len+1);
				break;

			case 't':
				printf ("option -t\n");
				len = strlen("-t");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-t", len+1);
				break;

			case 'v':
				printf ("option -v with value `%s'\n", optarg);
				len = strlen("-v");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-v", len+1);
				len = strlen(optarg);
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], optarg, len+1);
				break;

			case 'u':
				printf ("option -u  with value `%s'\n", optarg);
				len = strlen("-u");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-u", len+1);
				len = strlen(optarg);
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], optarg, len+1);
				break;

			case 'w':
				printf ("option -v with value `%s'\n", optarg);
				len = strlen("-w");
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], "-w", len+1);
				len = strlen(optarg);
				myargv[myidx] = malloc(len+1);
				memcpy(myargv[myidx++], optarg, len+1);
				break;

			case '?':
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

	/* Instead of reporting ‘--verbose’
	 *      and ‘--brief’ as they are encountered,
	 *           we report the final status resulting from them. */
	if (verbose_flag)
		puts ("verbose flag is set");

	/* Print any remaining command line arguments (not options). */
  /* The following code is used only when getop_long was called
	 * without the '-' option                                    */
	if (optind < argc) {
		printf ("non-option ARGV-elements: ");
		while (optind < argc) {
			len = strlen(argv[optind]);
			myargv[myidx] = malloc(len+1);
			memcpy(myargv[myidx++], argv[optind], len+1);

			printf ("%s ", argv[optind++]);
		}
		putchar ('\n');
	}
	/* */

	myargv[myidx] = 0;
	if (execv("/bin/echo", myargv) < 0) {
		perror("Echo:");
	}

	/*------------------------------------------------------*/

		/* Check argv for proper arguments and run 
		 * the corresponding script, if invoked.
		 */

		/*
			 if ( strncmp(argv[1], "ps", 5) == 0 ) {
			 if (execl("/usr/bin/docker", "docker", "ps", "-a", NULL) < 0) {
			 perror("Execl:");
			 }
			 } else if ( strncmp(argv[1], "images", 4) == 0 ) {
			 if (execl("/usr/bin/docker", "docker", "images", NULL) < 0) {
			 perror("Execl:");
			 }
			 } else if ( strncmp(argv[1], "run", 4) == 0 ) {
			 if (execl("/usr/bin/docker", "docker", "run", "-it", "--rm", "busybox", "sh", NULL) < 0) {
			 perror("Execl:");
			 }
			 } else {
			 printf("Usage: udocker [ps|images|run] options\n");
			 exit(1);
			 }
			 exit(0);
			 */
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
