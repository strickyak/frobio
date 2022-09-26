// shmem.h : Shard Memory for Frobio

#ifndef _FROBIO_SHMEM2_H_
#define _FROBIO_SHMEM2_H_

#include "frobio/nytypes.h"

typedef word link;
void* RGet(link* lp);
void RPut(link* lp, void* v);

struct bucket {
    word size;
    word free_list;
};

struct arena {
    word begin;
    word next;
    word end;
    struct bucket bucket[20];  // Only use a few sizes.
};

struct allocation {
    byte guard1;
    word bucket_link;
    byte guard2;
};

// Allocation Guard Words
#define SHMEM_ALLOC_GUARD_1 0xA1
#define SHMEM_ALLOC_GUARD_2 0xB2

struct shmem {
    byte magic0[4];
    byte magic4[4];
    byte name[4];
    word roots[26];  // [0] to [25] are named 'A' to 'Z'.
    byte num_blocks;
    byte pad1;
    struct arena arena;
};
// Arbitrary magic strings to mark the shard page(s).
#define SHMEM_MAGIC_0 "fSHM"
#define SHMEM_MAGIC_4 "\xc0\xb6\x85\xf5"

// Hardwired for one 8K block.
#define SHMEM_BLOCK_SIZE 8192 // on Coco3 MMU
#define SHMEM_HALF_BLOCK_SIZE 4096 // on Coco3 MMU
#define SHMEM_MASK (SHMEM_BLOCK_SIZE-1)
#define SHMEM_SHIFT 13  // on Coco
#define SHMEM_BLOCKS_BEGIN 1  //
#define SHMEM_BLOCKS_END 63   // coco: 63 is I/O.

error ShmemCreate(const char* name, word size, struct shmem **base_out);
error ShmemAttach(const char* name, struct shmem **base_out);
error ShmemDetach(struct shmem* base);

word* ShmemRootAddr(struct shmem* base, char which);

error ShmemMalloc(struct shmem* base, word size, void* *ptr_out);
error ShmemFree(struct shmem* base, void* ptr);

#define ERR(N,S) (printf("[E%d:%s:%s:%d]\n", (N), (S), __FILE__, __LINE__), (error)(N))

#endif // _FROBIO_SHMEM2_H_
