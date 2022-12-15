#include "frob2/froblib.h"
#include "frob2/frobos9.h"

#define debug_printf if(false)printf

void staticBufFillGap(Buf* buf, word width, word n, bool fill0);
byte* staticQFormatUnsignedLong(byte* p, unsigned long x);
byte* staticQFormatSignedLong(byte* p, signed long x);
byte* staticQFormatLongHex(byte* p, const byte* alphabet, unsigned long x);

//chop

byte ShortStaticBuffer[24];

//chop

// CharUp(c): convert to upper case for 26 ascii letters.
char CharUp(char c) {
  return ('a' <= c && c <= 'z') ? c - 32 : c;
}

// CharDown(c): convert to lower case for 26 ascii letters.
char CharDown(char c) {
  return ('A' <= c && c <= 'Z') ? c + 32 : c;
}

#if 0  // FOR CURLY
void BPutNumCurly(Buf* buf, byte c) {
          BPutChar(buf, '{');
          BPutU(buf, (word)c);
          BPutChar(buf, '}');
}//


void BEncodeCurly(Buf* buf, byte* str, int n) {
  BPutChar(buf, '"');
  for (int i = 0; i < n; i++) {
    byte c = str[i];
    if (32 <= c && c <= 127) {
      switch (c) {
        case '"':
        case '\'':
        case '\\':
        case '{':
        case '}':
          BPutNumCurly(buf, c);
            break;
        default:
          BPutChar(buf, c);
      }
    } else {
          BPutNumCurly(buf, c);
    }
  }
  BPutChar(buf, '"');
}//
#endif

// Hex Alphabets
const byte* LowerHexAlphabet = "0123456789abcdef";
const byte* UpperHexAlphabet = "0123456789ABCDEF";
//chop

void staticBufFillGap(Buf* buf, word width, word n, bool fill0) {
    if (width > n) {
        word gap = width - n;
        for (byte i = 0; i < gap; i++) {
            BufAppC(buf, fill0? '0' : ' ');
        }
    }
}

void BufAppStringQuoting(Buf* buf, const char* s, word precision) {
  char x;
  for (word i = 0; i<precision; i++) {
      x = *s;
      switch (x) {
         case 9:
            x = 't';
         goto escaped_x;
         case 10:
            x = 'n';
         goto escaped_x;
         case 13:
            x = 'r';
         goto escaped_x;
         case '\"':
            x = '\"';
         goto escaped_x;
         case '\'':
            x = '\'';
         goto escaped_x;
         case '\\':
            x = '\\';
escaped_x:
            BufAppC(buf, '\\');
            BufAppC(buf, x);
         break;
         default:
              if (' ' <= x && x <= '~') {
                // "Printable" ASCII
                BufAppC(buf, x);
              } else {
                // Needs hex escape
                BufAppC(buf, '\\');
                BufAppC(buf, 'x');
                BufAppC(buf, LowerHexAlphabet[(byte)x>>4]);
                BufAppC(buf, LowerHexAlphabet[(byte)x&15]);
              }
      }  // switch
  }  // next s
}

#if 0
static byte* QFormatUnsignedInt(byte* p, unsigned int x) {
  if (x > 9) {
    p = QFormatUnsignedInt(p, x / 10);
    *p++ = '0' + (byte)(x % 10);
  } else {
    *p++ = '0' + (byte)x;
  }
  return (*p = 0), p;
}//
static byte* QFormatSignedInt(byte* p, signed int x) {
  if (x<0) {
    *p++ = '-';
    p = QFormatUnsignedInt(p, (unsigned int)-x);
  } else {
    p = QFormatUnsignedInt(p, (unsigned int)x);
  }
  return p;
}//
const char* StaticFormatSignedInt(int x) {
    byte* p = QFormatSignedInt(ShortStaticBuffer, x);
    *p = 0;
    return (const char*)ShortStaticBuffer;
}//
#endif

byte* staticQFormatUnsignedLong(byte* p, unsigned long x) {
  if (x > 9) {
    p = staticQFormatUnsignedLong(p, x / 10);
    *p++ = '0' + (byte)(x % 10);
  } else {
    *p++ = '0' + (byte)x;
  }
  return (*p = 0), p;
}
byte* staticQFormatSignedLong(byte* p, signed long x) {
  if (x<0) {
    *p++ = '-';
    p = staticQFormatUnsignedLong(p, (unsigned long)-x);
  } else {
    p = staticQFormatUnsignedLong(p, (unsigned long)x);
  }
  return p;
}
const char* StaticFormatSignedLong(int x) {
    byte* p = staticQFormatSignedLong(ShortStaticBuffer, x);
    *p = 0;
    return (const char*)ShortStaticBuffer;
}

byte* staticQFormatLongHex(byte* p, const byte* alphabet, unsigned long x) {
  if (x > 15) {
    p = staticQFormatLongHex(p, alphabet, x >> 4);
    // TODO: report bug that (byte)x did not work.
    *p++ = alphabet[ (byte)(word)x & (byte)15 ];
  } else {
    *p++ = alphabet[ (byte)x ];
  }
  return (*p = 0), p;
}


