#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define DID_RELOADER_CMD "/usr/cluster/lib/sc/did_reloader"
int
main(int argc, char* argv[])
{
    FILE *p;
    int iscsi_devcount = argc-2;
    (void) printf("XXX %s called %d\n", argv[1], iscsi_devcount);
    if (iscsi_devcount > 0){
        if ((p = popen(argv[1], "w")) == NULL) {
            (void) printf("XXX %s cannot be started\n", argv[1]);
            return -1;
        }
        for (int i = 2; i < argc; i++){
	    printf ("arg %d:%s\n", i, argv[i]);
            if (fprintf(p, "%s\n", argv[i]) < 0)
                (void) printf("XXX fprintf failed");
        }
        if (pclose(p) < 0) {
                (void) printf("XXX pclose failed");
        }
    }  else {
                (void) printf("XXX iscsi_devcount <=0 ");
    }
    return 1;
}

