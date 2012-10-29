#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "did_reloader.h"

/*
*/
int
initList(LinkList **xList) {

    *xList = (LinkList *)malloc(sizeof (struct _list));
    if (!xList) return -1;
    LinkList *zlist = *xList;
    zlist->head = (iscsi_dev *)malloc(sizeof(struct _iscsi_dev));
    zlist->tail = (iscsi_dev *)malloc(sizeof(struct _iscsi_dev));
    if (!zlist->head || !zlist->tail) return -1;
    zlist->head->next = zlist->tail;
    zlist->tail->next = zlist->tail;
    zlist->size = 0;

    return 0;
}

/*
 * If "element" parameter is NULL, the function removes the element 
 * from the head of the list
 */
int
insertAfter(LinkList *xList, iscsi_dev *element, char *device_path) {

    iscsi_dev *newElement = NULL;
    int len = 0;

    if (!device_path) return -1;
    newElement = (iscsi_dev *)malloc(sizeof (struct _iscsi_dev));
    if (!newElement) return -1;

    len = strlen(device_path);
    newElement->dev_path = (char*)malloc(len - 1);
    if (!newElement->dev_path) {
	freeElem(newElement);
	return -1;
    }
    // Remove the null termination character in the string. If it is
    // not removed stat() will fail. 
    strncpy(newElement->dev_path, device_path, (len - 1));

    if (element == NULL) {
	element = xList->head;
    }
    newElement->next = element->next;
    element->next = newElement;
    xList->size++;

    return 0;
}

/*
*/
void
deleteNext(LinkList *xList, iscsi_dev *element) {

    iscsi_dev *deleteElem = NULL;

    deleteElem = element->next;
    element->next = deleteElem->next;

    freeElem(deleteElem);
    xList->size--;
}

/*
 * returns the next element in the source list after the moved element
 */
void
moveElementAfter(LinkList *xSourceList, iscsi_dev *element,
	LinkList *xDestList) {

    iscsi_dev *toMove = NULL;

    toMove = element->next;
    element->next = element->next->next;
    xSourceList->size--;

    toMove->next = xDestList->head->next->next;
    xDestList->head->next = toMove;
    xDestList->size++;

}

/*
*/
static void
freeElem(iscsi_dev *element) {
    if (!element) return;
    if (element->dev_path)
	free(element->dev_path);
    element->next = NULL;
    free(element);
    element = NULL;
}

/*
*/
void
freeList(LinkList *xList) {

    if (!xList) return;

    iscsi_dev *dev = xList->head;
    while (dev->next != dev->next->next) {
	deleteNext(xList, dev);
    }
    freeElem(xList->head);
    freeElem(xList->tail);
    free(xList);
}

