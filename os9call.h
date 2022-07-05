// OS9 System Calls, lightly wrapped for calling from C.
// This module contains no global data, including no
// const char* strings.

// TODO: use typedef `byte` and `word` instead of `int`,
// to mark naturally 1-byte and 2-byte values, and generate
// faster code.  Also, eliminate "asm" from prototypes,
// and use named variables (I have done this somewhere else).

#ifndef _OS9CALL_H_
#define _OS9CALL_H_

#include "frobio/nytypes.h"

asm void Os9Exit(byte status);

asm byte Os9Create(const char* path, int mode, int attrs, int* fd);

asm byte Os9Open(const char* path, int mode, int* fd);

asm byte Os9Delete(char* path);

asm byte Os9ChgDir(char* path, int mode);

asm byte Os9MakDir(char* path, int mode);

asm byte Os9GetStt(int path, int func, int* dOut, int* xOut, int* uOut);

asm byte Os9Read(int path, char* buf, int buflen, int* bytes_read);

asm byte Os9ReadLn(int path, char* buf, int buflen, int* bytes_read);

asm byte Os9Write(int path, const char* buf, int max, int* bytes_written);

asm byte Os9WritLn(int path, const char* buf, int max, int* bytes_written);

asm byte Os9Dup(int path, int* new_path);

asm byte Os9Close(int path);

asm byte Os9Sleep(int secs);

asm byte Os9Wait(int* child_id_and_exit_status);

asm byte Os9Fork(const char* program, const char* params, int paramlen, int lang_type, int mem_size, int* child_id);

asm byte Os9Chain(const char* program, const char* params, int paramlen, int lang_type, int mem_size);

asm byte Os9Send(int process_id, int signal_code);

#endif // _OS9CALL_H_
