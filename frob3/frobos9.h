// OS9 System Calls, lightly wrapped for calling from C.
// This module contains no global data, including no
// const char* strings.

// TODO: use typedef `byte` and `word` instead of `int`,
// to mark naturally 1-byte and 2-byte values, and generate
// faster code.  

// TODO: eliminate "asm" from prototypes, using named
// local variables in the assembly (I didn't know about that
// when I first wrote these).

#ifndef _FROB3_FROBOS9_H_
#define _FROB3_FROBOS9_H_

#include "frob3/froblib.h"

#if defined(unix) || defined(FOR_DECB)
// Can emulate OS9 calls?
#define IF_os9_THEN_asm /**/
#else
#define IF_os9_THEN_asm asm
#endif

// These are *counting*, so an equal number of EnableIrqs()
// must be called before Irqs are actually enabled.
void DisableIrqsCounting();
void EnableIrqsCounting();

/* alias CC='/opt/yak/fuzix/bin/m6809-unknown-gcc-4.6.4 -std=c99' */
#ifdef __SIZE_MAX__ // detect gcc
#if __SIZE_MAX__ == 65535U // detect gcc
#undef IF_os9_THEN_asm
#define IF_os9_THEN_asm extern
#endif
#endif


#if 1
#include "_generated_os9api_for_cmoc.h"
#else

IF_os9_THEN_asm void Os9Exit(byte status);

IF_os9_THEN_asm errnum Os9Create(int mode, int attrs, const char* path, int* fd);

IF_os9_THEN_asm errnum Os9Open(int mode, const char* path, int* fd);

IF_os9_THEN_asm errnum Os9Delete(const char* path);

IF_os9_THEN_asm errnum Os9ChgDir(const char* path, int mode);

IF_os9_THEN_asm errnum Os9MakDir(const char* path, int mode);

IF_os9_THEN_asm errnum Os9GetStt(int path, int func, int* dOut, int* xOut, int* uOut);

IF_os9_THEN_asm byte Os9SetStt(int path, int func, int* dOut, int* x_inout, int* u_inout);

IF_os9_THEN_asm errnum Os9Read(int path, char* buf, int buflen, int* bytes_read);

IF_os9_THEN_asm errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read);

IF_os9_THEN_asm errnum Os9Write(int path, const char* buf, int max, int* bytes_written);

IF_os9_THEN_asm errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written);

IF_os9_THEN_asm errnum Os9Dup(int path, int* new_path);

IF_os9_THEN_asm errnum Os9Close(int path);

IF_os9_THEN_asm errnum Os9Sleep(int secs);

IF_os9_THEN_asm errnum Os9Wait(int* child_id_and_exit_status);
errnum Os9ID(byte* proc_id_out, word* user_id_out);

IF_os9_THEN_asm errnum Os9Fork(const char* program, const char* params, int paramlen, int lang_type, int mem_size, int* child_id);

IF_os9_THEN_asm errnum Os9Chain(const char* program, const char* params, int paramlen, int lang_type, int mem_size);


IF_os9_THEN_asm errnum Os9Send(int process_id, int signal_code);
errnum Os9STime(byte* time_packet);

errnum Os9Mem(word* new_memory_size_inout, word* end_of_new_mem_out);
errnum Os9AllRam(byte num_blocks, word* start_block_out);
errnum Os9DelRam(byte num_blocks, word start_block);
errnum Os9MapBlk(word starting_block, byte num_blocks, word* first_address_out);
errnum Os9ClrBlk(byte num_blocks, word first_address);

#endif

#if ONLY_IN_KERNEL_MODE
// 64-byte block routines.
errnum Os9All64(word base, word* base_out, word* block_addr, byte* block_num);
errnum Os9Find64(byte block_num, word base, word* block_addr);
errnum Os9Ret64(byte block_num, word base);
#endif

// For the Gomar hyper ops.
void GomarHyperExit(errnum status);  // exits os9 process, if no hyper.

// Was used in NCL code.  Calls Os9Write with strlen(str) to stderr.
IF_os9_THEN_asm void Os9_print(const char* str);

#ifdef __SIZE_MAX__ // detect gcc
#if __SIZE_MAX__ == 65535U // detect gcc
#define IF_os9_THEN_asm extern
#endif
#endif

#endif // _FROB3_FROBOS9_H_
