#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <devid.h>
#include <libdevinfo.h>

#define   DISK_DRIVER    "sd" /* Disk driver name */
#define   MPXIO_DRIVER    "scsi_vhci" /* MPxIO driver name */
#define   ISCSI_DRIVER    "iscsi" /* iSCSI Initiator driver name */

int my_devpath_compare(const void *arg1, const void *arg2);

static int DEBUG = 0;

int
add2Array(char ***array, char *element, int *index, int *array_size)
{
	int arr_increment = 50;
	int i = *index;
	int sz = *array_size;
	char **arr = *array;

	if (i >= sz) {
	    sz += arr_increment;
	    arr = (char **)realloc(arr, sz * sizeof(char *));
	}
	if (arr == NULL) {
	    return (ENOMEM);
	}
	arr[i] = strdup(element);
	if (arr[i++] == NULL) {
	   return (ENOMEM);
	}

	*index = i;
	*array_size = sz;
	*array = arr;
	return (0);
}

int
walk_disknodes(di_node_t node, char ***xDevIdArray, int *xSize)
{
      char *phys_path = NULL;
      di_node_t phci_node, child;
      di_path_t phci_path;
      char **pathArray = NULL;
      int index = 0, arr_sz = 100, i;

      node = di_drv_first_node(ISCSI_DRIVER, node);
      while (node != DI_NODE_NIL) {
	    /*
	    * If the device node exports no minor nodes,
	    * there is no physical disk.
	    */
	   if (di_minor_next(node, DI_MINOR_NIL) == DI_MINOR_NIL) {
	        node = di_drv_next_node(node);
		continue;
	   }

	   pathArray = malloc(arr_sz * sizeof(char *));
	   /* Get Normal iSCSI Nodes */
	   child = di_child_node(node);
	   while (child != DI_NODE_NIL) {
		phys_path = di_devfs_path(child);
		if (phys_path != NULL) {
		   if (add2Array(&pathArray, phys_path, &index, &arr_sz) != 0) {
		       goto error;
		   }
		   di_devfs_path_free(phys_path);
		   phys_path = NULL;
		}
	        child = di_sibling_node(child);
	   }

	   /* Get iSCSI Nodes used in MPxIO Configurations */
	   phci_path = di_path_next_client(node, DI_PATH_NIL);
	   while (phci_path != NULL) {
	       phci_node = di_path_client_node(phci_path);
	       if (phci_node != DI_NODE_NIL) {
		   phys_path = di_devfs_path(phci_node);
		   if (phys_path != NULL) {
		       if (add2Array(&pathArray, phys_path, &index,
				   &arr_sz) != 0) {
			   goto error;
		       }
		       di_devfs_path_free(phys_path);
		       phys_path = NULL;
		   }
	       }
	       phci_path = di_path_next_client(node, phci_path);
	   }

	   node = di_drv_next_node(node);
      }
      qsort(pathArray, index, sizeof(char*), my_devpath_compare);
error:
      if (phys_path != NULL) {
	    di_devfs_path_free(phys_path);
	    for (i = 0; i < index; i++) {
		free(pathArray[i]);
	    }
	    free(pathArray);
	    index = 0;
	    pathArray = NULL;
	    printf("Execution must NOT come here\n");
      }
      *xDevIdArray = pathArray;
      *xSize = index;
      return (0);
}

int
my_devpath_compare(const void *arg1, const void *arg2) {
    const char **carg1, **carg2;
    carg1 = (const char**)arg1;
    carg2 = (const char**)arg2;
    return (strcmp(*carg1, *carg2));
}

int
main(int argc, char *argv[])
{
    di_node_t root_node;
    ddi_devid_t *array_ptr;
    char **path_array_ptr = NULL;
    int sz;
    char *devices_path = NULL;
    int path_len;
    char *dev_path;
    char *lasts;

    if ((root_node = di_init("/", DINFOSUBTREE|DINFOMINOR|DINFOPATH))
	    == DI_NODE_NIL) {
      fprintf(stderr, "di_init() failed\n");
      exit(1);
    }
    printf("di_init() success\n");
    walk_disknodes(root_node, &path_array_ptr, &sz);
    di_fini(root_node);

    printf("%d iSCSI devices\n", sz);
    for (int i=0; i < sz; i++) {
	printf("Array Element Valid %s\n", path_array_ptr[i]);
    }

    devices_path = malloc(MAXPATHLEN);
    if ((path_len = resolvepath(argv[1], devices_path, MAXPATHLEN)) > 0) {
	devices_path[path_len] = '\0';
	dev_path = strrchr(devices_path, ':');
	*dev_path = '\0';
	dev_path = strtok_r(devices_path, "/", &lasts);
	snprintf(devices_path, MAXPATHLEN, "/%s", lasts);
	printf("Device :%s\n", devices_path);
	if (bsearch(&devices_path, path_array_ptr, sz, sizeof(char*),
		my_devpath_compare) != NULL) {
	    printf("\n%s is iSCSI device\n", argv[1]);
	} else {
	    printf("\n%s is NOT a iSCSI device\n", argv[1]);
	}
    }

}
