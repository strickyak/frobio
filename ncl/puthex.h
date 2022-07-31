#ifndef _FROBIO_NCL_PUTHEX_H
#define _FROBIO_NCL_PUTHEX_H

#include <frobio/nytypes.h>

// convert a nybble to a hex char (uses UPPER).
char hexchar(byte i);

// panic: print message and exit 5.
void panic(const char *s);

#ifndef unix

// puthex prints a prefix and a hex number, like `(p=1234)`,
// only using a small buffer.  Quick and reliable for debugging.
void puthex(char prefix, int a);

// print call stack, for debugging.
asm void pc_trace(int mark, char* ptr);

#endif // !unix

#endif // _FROBIO_NCL_PUTHEX_H
