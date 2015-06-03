# udocker

Wrapper to enforce some docker command-line options in order to allow the user to run docker as non-root.

* force uid so that the container will run under this user privilege
* automatically map user home directory and present it as a volume to the container
* allow interactive container as well
* force --rm option to remove the container on exit
* many other options are disabled

The wrapper must run as suid and owned by root.
```
Usage: /usr/local/bin/udocker [ps|images|run] options image command

 Minimumm wrapper for docker run command to run as non-root
 Options are mainly for the 'docker run` command.

  -h|--help          print this help text
  --verbose          just echo the command string
  -it                run in interactive mode with a pseudo terminal
  --rm               Container to be removed at exit
  --name [container-name]
                     Name to be given to the container
  -v|--volumes-from [host-dir]:[container-dir]:[rw|ro]
                     Map host directories
  -u|--user [Username|UID]
  -w [dir]           Working directory inside the container
```  

 image:              docker image to be loaded
 command:            command to be started upon container launch
