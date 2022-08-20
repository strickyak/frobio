// shmem.h : Shard Memory for Frobio

#include "frobio/nytypes.h"

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

    byte os9_blk; // OS9 block number
    byte protocol_level;

    word roots[26];  // [0] to [25] are named 'A' to 'Z'.
    struct arena arena;
};
// Arbitrary magic strings to mark the shard page(s).
#define SHMEM_MAGIC_0 "fSHM"
#define SHMEM_MAGIC_4 "\xc0\xb6\x85\xf5"

// Hardwired for one 8K block.
#define ONE_BLOCK 1 // on Coco3, one 8K block.
#define SHMEM_PROTOCOL_LEVEL 1
#define SHMEM_BLOCK_SIZE 8192 // on Coco3 MMU
#define SHMEM_MASK (SHMEM_BLOCK_SIZE-1)
#define SHMEM_BLOCKS_BEGIN 1  //
#define SHMEM_BLOCKS_END 63   // coco: 63 is I/O.

extern word shmem_base;     // For Ptr and SetLink macros.
extern byte shmem_os9_blk;  // For Ptr and SetLink macros.

error ShmemCreate(const char* name);
error ShmemAttach(const char* name);
error ShmemDetach();

word* ShmemRootAddr(char which);

error ShmemMalloc(word size, word* ptr_out);
error ShmemFree(word ptr);

word Pointer(word link);
word Link(word ptr);

#define ERR(N,S) (printf("[E%d:%s:%s:%d]\n", (N), (S), __FILE__, __LINE__), (error)(N))

