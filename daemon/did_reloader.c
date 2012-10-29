#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <strings.h>
#include <syslog.h>
#include <errno.h>
#include <libsysevent.h>
#include <sys/nvpair.h>
#include <sys/sunddi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysevent/dr.h>
#include <sys/sysevent/eventdefs.h>
#include <sys/sysevent_impl.h>
#include "did_reloader.h"

#define RUNNING_DIR "/tmp"
#define LOCK_FILE   "did_reloader.lock"
#define LOG_FILE    "did_reloader.log"
/*
 * didadm options
 *   -a reload the did driver for the given set of disk paths
 */
#define DIDADM_CMD    "/usr/sbin/didadm -a"

static LinkList *all_devices = NULL;
static LinkList *success_list = NULL;

void 
log_message(char *filename, char *message)
{
    FILE *logfile;

    logfile = fopen(filename,"a");
    if(!logfile) 
	return;
    fprintf(logfile,"%s\n",message);
    fclose(logfile);
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
    if (i<0) 
	exit(1); /* fork error */
    if (i>0) 
	exit(0); /* parent exits */

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
update_did_drv(LinkList *xSuccessList)
{
  FILE *p;
  int ret = -1;
  char *cmd = NULL;
  int argsize = 0;
  iscsi_dev *dev = xSuccessList->head;
  int size = xSuccessList->size;

  log_message(LOG_FILE,"update_did_drv called");
  argsize = size * (MAXPATHLEN + 1);
  cmd = (char *)malloc(argsize + strlen(DIDADM_CMD));
  if (cmd == NULL) return ret;

  sprintf(cmd, "%s", DIDADM_CMD);
  while (dev->next != dev->next->next) {
      if (dev->next->dev_path != NULL)
	  sprintf(cmd, "%s %s", cmd, dev->next->dev_path);
      dev = dev->next;
  }
  log_message(LOG_FILE,cmd);
  if ((p = popen(cmd, "w")) == NULL) {
      free(cmd);
      return ret;
  }
  ret = pclose(p);

  free(cmd);
  return ret;
}

int
query_device(const char *path)
{
    int fd;
    int status;
    struct  stat    st;

    // path should not null. If it is ignore it and 
    // it will get removed
    if (path == NULL){
	return 0;
    }

    // the stat call tickles the device and causes iscsi devices
    // to be attached to the host 
    if (stat(path, &st) == 0) {
	return 0;
    } else {
	return -1;
    }
}

void 
device_monitor()
{
    iscsi_dev *dev = all_devices->head;
    iscsi_dev *next = NULL;
    while (dev->next != dev->next->next) {
	if (query_device(dev->next->dev_path) == 0) {
	    log_message(LOG_FILE,"Success for :");
	    log_message(LOG_FILE,dev->next->dev_path);
	    //move this device to the success list
	    moveElementAfter(all_devices, dev, success_list);
	} else {
	    // Go to the next element in the list
	    log_message(LOG_FILE,"Failure for :");
	    log_message(LOG_FILE,dev->next->dev_path);
	    dev = dev->next;
	}
    }
    // Load the DID for all the devices that are now accessible
    if (success_list->size > 0) {
	if (update_did_drv(success_list) < 0) {
	    log_message(LOG_FILE,"DID Driver Repair Failed");
	}
    }
    if (all_devices->size == 0) {
	//No devices to monitor, stop the daemon
	freeList(success_list);
	freeList(all_devices);
	log_message(LOG_FILE,"DID Reloader is complete. Exiting...");
	exit(0);
    }
}

void 
main(int argc, char* argv[])
{
    char buf[MAXPATHLEN];

    if (initList(&all_devices) < 0 ) {
	 log_message(LOG_FILE,"Not Enough memory. DID Reloader Exiting...");
	 goto cleanup;
    }
    if (initList(&success_list) < 0 ) {
	 log_message(LOG_FILE,"Not Enough memory. DID Reloader Exiting...");
	 goto cleanup;
    }
    while (fgets(buf, sizeof(buf), stdin) != NULL){
	if (insertAfter(all_devices, NULL, buf) < 0) {
	     log_message(LOG_FILE,"Not Enough memory. DID Reloader Exiting...");
	     goto cleanup;
	}
    }
    daemonize();
    while(1) {
	/* 
	 * This time interval is arbitrary at this point. This may
	 * need fine tuning.
	 */
	device_monitor();
	sleep(30); 

   }
cleanup:
    freeList(success_list);
    freeList(all_devices);
    exit(-1);
}
