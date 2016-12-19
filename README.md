# udocker

A wrapper to enforce some `docker` command-line options in order to allow the user to run `docker` as non-root. (Docker requires root privilege to start a container in `docker run`).

Please refer to Docker's official documentation and tutorials on the various commands, such as `docker run`, `docker images`, `docker ps -a`, `docker save`, `docker load` for more information.

`udocker` will:

* force uid so that the container will run under this user privilege
* map /etc/sudoers to /dev/null to disable sudo su
* automatically map user home directory and present it as a volume to the container
* force work dir to be user's home dir
* force interactive container, i.e. -d is not allowed and -i forced
* user can specify -t if a console session is desired
* force --rm option to remove the container on exit
* many other options are disabled

The wrapper must run as suid and owned by root.
```
 Usage: ./udocker [ps|images|pull|run|load|save] options -- image command

 Minimumm wrapper for docker run command to run as non-root
 Options are mainly for the 'docker run` command.

  -h|--help          print this help text
  --verbose          just echo the command string
  -it|d              run in interactive mode with a pseudo terminal
                     or in background detached mode
  --rm               Container to be removed at exit
  --name [container-name]
                     Name to be given to the container
  -v|--volumes-from [host-dir]:[container-dir]:[rw|ro]
                     Map host directories
  -u|--user [Username|UID]
  -e KEY=value       Export environment variable
  -P                 Expose all ports
  -p nnnn            port number to expose
  -w [dir]           Working directory inside the container
  --input image_file for docker load option
  --output image_file for docker save option

 image:              docker image to be loaded
 command:            command to be started upon container launch
```  
## Installation instruction

* Clone this repo
* Compile: `gcc udocker.c -o udocker`
* Copy to `/usr/bin`: `sudo cp udocker /usr/bin`
* Make it suid: `sudo chmod a+sx /usr/bin/udocker`

