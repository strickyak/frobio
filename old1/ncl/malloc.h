#ifndef _FROBIO_NCL_MALLOC_H_
#define _FROBIO_NCL_MALLOC_H_

#include "frobio/nytypes.h"

#ifndef unix // Use ordinary malloc for unix.

// Main public functions.
char *malloc(int n);
void free(void *p);
char *realloc(void *p, int n);

// Buckets for malloc/free quanta.
#define STACK_MARGIN 300        // paranoid gap between heap and stack.
#define SMALLEST_BUCKET 8       // smallest mallocation; power of 2.
#define NBUCKETS 12             // 8B to 16KB.

struct MallocHead {
  char barrierA;
  struct MallocHead *next;
  int cap;
  char barrierZ;
};

// Heap boundaries.
extern word heap_min;          // Set by start code to bss_end.
extern word heap_here;          // Set by start code to bss_end.
extern word heap_max;          // Set by every stkcheck().

extern struct MallocHead *buck_freelist[NBUCKETS];
extern int buck_num_alloc[NBUCKETS];
extern int buck_num_free[NBUCKETS];
extern int buck_num_brk[NBUCKETS];

#endif // !unix
#endif // _FROBIO_NCL_MALLOC_H_
