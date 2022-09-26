// linq.h -- doubly-linked lists, using links.

#ifndef _FROBIO_LINQ_H_
#define _FROBIO_LINQ_H_

#include "frobio/nytypes.h"
#include "frobio/shmem.h"

#define QGUARD8ITEM 0x83
#define QGUARD9ITEM 0x93
#define QGUARD8ROOT 0x85
#define QGUARD9ROOT 0x95

typedef struct queue {
    byte guard8;
    word next;  // link to Queue
    word prev;  // link to Queue
    byte guard9;
} Queue;

void QInit(Queue*root);
void QAppend(Queue*root, void* item);
void QRemove(Queue*root, void* item);

word QLen(Queue* root);

// These iterate forwards and return NULL at the end.
void* QFirst(Queue* root);
void* QNext(Queue* root, void* item);

// Sanity checks.
void QCheck(Queue* root);
void QCheckRoot(Queue* root);
void QCheckItem(void* item);

#endif // _FROBIO_LINQ_H_
