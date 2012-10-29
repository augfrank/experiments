#ifndef _DID_RELOADER_H
#define _DID_RELOADER_H

typedef struct _iscsi_dev {
    char *dev_path;
    struct _iscsi_dev *next;
} iscsi_dev;

typedef struct _list {
    iscsi_dev *head;
    iscsi_dev *tail;
    int size;
} LinkList;

int initList(LinkList **xList);
int insertAfter(LinkList *xList, iscsi_dev *element, char *device_path);
void deleteNext(LinkList *xList, iscsi_dev *element);
void moveElementAfter(LinkList *xSourceList, iscsi_dev *element,
	LinkList *xDestList);
static void freeElem(iscsi_dev *element);
void freeList(LinkList *xList);
#endif //_DID_RELOADER_H
