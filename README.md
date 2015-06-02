# udocker

Wrapper to enforce some docker command-line options in order to allow the user to run docker as non-root.

* force uid so that the container will run under this user privilege
* automatically map user home directory and present it as a volume to the container
* allow interactive container as well
* force --rm option to remove the container on exit
* many other options are disabled

The wrapper must run as suid and owned by root
