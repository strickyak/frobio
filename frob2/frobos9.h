// OS9 System Calls, lightly wrapped for calling from C.
// This module contains no global data, including no
// const char* strings.

// TODO: use typedef `byte` and `word` instead of `int`,
// to mark naturally 1-byte and 2-byte values, and generate
// faster code.  

// TODO: eliminate "asm" from prototypes, using named
// local variables in the assembly (I didn't know about that
// when I first wrote these).

#ifndef _FROBOS9_H_
#define _FROBOS9_H_

#include "frob2/froblib.h"

#ifdef unix

// Can emulate OS9 calls?
#define asm /**/

#endif

#if 0
asm void DisableIrqs();
asm void EnableIrqs();
#endif

// These are *counting*, so an equal number of EnableIrqs()
// must be called before Irqs are actually enabled.
void DisableIrqsCounting(); // untested
void EnableIrqsCounting(); // untested

asm void Os9Exit(byte status);

asm errnum Os9Create(const char* path, int mode, int attrs, int* fd);

asm errnum Os9Open(const char* path, int mode, int* fd);

asm errnum Os9Delete(const char* path);

asm errnum Os9ChgDir(const char* path, int mode);

asm errnum Os9MakDir(const char* path, int mode);

asm errnum Os9GetStt(int path, int func, int* dOut, int* xOut, int* uOut);

asm byte Os9SetStt(int path, int func, int* dOut, int* x_inout, int* u_inout);

asm errnum Os9Read(int path, char* buf, int buflen, int* bytes_read);

asm errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read);

asm errnum Os9Write(int path, const char* buf, int max, int* bytes_written);

asm errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written);

asm errnum Os9Dup(int path, int* new_path);

asm errnum Os9Close(int path);

asm errnum Os9Sleep(int secs);

asm errnum Os9Wait(int* child_id_and_exit_status);

asm errnum Os9Fork(const char* program, const char* params, int paramlen, int lang_type, int mem_size, int* child_id);

asm errnum Os9Chain(const char* program, const char* params, int paramlen, int lang_type, int mem_size);

asm errnum Os9Send(int process_id, int signal_code);
errnum Os9STime(byte* time_packet);

errnum Os9Mem(word* new_memory_size_inout, word* end_of_new_mem_out);
errnum Os9AllRam(byte num_blocks, word* start_block_out);
errnum Os9DelRam(byte num_blocks, word start_block);
errnum Os9MapBlk(word starting_block, byte num_blocks, word* first_address_out);
errnum Os9ClrBlk(byte num_blocks, word first_address);

// Was used in NCL code.  Calls Os9Write with strlen(str) to stderr.
asm void Os9_print(const char* str);

#endif // _FROBOS9_H_