void BufFormatVA(Buf* buf, const char* format, va_list ap) {
    // Quick Buffer for integer formatting.
    byte qbuf[24]; // TODO use ShortStaticBuffer.

    for (const char* s = format; *s; s++) {
        if (*s != '%') {
            BufAppC(buf, *s);
            continue;
        }

        s++;
        bool fill0 = false, longingly = false;
        word width = 0;
        word precision = 0;
        bool use_precision = 0;
        if (*s == '0') {
            fill0 = true;
            s++;
        }
        while ('0'<=*s && *s<='9') {
          if (use_precision) {
            precision = 10*precision + (*s - '0');
          } else {
            width = 10*width + (*s - '0');
          }
          s++;
        }
        if (*s == 'l') {
            longingly = true;
            s++;
        }
        if (*s == '*') {
            int x = va_arg(ap, int);
          if (use_precision) {
            precision = x;
          } else {
            width = x;
          }
            s++;
        }
        switch (*s) {
        case 'X':
        case 'x': {
            unsigned long x;
            if (longingly) {
                x = va_arg(ap, unsigned long);
            } else {
                x = va_arg(ap, unsigned int);
            }
            
            byte n = (byte)(staticQFormatLongHex(qbuf, ((*s=='X')? UpperHexAlphabet: LowerHexAlphabet), x) - qbuf);
            staticBufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 'u': {
            unsigned long x;
            if (longingly) {
                x = va_arg(ap, unsigned long);
            } else {
                x = va_arg(ap, unsigned int);
            }
            
            byte n = (byte)(staticQFormatUnsignedLong(qbuf, x) - qbuf);
            staticBufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 'd': {
            signed long x;
            if (longingly) {
                x = va_arg(ap, signed long);
            } else {
                x = va_arg(ap, signed int);
            }
            
            byte n = (byte)(staticQFormatSignedLong(qbuf, x) - qbuf);
            staticBufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 's': {
            const char* x = va_arg(ap, const char*);
            if (!x) x = "<NULL>";
            //debug_printf("arg(s)%s ", x);
            precision = (precision == 0) ? strlen(x) : precision;
            staticBufFillGap(buf, width, precision, fill0);
            BufAppS(buf, x, precision);
        }
        break;
        case 'q': {
            const char* x = va_arg(ap, const char*);
            if (!x) x = "<NULL>";
            //debug_printf("arg(s)%s ", x);
            precision = (precision == 0) ? strlen(x) : precision;
            BufAppC(buf, '\"');
            BufAppStringQuoting(buf, x, precision);
            BufAppC(buf, '\"');
        }
        break;
        case 'c': {
            char ch = (char) va_arg(ap, int);
            if (' ' <= ch && ch <= '~') {
                BufAppC(buf, ch);
            } else {
                BufAppC(buf, '<');
                byte n = (byte)(staticQFormatUnsignedLong(qbuf, (byte)ch) - qbuf);
                BufAppS(buf, (const char*)qbuf, n);
                BufAppC(buf, '>');
            }
        }
        break;
        default:
            BufAppC(buf, *s);
        }
    }
}
void BufFormat(Buf* buf, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    BufFormatVA(buf, format, ap);
    va_end(ap);
}

void WritLnAll(int path, const char* s, word n) {
    while (n>0) {
        int wrote = 0;
#ifdef unix
        wrote = write(path, s, n);
        errnum e = wrote < 1 ? errno : 0;
#else
        errnum e = Os9WritLn(path, s, n, &wrote);
#endif
        if (e) {FailE(e, "cannot WritLn path %d", path); return;}
        s += wrote;
        n -= wrote;
    }
}

int Printf(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = buf.n;
    errnum e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    WritLnAll(1, buf.s, buf.n);
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int EPrintf(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = buf.n;
    errnum e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    WritLnAll(2, buf.s, buf.n);
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int FPrintf(File* f, const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = buf.n;
    errnum e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    WritLnAll(f->fd, buf.s, buf.n);
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int SPrintf(char* dest, const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    word n = buf.n;
    BufFinish(&buf);
    memcpy(dest, buf.s, n+1);
    BufDel(&buf);
    return n;
}

char* StrFormat(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    return BufFinish(&buf);
}

// Debugging routines should not call Buf or Malloc.
void PutHex(byte c, word w) {
  // const byte* UpperHexAlphabet = "0123456789ABCDEF";
  // byte* staticQFormatLongHex(byte* p, const byte* alphabet, unsigned long x);
  byte* p = (byte*) ShortStaticBuffer;
  *p++ = '[';
  *p++ = c;
  *p++ = ':';
  p = staticQFormatLongHex(p, UpperHexAlphabet, w);
  *p++ = ']';
  *p = '0';
  FPuts((char*)ShortStaticBuffer, StdErr);
}

void Panic(const char* message) {
  FPuts("\n*** PANIC: ", StdErr);
  FPuts(message, StdErr);
  FPuts("\n", StdErr);
}

void PC_Trace(byte c, const void* w) {
  PutHex(c, (word)w); // TODO
}
