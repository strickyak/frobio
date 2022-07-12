// ny: NitroYak: Yak Libs for NitrOS9

#ifndef _FROBIO_NYLIB_H_
#define _FROBIO_NYLIB_H_

#include "frobio/nytypes.h"

// Set len bytes to 0, starting at p.
void NyZero(void* p, word len);

// NyWhite returns true if input is white:
// Spaces and all control chars <32 are white.
bool NyWhite(char c);

// NySplit splits on NyWhite chars, which may be redundant.
// No empty words are returned.
// Return number of words out, which may be 0.
int NySplit(char* s, char**words_out, int max_words);

// String comparison.
bool NyStrEq(const char* a, const char* b);
bool NyCharEqIgnoreCase(char x, char y);
bool NyStrEqIgnoreCase(const char* a, const char* b);

// Print a fatal message with one `%d` in it, and exit 255.
// TODO: print on stderr instead of stdout.
void NyFatalD(const char* fmt, int d);
void NyFatalS(const char* fmt, const char* s);

// TODO: broken?
void NyFormatDottedDecimalQuad(char* buffer, quad addr);

byte NyDeHex(byte ch);
void NySkipSpaces(const char** pp);
byte NyParseChar(const char** pp);
byte NyParseHexByte(const char** pp);
word NyParseHexWord(const char** pp);
word NyParseDecimalWord(const char** pp);
byte NyCheckByte(word a);
quad NyFormQuadFromBytes(byte a, byte b, byte c, byte d);
quad NyParseDottedDecimalQuad(const char** pp);
quad NyParseDottedDecimalQuadAndPort(const char** pp, word* port);

#endif // _FROBIO_NYLIB_H_
