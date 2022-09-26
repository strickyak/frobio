#include "frobio/shmem.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

word shmem_base;  // For Ptr and POINTER macros.
byte shmem_os9_blk;  // For Ptr and POINTER macros.

word Pointer(word link) {
    if (!link) return 0;
    assert((~SHMEM_MASK & link) == 0);
    return shmem_base + (SHMEM_MASK & link);
}

#define LINK(P) (printf("LINK[%d](%x)", __LINE__,(P)), Link(P))

word Link(word ptr) {
    if (!ptr) return 0;
    word down = (~SHMEM_MASK & ptr);
    printf(" Link(ptr:%x base:%x and:%x down:%x ret:%x) ", ptr, shmem_base, (~SHMEM_MASK & ptr), down, SHMEM_MASK & ptr );
    assert(down == shmem_base);
    assert((~SHMEM_MASK & ptr) == shmem_base);
    return SHMEM_MASK & ptr;
}

static error Find(const char* name, word* blk_out) {
    // TODO: Use F$GBlkMp to be more correct.
    byte blk;
    error err;
    shmem_base = 0;

    for (blk = SHMEM_BLOCKS_BEGIN; blk < SHMEM_BLOCKS_END; blk++) {
        err = Os9MapBlk(/*starting_block*/blk, ONE_BLOCK, &shmem_base);
        if (err) NyFatalD("cannot MapBlk: err=%d.", err);

        struct shmem* mem = (struct shmem*)shmem_base;
        if (strncmp((const char*)mem->magic0, SHMEM_MAGIC_0, 4)) goto Different;
        if (strncmp((const char*)mem->magic4, SHMEM_MAGIC_4, 4)) goto Different;
        if (strncmp((const char*)mem->name, name, 4)) goto Different;
        if (mem->protocol_level != SHMEM_PROTOCOL_LEVEL) goto Different;
        goto Found;

Different:
        err = Os9ClrBlk(ONE_BLOCK, shmem_base);
        shmem_base = NULL;
        if (err) NyFatalD("cannot ClrBlk: err=%d.", err);
    }
    *blk_out = 0;
    return 1;  // Not Found.

Found:
    *blk_out = blk;
    return OKAY;  // Found.
}

error ShmemCreate(const char* name) {
    printf(" [%d::%04x] ", __LINE__, shmem_base);
    assert(!shmem_base);
    word blk = 0;
    error err = Find(name, &blk);
    if (err) {
      err = Os9AllRam(ONE_BLOCK, &blk);
      if (err) NyFatalD("cannot Os9AllRam: err=%d.", err);

      err = Os9MapBlk(blk, ONE_BLOCK, (word*)&shmem_base);
      if (err) NyFatalD("cannot Os9MapBlk: err=%d.", err);
    }
    assert(shmem_base);

    memset16((void*)shmem_base, 0, ONE_BLOCK*(SHMEM_BLOCK_SIZE>>1));

    struct shmem* base = (struct shmem*) shmem_base;
    memcpy(base->magic0, SHMEM_MAGIC_0, 4);
    memcpy(base->magic4, SHMEM_MAGIC_4, 4);
    strncpy((char*)base->name, name, 4);
    base->protocol_level = SHMEM_PROTOCOL_LEVEL;
    assert(blk < 256);
    base->os9_blk = (byte)blk;
    struct arena* arena = &base->arena;
    arena->begin = LINK((word)(base+1)); // after the shmem.
    arena->next = arena->begin;
    arena->end = arena->begin + ONE_BLOCK*SHMEM_BLOCK_SIZE;
    return OKAY;
}
error ShmemAttach(const char* name) {
    assert(!shmem_base);
    word blk = 0;
    error e = Find(name, &blk);
    if (e==OKAY) {
        assert(shmem_base);
    } else {
        assert(!shmem_base);
    }
    return e;
}
error ShmemDetach() {
    assert(shmem_base);
    error e = Os9ClrBlk(ONE_BLOCK, shmem_base);
    shmem_base = 0;
    return OKAY;
}

