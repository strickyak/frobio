// ny: NitroYak: Yak Libs for NitrOS9

#include <cmoc.h>
#include "frobio/nylib.h"

void NyZero(void* p, word len) {
    byte* bp = (byte*)p;
    for (word i = 0; i < len; i++) {
        bp[0] = 0;
    }
}

bool NyWhite(char c) {
  return (byte)c <= ' ';
}

int NySplit(char* s, char* *words_out, int max_words) {
  int count = 0;
  while (count < max_words) {
    while (*s && NyWhite(*s)) s++; // skip white space.
    if (!*s) break;
    *words_out++ = s;
    ++count;
    while (*s && !NyWhite(*s)) s++; // skip non-white.
    if (!*s) break;
    *s++ = '\0';
  }
  return count;
}

bool NyStrEq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) return 0;
    a++;
    b++;
  }
  return (*a == *b);
}

bool NyCharEqIgnoreCase(char x, char y) {
  if ('a' <= x && x <= 'z') x -= 32;
  if ('a' <= y && y <= 'z') y -= 32;
  return x == y;
}

bool NyStrEqIgnoreCase(const char* a, const char* b) {
  while (*a && *b) {
    if (!NyCharEqIgnoreCase(*a, *b)) return 0;
    a++;
    b++;
  }
  return NyCharEqIgnoreCase(*a, *b);
}

// TODO: use fprintf(stderr, ...) for Fatal.
// TODO: use varargs and "v" forms correctly.
typedef void (*avoid_warning_for_printf_with_int)(const char* fmt, int d);
void NyFatalD(const char* fmt, int d) {
  printf("\n*** FATAL: ");
  ((avoid_warning_for_printf_with_int)printf)(fmt, d);
  exit(255);
}
typedef void (*avoid_warning_for_printf_with_str)(const char* fmt, const char* s);
void NyFatalS(const char* fmt, const char* s) {
  printf("\n*** FATAL: ");
  ((avoid_warning_for_printf_with_str)printf)(fmt, s);
  exit(255);
}


////////////// Parsing

byte NyDeHex(byte ch) {
  if ('0' <= ch && ch <= '9') {
    return ch - '0';
  }
  if ('A' <= ch && ch <= 'Z') {
    return ch - 'A' + 10;
  }
  if ('a' <= ch && ch <= 'z') {
    return ch - 'a' + 10;
  }
  // assert(0);  // TODO: panic/recover
  NyFatalD("bad hex char: %d(decimal)", ch);
}

// TODO: sprintf is broken?
void NyFormatDottedDecimalQuad(char* buffer, quad addr) {
    byte* p = (byte*)&addr;
    sprintf(buffer, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

// Hereafter, pp will point to a pointer to const char,
// and will be an argument to all functions.
// In the body, P will act like a normal pointer, using (*pp).
#define P (*pp)

void NySkipSpaces(const char** pp) {
  while (*P == ' ') {
    P++;
  }
}
byte NyParseChar(const char** pp) { // must be the current char
  return NyDeHex(*P++);
}
byte NyParseHexByte(const char** pp) { // must be 2 chars
  byte a = NyParseChar(pp);
  byte b = NyParseChar(pp);
  return (a << 4) | b;
}
word NyParseHexWord(const char** pp) { // must be 4 chars
  byte a = NyParseHexByte(pp);
  byte b = NyParseHexByte(pp);
  return ((word)a << 8) | b;
}

word NyParseDecimalWord(const char** pp) {
  word z = 0;
  while (*P) {
    if ('0' <= *P && *P <= '9') {
      z = 10*z + (*P - '0');
    } else {
      break;
    }
    P++;
  }
  return z;
}

byte NyCheckByte(word a) {
  if (a > 255) {
    NyFatalD("expected a byte 0 to 255: %d", a);
  }
  return (byte)a;
}

quad NyFormQuadFromBytes(byte a, byte b, byte c, byte d) {
  quad z;
  byte* p = (byte*)&z;
  p[0]=a; p[1]=b; p[2]=c; p[3]=d;
  return z;
}

quad NyParseDottedDecimalQuad(const char** pp) {
  word a = NyParseDecimalWord(pp);
  if (*P != '.') goto no_dot;
  P++;
  word b = NyParseDecimalWord(pp);
  if (*P != '.') goto no_dot;
  P++;
  word c = NyParseDecimalWord(pp);
  if (*P != '.') goto no_dot;
  P++;
  word d = NyParseDecimalWord(pp);

  return NyFormQuadFromBytes(
      NyCheckByte(a), NyCheckByte(b), NyCheckByte(c), NyCheckByte(d));

no_dot:
  NyFatalD("expected dot in dotted quad", (word)P);
  return 0;
}

// On call: default port is in *port.
// On return: *port may be replaced with specified port.
quad NyParseDottedDecimalQuadAndPort(const char** pp, word* port) {
  quad addy = NyParseDottedDecimalQuad(pp);
  if (*P == ':') {
    P++;
    *port = NyParseDecimalWord(pp);
  }
  return addy;
}
