#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
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

int main(int argc, char **argv)
{

 /* Check argc count only at this point */

 if ( argc == 1 ) {
  help(argv[0]);
  exit(1);
 }

 /* Set euid and egid to actual user */

 uid = getuid();
 gid = getgid();
 struct passwd *pw = getpwuid(uid);
 const char *homedir = pw->pw_dir;
 printf("uid:gid %d:%d %s %d\n", getuid(), getgid(), homedir, argc);
 seteuid(getuid());
 setegid(getgid());
  
/*------------------------------------------------------*/

 int c;
 char **my_argv;
 int my_argv_idx = 0;
 extern int opterr;
 int nextchar;
 opterr = 1;
 int optname;

 while (1)
 {
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

  printf("\nIndex: %d\n", optind);

  /* getopt_long stores the option index here. */
  int option_index = 0;

  /* set opstring[0] = '-'                     */
  c = getopt_long (argc, argv, "-hitu:w:",
                   long_options, &option_index);

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
     if (optarg)
      printf (" with arg %s", optarg);
     printf ("\n");
     break;

    case 1:
     printf("Plain command-line argument: %s\n", optarg);
     break;

    case 'h':
     puts ("option -h\n");
     printf("argv[0]: %s\n", argv[0]);
     help(argv[0]);
     break;

    case 'i':
     puts ("option -i\n");
     break;

    case 't':
     puts ("option -t\n");
     break;

    case 'c':
     printf ("option -c with value `%s'\n", optarg);
     break;

    case 'v':
     printf ("option -v with value `%s'\n", optarg);
     break;

    case 'u':
     printf ("option -u  with value `%s'\n", optarg);
     break;

    case 'w':
     printf ("option -v with value `%s'\n", optarg);
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
  if (optind < argc)
   {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
     printf ("%s ", argv[optind++]);
     putchar ('\n');
   }

 /*------------------------------------------------------*/

 /* Set uid, gid, euid and egid to root */
 setegid(0);
 seteuid(0);
 setgid(0);
 setuid(0);

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

void help(char *name)
 {
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