word* ShmemRootAddr(char which) {
    assert(shmem_base);
    assert('A' <= which && which <= 'Z');
    struct shmem* base = (struct shmem*) shmem_base;
    return base->roots + (which - 'A');
}

struct bucket* FindBucketForSize(word size) {
    struct shmem* base = (struct shmem*) shmem_base;
    struct arena* arena = &base->arena;
    struct bucket* buck = arena->bucket;
    word buck_end = (word)buck + sizeof arena->bucket;
    for( ; (word)buck < buck_end; buck++) {
        if (!buck->size) {
            // start new bucket.
            buck->size = size;
            return buck;
        }
        if (buck->size == size) {
            return buck;
        }
    }
    return NULL;
}

#define D printf(" -%d- ", __LINE__);
error ShmemMalloc(word size, word* ptr_out) {
    assert(ptr_out);
    assert(size > 1);
    assert(size < SHMEM_BLOCK_SIZE);
    struct shmem* base = (struct shmem*) shmem_base;
    assert(base);
D
    struct bucket* buck = FindBucketForSize(size);
    if (!buck) {
D       *ptr_out = 0;
        return ERR(2, "SHMEM_OUT_OF_BUCKETS");
    }
D   struct allocation* a = (struct allocation*) Pointer(buck->free_list);
    if (a) {
        // next link is first in payload.
D       buck->free_list = *(word*)(a+1);
    } else {
        printf(" %x %x \n", base, base->arena.next);
D       a = (struct allocation*) Pointer(base->arena.next);
D       word next_addr = (word)(a+1) + size;
D       word next_link = LINK(next_addr);
D       if ((word)next_link >= base->arena.end) {
            *ptr_out = 0;
            return ERR(3, "SHMEM_OUT_OF_ARENA");
        }
        base->arena.next = next_link;
    }
    a->guard1 = SHMEM_ALLOC_GUARD_1;
    a->bucket_link = LINK((word)buck);
    a->guard2 = SHMEM_ALLOC_GUARD_2;
    word ptr = (word)(a+1);
    memset((void*)ptr, 0, size);
    *ptr_out = ptr;
D   return OKAY;
}

error ShmemFree(word ptr) {
    assert(ptr);

    // Back up to find the allocation pointer.
    struct allocation* a = ((struct allocation*)ptr) - 1;
    assert(a->guard1 == SHMEM_ALLOC_GUARD_1);
    assert(a->guard2 == SHMEM_ALLOC_GUARD_2);
    // Clear freed memory
    struct bucket* buck = (struct bucket*)Pointer(a->bucket_link);
    assert(buck);  // If malloc'ed, bucket must exist.
    assert(buck->size > 1);
    assert(buck->size < SHMEM_BLOCK_SIZE);
    memset((void*)ptr, 0, buck->size);

    // Hook freed memory onto front of bucket's free_list.
    *(word*)ptr = buck->free_list;
    buck->free_list = LINK((word)a);
    return OKAY;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage:  shmem NAME\n");
        exit(2);
    }
    const char* name = argv[1];

    printf("attach %s...", name);
    printf(" [%d::%04x] ", __LINE__, shmem_base);
    error e = ShmemAttach(name);
    printf(" [%d::%04x] ", __LINE__, shmem_base);
    printf("...[%d] ", e);
    if (e) {
        printf(" [%d::%04x] ", __LINE__, shmem_base);
        printf("create %s...", name);
        e = ShmemCreate(name);
        printf("...[%d] ", e);
        printf(" [%d::%04x] ", __LINE__, shmem_base);
    }
    assert(!e);
    printf(" base=%04x (blk=%d) ", shmem_base, shmem_os9_blk);
        
    printf(" malloc ");
    word p = NULL;
    e = ShmemMalloc(100, &p);
    printf("[%d]=%04x  free ", e, p);
    e = ShmemFree(p);
    printf("[%d]  okay\n", e);

    return 0;
}
