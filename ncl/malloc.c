// Malloc & Free, by Strick.
//
// Raise requested malloc size to the next power of two.
// There is a free list for each power of two.
// So if random sizes are used, we waste perhaps 25%.
// But it is fast.

#include "frobio/ncl/malloc.h"
#include "frobio/ncl/puthex.h"

#define ZERO_MALLOC             // catch bugs faster.
#define ZERO_FREE               // catch bugs faster.
// #define AUDIT_MALLOC_FREE  // for leak and unmatched malloc/free detection.

// Heap boundaries.
word heap_min;          // Set by start code to bss_end.
word heap_brk;          // Set by start code to bss_end.
word heap_max;          // Set by every stkcheck().

struct MallocHead *buck_freelist[NBUCKETS];
int buck_num_alloc[NBUCKETS];
int buck_num_free[NBUCKETS];
int buck_num_brk[NBUCKETS];

void heap_check_block(struct MallocHead *h, int cap) {
  // pc_trace('?', (char*)h);
  if (h->barrierA != 'A' || h->barrierZ != 'Z' || (cap && h->cap != cap)) {
    puthex('h', h);
    puthex('A', h->barrierA);
    puthex('Z', h->barrierZ);
    puthex('c', h->cap);
    puthex('C', cap);
    pc_trace('*', (char *) h);
    panic("corrupt heap");
  }
}

byte which_bucket(int n, int *capP) {
  byte b;
  int cap = SMALLEST_BUCKET;
  for (b = 0; b < NBUCKETS; b++) {
    if (n <= cap)
      break;
    cap += cap;
  }
  if (b >= NBUCKETS) {
    puthex('m', n);
    panic("malloc too big");
  }
  *capP = cap;
  return b;
}

#if 0
void ShowChains() {
  for (byte b = 0; b < NBUCKETS; b++) {
    printf_d("Bucket [%d]: ", b);
    for (struct MallocHead * p = buck_freelist[b]; p; p = p->next) {
      puthex('=', p);
    }
    puts("\r");
  }
  puts("\r");
}
#endif

char *malloc(int n) {
  int cap;
  byte b = which_bucket(n, &cap);
  buck_num_alloc[b]++;

  // Try an existing unused block.

  struct MallocHead *h = buck_freelist[b];
  if (h) {
    h->cap = cap;
    heap_check_block(h, cap);
    buck_freelist[b] = h->next;
#ifdef ZERO_MALLOC
    memset((char *) (h + 1), 0, cap);
#endif
#ifdef AUDIT_MALLOC_FREE
    pc_trace('M', (char *) h);
#endif
    return (char *) (h + 1);
  }

  // Break fresh memory.
  char *p = (char *) heap_brk;
  heap_brk += (word) (cap + sizeof(struct MallocHead));
  if (heap_brk > heap_max - STACK_MARGIN) {
    puthex('n', n);
    puthex('c', cap);
    puthex('u', heap_brk);
    puthex('m', heap_max);
    panic(" *oom* ");
  }
  buck_num_brk[b]++;

  h = ((struct MallocHead *) p);
  h->barrierA = 'A';
  h->barrierZ = 'Z';
  h->cap = cap;
  h->next = NULL;
#ifdef ZERO_MALLOC
  memset((char *) (h + 1), 0, cap);
#endif
#ifdef AUDIT_MALLOC_FREE
  pc_trace('M', (char *) h);
#endif
  return (char *) (h + 1);
}

void free(void *p) {
  if (!p)
    return;

  struct MallocHead *h = ((struct MallocHead *) p) - 1;
  if (!h->cap) {                // TODO -- because double-frees.
    panic("DoubleFree");
    return;
  }
  int cap;
  byte b = which_bucket(h->cap, &cap);
  heap_check_block(h, cap);
  buck_num_free[b]++;

#ifdef ZERO_FREE
  memset((char *) p, 0, cap);
#endif
  h->cap = 0;                   // TODO -- because double-frees.
  h->next = buck_freelist[b];
  buck_freelist[b] = h;
#ifdef AUDIT_MALLOC_FREE
  pc_trace('F', (char *) h);
#endif
}

char *realloc(void *p, int n) {
  if (!p)
    return malloc(n);
  struct MallocHead *h = ((struct MallocHead *) p) - 1;
  if (n <= h->cap) {
    return (char *) p;
  }

  char *z = malloc(n);
  memcpy(z, p, h->cap);
  free(p);
  return z;
}
