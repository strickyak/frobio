// OS9 System Calls, lightly wrapped for calling from C.
// This module contains no global data, including no
// const char* strings.

// TODO: use typedef `byte` and `word` instead of `int`,
// to mark naturally 1-byte and 2-byte values, and generate
// faster code.  

// TODO: eliminate "asm" from prototypes, using named
// local variables in the assembly (I didn't know about that
// when I first wrote these).

#ifndef _OS9CALL_H_
#define _OS9CALL_H_

#include "frobio/nytypes.h"

asm void DisableIrqs();
asm void EnableIrqs();

// These are *counting*, so an equal number of EnableIrqs()
// must be called before Irqs are actually enabled.
asm void DisableIrqsCounting(); // untested
asm void EnableIrqsCounting(); // untested

asm void Os9Exit(byte status);

asm error Os9Create(const char* path, int mode, int attrs, int* fd);

asm error Os9Open(const char* path, int mode, int* fd);

asm error Os9Delete(const char* path);

asm error Os9ChgDir(const char* path, int mode);

asm error Os9MakDir(const char* path, int mode);

asm error Os9GetStt(int path, int func, int* dOut, int* xOut, int* uOut);

asm byte Os9SetStt(int path, int func, int* dOut, int* x_inout, int* u_inout);

asm error Os9Read(int path, char* buf, int buflen, int* bytes_read);

asm error Os9ReadLn(int path, char* buf, int buflen, int* bytes_read);

asm error Os9Write(int path, const char* buf, int max, int* bytes_written);

asm error Os9WritLn(int path, const char* buf, int max, int* bytes_written);

asm error Os9Dup(int path, int* new_path);

asm error Os9Close(int path);

asm error Os9Sleep(int secs);

asm error Os9Wait(int* child_id_and_exit_status);

asm error Os9Fork(const char* program, const char* params, int paramlen, int lang_type, int mem_size, int* child_id);

asm error Os9Chain(const char* program, const char* params, int paramlen, int lang_type, int mem_size);

asm error Os9Send(int process_id, int signal_code);

error Os9AllRam(byte num_blocks, word* start_block_out);
error Os9DelRam(byte num_blocks, word start_block);
error Os9MapBlk(word starting_block, byte num_blocks, word* first_address_out);
error Os9ClrBlk(byte num_blocks, word first_address);

// Was used in NCL code.  Calls Os9Write with strlen(str).
asm error Os9_puts(const char* str);

#endif // _OS9CALL_H_
