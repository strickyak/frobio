// queue.c -- doubly-linked lists.

#include "frobio/queue.h"

// TODO -- remove this debugging verbosity.
#define L   printf(" Q#%d ", __LINE__)

void QCheckRoot(Queue* root) {
    assert(root->guard8 == QGUARD8ROOT);
    assert(root->guard9 == QGUARD9ROOT);
    assert(root->next);
    assert(root->prev);
}

void QCheckItem(void* item) {
    Queue* it = (Queue*)item;
    assert(it->guard8 == QGUARD8ITEM);
    assert(it->guard9 == QGUARD9ITEM);
    assert(it->next);
    assert(it->prev);
}

void QCheck(Queue* root) {
    QCheckRoot(root);
    // Check frontwards.
    Queue* it = root->next;
    word f = 0;
    while (it->next != root) {
        QCheckItem(it);
        ++f;
        it = it->next;
    }
    // Now check backwards.
    it = root->prev;
    word b = 0;
    while (it->prev != root) {
        QCheckItem(it);
        ++b;
        it = it->prev;
    }
    // Check same length frontwards as backwards.
    assert(f == b);
}

void QInit(Queue*root) {
    root->prev = root->next = root;
    root->guard8 = QGUARD8ROOT;
    root->guard9 = QGUARD9ROOT;
    QCheck(root);
}

void QAppend(Queue*root, void* item) {
    QCheck(root);
    Queue* it = (Queue*)item;
    Queue* prev = root->prev;

    it->prev = prev;
    prev->next = it;

    it->next = root;
    root->prev = it;
    
    it->guard8 = QGUARD8ITEM;
    it->guard9 = QGUARD9ITEM;
    L; QCheckItem(it);
}

void QRemove(Queue*root, void* item) {
    L; QCheck(root);
    L; QCheckItem(item);
    Queue* it = (Queue*)item;
L; 
    // Link the chain around the item.
    it->prev->next = it->next;
    it->next->prev = it->prev;

    // Invalidate guards.
    it->guard8 = 8;
    it->guard9 = 9;
}

word QLen(Queue* root) {
    QCheck(root);
    if (root->next == root) return 0;
    Queue* it = root->next;
    word n = 1;  // already did one ->next
    while (it->next != root) {
        ++n;
        it = it->next;
    }
    return n; 
}

void* QFirst(Queue* root) {
    QCheck(root);
    if (root->next == root) return NULL;
    return root->next;
}
void* QNext(Queue* root, void* item) {
    QCheck(root);
    Queue* it = (Queue*)item;
    if (it->next == root) return NULL;
    return it->next;
}
