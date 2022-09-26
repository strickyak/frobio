#include "frobio/queue.h"

Queue q1, q2;

typedef struct thing { Queue links; int x; } T;

T t1, t2, t3, t4, t5;

int main() {
    t1.x = 100;
    t2.x = 200;
    t3.x = 303;
    t4.x = 400;
    t5.x = 550;

    QInit(&q1);
    QAppend(&q1, &t1);
    QAppend(&q1, &t2);
    QAppend(&q1, &t3);
    QAppend(&q1, &t4);
    QAppend(&q1, &t5);

    printf("QLen=%d\n", QLen(&q1));
    assert(QLen(&q1) == 5);
    assert(QFirst(&q1) == &t1);
    assert(QNext(&q1, &t1) == &t2);
    assert(QNext(&q1, &t2) == &t3);
    assert(QNext(&q1, &t5) == NULL);

#define L   printf(" #%d ", __LINE__)
L;
    L; QInit(&q2);
    void* next;
    for (void* it = QFirst(&q1); it; it = next) {
        next = QNext(&q1, it);  // grab next before we remove it.

        L; QCheckItem(it);
        L; QRemove(&q1, it);
        L; QCheck(&q1);
        L; QCheck(&q2);
        L; QAppend(&q2, it);
        L; QCheck(&q1);
        L; QCheck(&q2);
        L; QCheckItem(it);
    }
    int sum = 0; L;
    for (void* it = QFirst(&q2); it; it = QNext(&q2, it)) {
        L; sum += ((T*)it)->x;
    }
    assert(sum == 1553);

    return 0;
}
