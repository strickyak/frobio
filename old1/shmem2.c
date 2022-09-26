#include "frobio/shmem2.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

void* RGet(link* lp) {
    if (*lp == 1) return NULL;
    return (void*)( (word)*lp + (word)lp );
}
void RPut(link* lp, void* v) {
    if (!v) {
        *lp = 1;
    } else {
        *lp = (word)v - (word)lp;
    }
}

static error Find(const char* name, byte num_blocks,
                  struct shmem* *base_out) {
    for (byte blk = SHMEM_BLOCKS_BEGIN; blk < SHMEM_BLOCKS_END; blk++) {
        error err = Os9MapBlk(/*starting_block*/blk, num_blocks, (word*)base_out);
        if (err) NyFatalD("cannot MapBlk: err=%d.", err);

        struct shmem* mem = *base_out;

        if (
            !strncmp((const char*)mem->magic0, SHMEM_MAGIC_0, 4) &&
            !strncmp((const char*)mem->magic4, SHMEM_MAGIC_4, 4) &&
            !strncmp((const char*)mem->name, name, 4)
        ) {
            return OKAY;  // Found.
        }

        err = Os9ClrBlk(num_blocks, (word)*base_out);
        if (err) NyFatalD("cannot ClrBlk: err=%d.", err);
    }
    return ERR(11, "SHMEM_NOT_FOUND");
}

error ShmemCreate(const char* name, word size,
                  struct shmem* *base_out) {
    byte num_blocks = (byte)(
        (size + SHMEM_MASK) >> SHMEM_SHIFT
    );
    error err = Find(name, num_blocks, base_out);
    if (err) {
      word start_block = 0;
      err = Os9AllRam(num_blocks, &start_block);
      if (err) NyFatalD("cannot Os9AllRam: err=%d.", err);

      err = Os9MapBlk(start_block, num_blocks, (word*)base_out);
      if (err) NyFatalD("cannot Os9MapBlk: err=%d.", err);
    }
    assert(*base_out);

    word addr = (word)*base_out;
    for (byte i = 0; i < num_blocks; i++) {
        memset16((void*)addr, 0, SHMEM_HALF_BLOCK_SIZE);
        addr += SHMEM_BLOCK_SIZE;
    }

    struct shmem* base = *base_out;
    memcpy(base->magic0, SHMEM_MAGIC_0, 4);
    memcpy(base->magic4, SHMEM_MAGIC_4, 4);
    strncpy((char*)base->name, name, 4);
    base->num_blocks = num_blocks;
    base->pad1 = 0;

    struct arena* arena = &base->arena;

    RPut(&arena->begin , (void*)(base+1));
    RPut(&arena->next , RGet(&arena->begin));
    RPut(&arena->end , ((char*)RGet(&arena->begin) + num_blocks*SHMEM_BLOCK_SIZE));

    return OKAY;
}

error ShmemAttach(const char* name,
                  struct shmem* *base_out) {
    error e = Find(name, 1, base_out);
    if (e) return e;

    byte num_blocks = (*base_out)->num_blocks;
    if (num_blocks > 1) {
        ShmemDetach(*base_out);
        e = Find(name, num_blocks, base_out);
        return e;
    }
    return OKAY;
}

error ShmemDetach(struct shmem* base) {
    assert(base);
    return Os9ClrBlk(base->num_blocks, (word)base);
}

word* ShmemRootAddr(struct shmem* base,char which) {
    assert(base);
    assert('A' <= which && which <= 'Z');
    return base->roots + (which - 'A');
}

struct bucket* FindBucketForSize(
        struct shmem* base,word size) {
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
error ShmemMalloc(struct shmem* base,word size, void* *ptr_out) {
    assert(ptr_out);
    assert(size > 1);
    assert(size < SHMEM_BLOCK_SIZE);
    assert(base);
D
    struct bucket* buck = FindBucketForSize(base, size);
    if (!buck) {
D       *ptr_out = NULL;
        return ERR(2, "SHMEM_OUT_OF_BUCKETS");
    }
D   struct allocation* a = (struct allocation*) RGet(&buck->free_list);
    if (a) {
        // next link is first in payload.
D       buck->free_list = *(word*)(a+1);
    } else {
        printf(" %x %x \n", base, base->arena.next);
D       a = (struct allocation*) RGet(&base->arena.next);
D       word next_addr = (word)(a+1) + size;
// D       word next_link = LINK(next_addr);
        link next_link;
D       RPut(&next_link , (void*)next_addr );
D       if ((word)next_link >= base->arena.end) {
            *ptr_out = NULL;
            return ERR(3, "SHMEM_OUT_OF_ARENA");
        }
        base->arena.next = next_link;
    }
    a->guard1 = SHMEM_ALLOC_GUARD_1;
    // a->bucket_link = LINK((word)buck);
    RPut(&a->bucket_link , buck);
    a->guard2 = SHMEM_ALLOC_GUARD_2;
    struct allocation *ptr = a+1;
    memset(ptr, 0, size);
    *ptr_out = ptr;
D   return OKAY;
}

error ShmemFree(struct shmem* base, void* ptr) {
    assert(ptr);

    // Back up to find the allocation pointer.
    struct allocation* a = ((struct allocation*)ptr) - 1;
    assert(a->guard1 == SHMEM_ALLOC_GUARD_1);
    assert(a->guard2 == SHMEM_ALLOC_GUARD_2);
    // Clear freed memory
    struct bucket* buck = (struct bucket*)RGet(&a->bucket_link);
    assert(buck);  // If malloc'ed, bucket must exist.
    assert(buck->size > 1);
    assert(buck->size < SHMEM_BLOCK_SIZE);
    memset((void*)ptr, 0, buck->size);

    // Hook freed memory onto front of bucket's free_list.
    *(word*)ptr = buck->free_list;
    RPut(&buck->free_list , a);
    return OKAY;
}

void Demo(const char* name) {
    struct shmem* shmem_base = 0;

    printf("attach %s...", name);
    printf(" [%d::%04x] ", __LINE__, shmem_base);
    error e = ShmemAttach(name, &shmem_base);
    printf(" [%d::%04x] ", __LINE__, shmem_base);
    printf("...[%d] ", e);
    if (e) {
        printf(" [%d::%04x] ", __LINE__, shmem_base);
        printf("create %s...", name);
        e = ShmemCreate(name, 1, &shmem_base);
        printf("...[%d] ", e);
        printf(" [%d::%04x] ", __LINE__, shmem_base);
    }
    assert(!e);
    printf(" base=%04x ", shmem_base);
        
    printf(" malloc ");
    void* p = NULL;
    e = ShmemMalloc(shmem_base, 100, &p);
    printf("[%d]=%04x  free ", e, p);
    e = ShmemFree(shmem_base, p);
    printf("[%d]  okay\n", e);
}

int main(int argc, char* argv[]) {
    argc--, argv++;
    if (argc <= 1) {
        printf("Usage:  shmem <cmd>\n");
        printf("Usage:  shmem demo <name>\n");
        exit(2);
    }
    const char* cmd = argv[0];
    printf("cmd=%s ", cmd);
    // if (!strcmp(cmd, "demo") && argc==2) Demo(argv[1]);
    if (cmd[0]=='D' && argc==2) Demo(argv[1]);
    else printf("Unknown command: %s\n", cmd);
    return 0;
}
