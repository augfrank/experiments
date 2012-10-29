#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <strings.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define RUNNING_DIR "/tmp"
#define LOCK_FILE   "daemon.lock"
#define LOG_FILE    "daemon.log"

void 
log_message(char *filename, char *message)
{
    syslog(LOG_ERR, message);
}

void 
signal_handler(int sig)
{
    switch(sig) {
		case SIGHUP:
			log_message(LOG_FILE,"hangup signal catched");
			break;
		case SIGTERM:
			log_message(LOG_FILE,"terminate signal catched");
			exit(0);
		break;
    }
}

void 
daemonize()
{
    int i,lfp;
    char str[10];

    if(getppid() == 1) 
	return;
    i=fork();
    if (i<0) exit(1); /* fork error */
    if (i>0) exit(0); /* parent exits */

    /* child (daemon) continues */
    setsid(); /* obtain a new process group */

    /* close standard file descriptors */
    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);

    i = open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standard I/O */
    umask(027); /* set newly created file permissions */
    chdir(RUNNING_DIR); /* change running directory */
    lfp = open(LOCK_FILE, O_RDWR|O_CREAT, 0640);
    if (lfp<0) 
		exit(1); /* can not open */
    if (lockf(lfp,F_TLOCK,0)<0) {
		log_message(LOG_FILE, "Daemon is already running. Exiting..");
		exit(0); /* can not lock */
    }

    /* first instance continues */
    sprintf(str,"%d\n",getpid());
    write(lfp,str,strlen(str)); /* record pid to lockfile */
    signal(SIGCHLD,SIG_IGN); 
    signal(SIGTSTP,SIG_IGN); 
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,signal_handler); 
    signal(SIGTERM,signal_handler); 
}

int 
main(int argc, char* argv[])
{
    daemonize();
    while(1) {
	/* 
     *
	 */
	sleep(30); 

   }
    exit(-1);
}
