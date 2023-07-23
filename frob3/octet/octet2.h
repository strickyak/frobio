#ifndef OCTET_H_2
#define OCTET_H_2

#include "frob3/frobtype.h"

// Object classes 1 through 15 do not contain pointers.
// Object classes 16 through 255 do contain pointers.
#define O_LAST_NONPTR_CLASS 15

// Public API.
typedef void (*OMarkRoots_t)(void);
void oinit(word begin, word end, OMarkRoots_t);
void ozero(word p, word len);
void omemcpy(word dest, word src, word len);
word oalloc_try(word len, byte cls);
word oalloc(word len, byte cls);
void ofree(word addr);
void omark(word addr);
void ogc();

// Global Variables.
extern word ORamBegin;
extern word ORamUsed;
extern word ORamEnd;
extern OMarkRoots_t OMarkRoots;

#define ogetb(A)     (*(byte*)(A))
#define oputb(A, X)  (*(byte*)(A) = (byte)(X))
#define ogetw(A)     (*(word*)(A))
#define oputw(A, X)  (*(word*)(A) = (word)(X))

#define ocls(addr) ogetb((addr)-1)
#define ocap(addr) (0x7F & ogetb((addr)-2))

#endif  // OCTET_H_2
