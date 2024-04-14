#ifndef _FROB2_FROBJOT_FROBJOT_H_
#define _FROB2_FROBJOT_FROBJOT_H_

// Jot: small (62 bytes max) ASCII strings.
//
// Jots require exactly 64 bytes, because they
// start with a length byte, and end with a '\0'.
//
// If the Jot overflows, its final character
// is changed to '#'.
//
// Jots only contain nice characters, which are
// CR, Space, and the 95 printable ASCII chars.
// If NL is appended, it turns into CR.
// If other characters are appended, they turn
// into \ooo octal escapes strings.

#include "frob2/frobtype.h"

#define JOT_MAX 62

typedef struct jot {
  byte len; 
  char s[JOT_MAX+1];
} Jot;

extern const byte HexAlphabet[];

bool JotNiceCharP(char c);

// To initialize a Jot: struct j62 = {0};


void JotAppC(Jot* j, char c);
void JotAppU(Jot* j, word x);
void JotAppX(Jot* j, word x);
void JotAppS(Jot* j, const char* x);
void JotAppJot(Jot* j, const Jot* x);
void JotFormatVA(Jot* j, const char* format, va_list ap);
void JotPrintf(Jot* j, const char* format, ...);

#endif // _FROB2_FROBJOT_FROBJOT_H_
