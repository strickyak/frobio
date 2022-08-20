// linq.c -- doubly-linked lists, using links.

#include "frobio/linq.h"

// TODO -- remove this debugging verbosity.
#define LOG   printf(" Q#%d ", __LINE__)

#define QP(LINK) ((Queue*)Pointer(LINK))
#define L(PTR) Link((word)(PTR))

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
    Queue* it = QP(root->next);
    word f = 0;
    word root_link = L(root);
    while (it->next != root_link) {
        QCheckItem(it);
        ++f;
        it = QP(it->next);
    }
    // Now check backwards.
    it = QP(root->prev);
    word b = 0;
    while (it->prev != root_link) {
        QCheckItem(it);
        ++b;
        it = QP(it->prev);
    }
    // Check same length frontwards as backwards.
    assert(f == b);
}

void QInit(Queue*root) {
    root->prev = root->next = L(root);
    root->guard8 = QGUARD8ROOT;
    root->guard9 = QGUARD9ROOT;
    QCheck(root);
}

void QAppend(Queue*root, void* item) {
    QCheck(root);
    Queue* it = (Queue*)item;
    Queue* prev = QP(root->prev);

    it->prev = L(prev);
    prev->next = L(it);

    it->next = L(root);
    root->prev = L(it);
    
    it->guard8 = QGUARD8ITEM;
    it->guard9 = QGUARD9ITEM;
    LOG; QCheckItem(it);
}

void QRemove(Queue*root, void* item) {
    LOG; QCheck(root);
    LOG; QCheckItem(item);
    Queue* it = (Queue*)item;
LOG; 
    // Link the chain around the item.
    QP(it->prev)->next = L(it->next);
    QP(it->next)->prev = L(it->prev);

    // Invalidate guards.
    it->guard8 = 8;
    it->guard9 = 9;
}

word QLen(Queue* root) {
    QCheck(root);
    if (QP(root->next) == root) return 0;
    Queue* it = QP(root->next);
    word n = 1;  // already did one ->next
    while (QP(it->next) != root) {
        ++n;
        it = QP(it->next);
    }
    return n; 
}

void* QFirst(Queue* root) {
    QCheck(root);
    if (QP(root->next) == root) return NULL;
    return QP(root->next);
}
void* QNext(Queue* root, void* item) {
    QCheck(root);
    Queue* it = (Queue*)item;
    if (QP(it->next) == root) return NULL;
    return QP(it->next);
}
