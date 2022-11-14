// ny: NitroYak: Yak Libs for NitrOS9

#include "frob2/froblib.h"

// Hereafter, pp will point to a pointer to const char,
// and will be an argument to all functions.
// In the body, P will act like a normal pointer, using (*pp).
#define P (*pp)

//chop
byte Verbosity = LLInfo;
//chop
Buf ErrBuf;
byte ErrNo;
const char* ErrFile;
word ErrLine;
//chop

void SetFailure(const char* file, word line, byte e, const char* fmt, ...) {
    if (!ErrBuf.s) {
        // First time, allocate.
        ErrBuf.s = (char*)Malloc(128);
        Assert(ErrBuf.s);
        ErrBuf.s[0] = 0;  // Clear any failure.
    }
    if (!fmt) {
        ErrBuf.s[0] = 0;  // Clear any failure.
        ErrBuf.n = 0;  // Clear any failure.
        return;
    }

    if (ErrBuf.n) {
      FPuts("\n[prev error] ", StdErr);
      WritLnAll(2, ErrBuf.s, ErrBuf.n);
      FPuts("\n", StdErr);
    }
    ErrBuf.s[0] = 0;
    ErrBuf.n = 0;

    ErrFile = file;
    ErrLine = line;
    ErrNo = e;

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&ErrBuf, fmt, ap);
    va_end(ap);

    BufFinish(&ErrBuf);
}

Buf LogBuf;
void Logger(const char* file, word line, byte level, const char* fmt, ...) {
    extern byte disable_irq_count;

    // fmt may be NULL just to flush when irqs are re-enabled.
    if (fmt) {
        if (!LogBuf.s) {
            // First time, allocate.
            LogBuf.s = (char*)Malloc(128);
        }
        if (level > Verbosity) return;
        if (level > MAX_VERBOSE) return;

        if (disable_irq_count) {
            if (Verbosity > LLError) {
                BufAppC(&LogBuf, '~');  // non-error just logs '~' during disabled irqs.
                return;
            }
        }

        BufFormat(&LogBuf, "%d:%s:%d ", level, file, line);
        if (level == LLFatal) {
            BufAppStr(&LogBuf, "FATAL: ");
        }

        va_list ap;
        va_start(ap, fmt);
        BufFormatVA(&LogBuf, fmt, ap);
        va_end(ap);
    }

    // Never flush to stderr while irqs are disabled.
    if (disable_irq_count) return;
   
    // Skip writing if empty. 
    if (!LogBuf.n) return;

    // Append \n if it is not already there.
    if (LogBuf.s[LogBuf.n-1] != '\n') {
        BufAppC(&LogBuf, '\n');
    }

    // Finish, Print, and Reset the buffer.
    char* s = BufFinish(&LogBuf);
    FPuts(s, StdErr);
    if (level == LLFatal) {
        exit(127);
    }
    LogBuf.n = 0;
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
  Assert(0);
}

void NyFormatDottedDecimalQuad(char* buffer, quad addr) {
    byte* p = (byte*)&addr;
    SPrintf(buffer, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

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
    LogFatal("Byte should be less than 256: %d", a);
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
  const char* orig = P;
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
  LogFatal("missing dot in dotted quad: %s", orig);
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


size_t StrLen(const byte* s) {
    return strlen((const char*)s);
}
void StrCat(byte*a, const byte*b) {
    strcat((char*)a, (const char*)b);
}


bool StrEq(const byte*a, const byte*b) {
    return 0 == strcmp((const char*)a, (const char*)b);
}
bool StrNEq(const byte*a, const byte*b, word n) {
    return 0 != strcmp((const char*)a, (const char*)b);
}
bool StrCaseEq(const byte*a, const byte*b) {
    return 0 == strcasecmp((const char*)a, (const char*)b);
}
#if 0
bool StrNCaseEq(const byte*a, const byte*b, word n) {
    return 0 != strncasecmp((const char*)a, (const char*)b, n);
} //
#endif
