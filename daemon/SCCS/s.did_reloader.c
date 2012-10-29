h51306
s 00319/00000/00000
d D 1.1 08/08/11 17:44:27 augustus 1 0
c date and time created 08/08/11 17:44:27 by augustus
e
u
U
f e 0
t
T
I 1
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

#define RUNNING_DIR "/tmp"
#define LOCK_FILE   "did_reloader.lock"
#define LOG_FILE    "did_reloader.log"
/*
 * didadm options
 *   -u upload the path information to the driver
 *   -i initialize the did driver
 */
#define DIDADM_CMD    "/usr/sbin/didadm -u -i"

typedef struct _iscsi_dev {
    char *dev_path;
    struct _iscsi_dev *next;
} iscsi_dev;

static iscsi_dev *iscsi_devices = NULL;
static iscsi_dev *last = NULL;

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
    if (lockf(lfp,F_TLOCK,0)<0) 
	exit(0); /* can not lock */

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
update_did_drv()
{
  FILE *p;

   /* 
    * This code needs to get smarter so that the 
    * command is run only when there are did instances
    * that did not load. Otherwise this should be a noop 
    */
  log_message(LOG_FILE,"update_did_drv called");
  if ((p = popen(DIDADM_CMD, "w")) == NULL)
	  return (-1);
  return (pclose(p));
}

static void
event_handler(sysevent_t *ev)
{
    nvlist_t *nvlist;
    const char *class_name, *subclass;
    sysevent_value_t se_val;
    sysevent_t *local_ev = NULL;

    FILE *logfile;

    logfile = fopen(LOG_FILE,"a");
    if(!logfile) 
	return;
    se_print(logfile, ev);
    fclose(logfile);
    // sysevent_dup() dup unpacks the event and allows for query 
    // operations on its name-value pairs
    local_ev = sysevent_dup(ev);
    class_name = (const char*)sysevent_get_class_name(local_ev);
    subclass = (const char*)sysevent_get_subclass_name(local_ev);
    if (strcmp(EC_DEVFS, class_name) == 0) {
	if (strcmp(ESC_DEVFS_DEVI_ADD, subclass) == 0) {
	    log_message(LOG_FILE, "Got EC_DEVFS/ESC_DEVFS_DEVI_ADD");
	    if (update_did_drv() < 0) {
		 log_message(LOG_FILE,"DID Driver Update Failed");
	    } else {
		 log_message(LOG_FILE,"DID Driver Update Done");
	    }
	    if (sysevent_lookup_attr(local_ev, DEVFS_PATHNAME,
		SE_DATA_TYPE_STRING, &se_val) == 0 &&
		se_val.value.sv_string != NULL) {
		log_message(LOG_FILE, "sysevent_lookup_attr success");
		log_message(LOG_FILE, se_val.value.sv_string);
	    } else {
		log_message(LOG_FILE, "sysevent_lookup_attr failure");
	    }
	}
    }
    sysevent_free(local_ev);
}

int
subscribe_sysevents()
{
    sysevent_handle_t *shp;
    const char *devfs_subclasses[] = {
	ESC_DEVFS_DEVI_ADD,
	};

    /* Bind event handler and create subscriber handle */
    log_message(LOG_FILE,"subscribe_sysevents called");
    shp = sysevent_bind_handle(event_handler);
    if (shp == NULL) {
	log_message(LOG_FILE, "Event bind handle failed");
	return(-1);
    }

    /* Subscribe to EC_DEVFS event notifications */
    if (sysevent_subscribe_event(shp, EC_DEVFS, devfs_subclasses,
	sizeof (devfs_subclasses)/sizeof (devfs_subclasses[0])) != 0) {
	log_message(LOG_FILE, "EC_DEVFS Event subscription failed");
	sysevent_unbind_handle(shp);
	return(-1);
    }
    return 0;
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
free_dev(iscsi_dev *dev) {
    if (dev) {
	if (dev->dev_path) {
	    free(dev->dev_path);
	    dev->dev_path = NULL;
	}
	free(dev);
    }
    dev = NULL;
}


void 
device_monitor()
{
    iscsi_dev *dev = iscsi_devices;
    iscsi_dev *previous = NULL;
    while (dev != NULL) {
	if (query_device(dev->dev_path) == 0) {
	    log_message(LOG_FILE,"Success for :");
	    log_message(LOG_FILE,dev->dev_path);
	    //remove this device from the list
	    iscsi_dev *next_ptr = NULL;
	    next_ptr = dev->next;
	    if (previous != NULL) {
		previous->next = next_ptr;
		dev->next = NULL;
	    } else {
		iscsi_devices = next_ptr;
	    }
	    free_dev(dev);
	    dev = next_ptr;
	} else {
	    // Go to the next element in the list
	    log_message(LOG_FILE,"Failure for :");
	    log_message(LOG_FILE,dev->dev_path);
	    previous = dev;
	    dev = dev->next;
	}
    }
    if (iscsi_devices == NULL) {
	//No devices to monitor, stop the daemon
	 log_message(LOG_FILE,"DID Reloader is complete. Exiting...");
	 exit(1);
    }
}

void
add_to_list(char *path) {

    int len = 0;
    if (path == NULL) {
	return;
    }
    len = strlen(path);
    if (len <=0) {
	return;
    }
    iscsi_dev *new = (iscsi_dev*)malloc(sizeof(iscsi_dev*));
    if (new == NULL) {
	 log_message(LOG_FILE,"Out of Memory");
	 exit(1);
    }
    new->dev_path = (char*) malloc(len);
    if (new->dev_path == NULL) {
	 log_message(LOG_FILE,"Out of Memory");
	 exit(1);
    }

    // Remove the null termination character in the string. If it is
    // not removed stat() will fail. 
    strncpy(new->dev_path, path, (len - 1));
    new->next = NULL;

    if (iscsi_devices == NULL) {
	iscsi_devices = new;
	last = iscsi_devices;
    } else {
	last->next = new;
	last = new;
    }
}

void 
main(int argc, char* argv[])
{
    char buf[MAXPATHLEN];
    if (argc <=0 ) {
	 log_message(LOG_FILE,"No devices to load");
	 exit(1);
    }
    while (fgets(buf, sizeof(buf), stdin) != NULL){
	add_to_list(buf);
    }
    daemonize();
    if (subscribe_sysevents() < 0) {
	 log_message(LOG_FILE,"Event subsciption failed");
	 exit(1);
    };
    while(1) {
	/* 
	 * This time interval is arbitrary at this point. This may
	 * need fine tuning.
	 */
	device_monitor();
	sleep(30); 

   }
}
E 1
