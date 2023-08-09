// Malloc & Free, by Strick.
//
// Raise requested malloc size to the next power of two.
// There is a free list for each power of two.
// So if random sizes are used, we waste perhaps 25%.

#include "frob3/froblib.h"
#include "frob3/froblib/malloc.h"
#include "frob3/frobos9.h"

#define ZERO_MALLOC             // catch bugs faster.
#define ZERO_FREE               // catch bugs faster.
// #define AUDIT_MALLOC_FREE  // for leak and unmatched malloc/free detection.

struct Heap {
    word h_heap_min;
    word h_heap_here;
    word h_heap_max;
    bool h_heap_retry;  // Not thread-safe.

    struct MallocHead *h_buck_freelist[NBUCKETS];
    int h_buck_num_alloc[NBUCKETS];
    int h_buck_num_free[NBUCKETS];
    int h_buck_num_brk[NBUCKETS];
};

#ifdef FOR_FIX
#define HEAP ((struct Heap *)0x100)
#else
struct Heap TheHeap;
#define HEAP (&TheHeap)
#endif

#define heap_min  (HEAP->h_heap_min)
#define heap_here  (HEAP->h_heap_here)
#define heap_max  (HEAP->h_heap_max)
#define heap_retry  (HEAP->h_heap_retry)

#define buck_freelist  (HEAP->h_buck_freelist)
#define buck_num_alloc  (HEAP->h_buck_num_alloc)
#define buck_num_free  (HEAP->h_buck_num_free)
#define buck_num_brk  (HEAP->h_buck_num_brk)

#ifdef unix
extern byte MemoryPoolForUnix[1024000];
#endif

// forward
extern void MallocOOM(errnum e, word n, word cap);
extern byte which_bucket(int n, int *capP);
extern void heap_check_block(struct MallocHead *h, int cap);

// chop

#ifdef unix
byte MemoryPoolForUnix[1024000];
#endif

void MallocOOM(errnum e, word n, word cap) {
    PutHex('e', e);
    PutHex('n', n);
    PutHex('c', cap);
    PutHex('u', heap_here);
    PutHex('m', heap_max);
    Panic(" *oom* ");
}

void heap_check_block(struct MallocHead *h, int cap) {
  // PC_Trace('?', (char*)h);
  if (h->barrierA != 'A' || h->barrierZ != 'Z' || (cap && h->cap != cap)) {
    PutHex('h', (word)h);
    PutHex('A', h->barrierA);
    PutHex('Z', h->barrierZ);
    PutHex('c', h->cap);
    PutHex('C', cap);
    PC_Trace('*', (char *) h);
    Panic("corrupt heap");
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
    PutHex('m', n);
    Panic("malloc too big");
  }
  *capP = cap;
  return b;
}

#if 0
void ShowChains() {
  for (byte b = 0; b < NBUCKETS; b++) {
    printf_d("Bucket [%d]: ", b);
    for (struct MallocHead * p = buck_freelist[b]; p; p = p->next) {
      PutHex('=', p);
    }
    puts("\r");
  }
  puts("\r");
}//
#endif

void *Malloc(word n) {
  if (!heap_max) {
  //printf("m:first:%d\n", __LINE__);
#ifdef unix
      // byte MemoryPoolForUnix[102400];
       heap_min = (word)MemoryPoolForUnix;
       heap_here = (word)MemoryPoolForUnix;
       heap_max = (word)MemoryPoolForUnix + sizeof MemoryPoolForUnix;
#else
       word new_memory_size = 0;
       word end_of_new_mem = 0;
       errnum err = Os9Mem(new_memory_size, &new_memory_size, &end_of_new_mem);
       if (err) MallocOOM(err, n, 0);
       //printf(" *** 1st Os9Mem e=%x => %x %x\n", err, new_memory_size, end_of_new_mem);
       heap_min = end_of_new_mem;
       heap_here = end_of_new_mem;

       // round up to multiple of 0x2000, and get approval for that.
       // HARDWIRED CONSTANTS FOR COCO3/MOOH 8K MEMORY PAGES.
       new_memory_size = (end_of_new_mem + 0x2000) & 0xE000;
       err = Os9Mem(new_memory_size, &new_memory_size, &end_of_new_mem);
       //printf(" *** 2nd Os9Mem e=%x => %x %x\n", err, new_memory_size, end_of_new_mem);
       if (err) MallocOOM(err, n, 0);
       heap_max = end_of_new_mem;
#endif
       Assert(heap_max >= heap_min);
  }
  //printf("m:%d:%x,%x,%x\n", __LINE__, heap_min, heap_here, heap_max);
  int cap;
  byte b = which_bucket(n, &cap);
  //printf("m:%d:n=%d,b=%d\n", __LINE__, n, b);
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
    PC_Trace('M', (char *) h);
#endif
    return (char *) (h + 1);
  }

  // Break fresh memory.
  char *p = (char *) heap_here;
  word new_brk = heap_here + (word) (cap + sizeof(struct MallocHead));
  if (new_brk > heap_max) {
    if (heap_retry) {
        MallocOOM(255, n, cap); // Double fault.
    }

#ifdef unix
    Assert(!"TODO: Use sbrk on unix and OS9");
#else
    // Add another 0x2000 page.
    word new_memory_size = heap_max + 0x2000;
    word end_of_new_mem = 0;
    errnum err = Os9Mem(new_memory_size, &new_memory_size, &end_of_new_mem);
    //printf(" *** Os9Mem e=%x => %x %x\n", err, new_memory_size, end_of_new_mem);
    if (err) MallocOOM(err, n, cap);
    heap_max = new_memory_size;
#endif

    heap_retry = true;
    p = (char*)Malloc(n);
    heap_retry = false;
    return p;
  }

  heap_here = new_brk;
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
  PC_Trace('M', (char *) h);
#endif
  return (char *) (h + 1);
}

void Free(void *p) {
  if (!p)
    return;

  struct MallocHead *h = ((struct MallocHead *) p) - 1;
  if (!h->cap) {                // TODO -- because double-frees.
    Panic("DoubleFree");
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
  PC_Trace('F', (char *) h);
#endif
}

void *ReAlloc(void *p, word n) {
  if (!p)
    return Malloc(n);
  struct MallocHead *h = ((struct MallocHead *) p) - 1;
  if (n <= h->cap) {
    return (char *) p;
  }

  char *z = (char*)Malloc(n);
  memcpy(z, p, h->cap);
  Free(p);
  return z;
}
