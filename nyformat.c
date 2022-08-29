#include <stdarg.h>
#include "frobio/nyformat.h"
#include "frobio/nystdio.h"

#ifdef unix
#include <unistd.h>
#include <errno.h>
#else
#include "frobio/os9call.h"
#endif

#define Debug printf

// Up(c): convert to upper case for 26 ascii letters.
char Up(char c) {
  return ('a' <= c && c <= 'z') ? c - 32 : c;
}

// Down(c): convert to lower case for 26 ascii letters.
char Down(char c) {
  return ('A' <= c && c <= 'Z') ? c + 32 : c;
}

#if 0  // FOR CURLY
void BPutNumCurly(Buf* buf, byte c) {
          BPutChar(buf, '{');
          BPutU(buf, (word)c);
          BPutChar(buf, '}');
}


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
}
#endif

// Hex Alphabets
const byte* LowerHexAlphabet = "0123456789abcdef";
const byte* UpperHexAlphabet = "0123456789ABCDEF";

static void BufFillGap(Buf* buf, byte width, byte n, bool fill0) {
    if (width > n) {
        byte gap = width - n;
        for (byte i = 0; i < gap; i++) {
            BufAppC(buf, fill0? '0' : ' ');
        }
    }
}

void BufAppStringQuoting(Buf* buf, const char* s) {
  for (; *s; s++) {
      switch (*s) {
         case 9:
            BufAppC(buf, '\\');
            BufAppC(buf, 't');
         break;
         case 10:
            BufAppC(buf, '\\');
            BufAppC(buf, 'n');
         break;
         case 13:
            BufAppC(buf, '\\');
            BufAppC(buf, 'r');
         break;
         case '\"':
            BufAppC(buf, '\\');
            BufAppC(buf, '\"');
         break;
         case '\'':
            BufAppC(buf, '\\');
            BufAppC(buf, '\'');
         break;
         default:
              if (' ' <= *s && *s <= '~') {
                // "Printable" ASCII
                BufAppC(buf, *s);
              } else {
                // Needs hex escape
                BufAppC(buf, '\\');
                BufAppC(buf, 'x');
                BufAppC(buf, LowerHexAlphabet[*s>>4]);
                BufAppC(buf, LowerHexAlphabet[*s&15]);
              }
      }  // switch
  }  // next s
}


// Define or Undef:
#undef LONG_DECIMAL

#ifdef LONG_DECIMAL
#define DECIMAL_TYPE long
#else
#define DECIMAL_TYPE int
#endif

static byte* QFormatUnsignedInt(byte* p, unsigned DECIMAL_TYPE x) {
  if (x > 9) {
    p = QFormatUnsignedInt(p, x / 10);
    *p++ = '0' + (byte)(x % 10);
  } else {
    *p++ = '0' + (byte)x;
  }
  return (*p = 0), p;
}
static byte* QFormatSignedInt(byte* p, signed DECIMAL_TYPE x) {
  //Debug("signed! ");
  if (x<0) {
    *p++ = '-';
    p = QFormatUnsignedInt(p, (unsigned DECIMAL_TYPE)-x);
  } else {
    p = QFormatUnsignedInt(p, (unsigned DECIMAL_TYPE)x);
  }
  return p;
}

static byte* QFormatLongHex(byte* p, const byte* alphabet, unsigned long x) {
  //Debug("(LongHex in %x < %lx) ", (unsigned)(word)p, x);
  if (x > 15) {
    p = QFormatLongHex(p, alphabet, x >> 4);
    // TODO: report bug that (byte)x did not work.
    *p++ = alphabet[ (byte)(word)x & (byte)15 ];
  } else {
    *p++ = alphabet[ (byte)x ];
  }
  //Debug("(LongHex out %x >) ", (unsigned)(word)p);
  return (*p = 0), p;
}


void BufFormatVA(Buf* buf, const char* format, va_list ap) {
    // Quick Buffer for integer formatting.
    byte qbuf[20];

    // va_list ap;
    // va_start(ap, format);
    for (const char* s = format; *s; s++) {
        if (*s != '%') {
            BufAppC(buf, *s);
            continue;
        }

        s++;
        bool fill0 = false, longingly = false;
        byte width = 0;
        if (*s == '0') {
            fill0 = true;
            s++;
        }
        while ('0'<=*s && *s<='9') {
            width = 10*width + (*s - '0');
            s++;
        }
        if (*s == 'l') {
            longingly = true;
            s++;
        }
        switch (*s) {
        case 'X':
        case 'x': {
            unsigned long x;
            if (longingly) {
                x = va_arg(ap, unsigned long);
                //Debug("arg(ul)%lx ", x);
            } else {
                x = va_arg(ap, unsigned);
                //Debug("arg(unsigned)%lx ", x);
            }
            
            byte n = (byte)(QFormatLongHex(qbuf, ((*s=='X')? UpperHexAlphabet: LowerHexAlphabet), x) - qbuf);
            BufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 'u': {
            unsigned DECIMAL_TYPE x;
#ifdef LONG_DECIMAL
            if (longingly) {
                x = va_arg(ap, unsigned DECIMAL_TYPE);
                //Debug("arg(ul)%lx ", x);
            } else {
                x = va_arg(ap, unsigned);
                //Debug("arg(unsigned)%lx ", x);
            }
#else
            assert(!longingly);
            x = va_arg(ap, unsigned DECIMAL_TYPE);
#endif
            // unsigned x = va_arg(ap, unsigned);
            //Debug("arg(u)%lu ", x);
            byte n = (byte)(QFormatUnsignedInt(qbuf, x) - qbuf);
            BufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 'd': {
            signed DECIMAL_TYPE x;
#ifdef LONG_DECIMAL
            if (longingly) {
                x = va_arg(ap, signed DECIMAL_TYPE);
                //Debug("arg(ul)%lx ", x);
            } else {
                x = va_arg(ap, signed int);
                //Debug("arg(unsigned)%lx ", x);
            }
#else
            assert(!longingly);
            x = va_arg(ap, unsigned DECIMAL_TYPE);
#endif
            //Debug("arg(d)%ld ", x);
            byte n = (byte)(QFormatSignedInt(qbuf, x) - qbuf);
            BufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 's': {
            const char* x = va_arg(ap, const char*);
            if (!x) x = "<NULL>";
            //Debug("arg(s)%s ", x);
            byte n = (byte) strlen(x);
            BufFillGap(buf, width, n, fill0);
            BufAppS(buf, x, n);
        }
        break;
        case 'q': {
            const char* x = va_arg(ap, const char*);
            if (!x) x = "<NULL>";
            //Debug("arg(s)%s ", x);
            word n = (word) strlen(x);
            BufAppC(buf, '\"');
            BufAppStringQuoting(buf, x);
            BufAppC(buf, '\"');
        }
        break;
        default:
            BufAppC(buf, *s);
        }
    }

    // va_end(ap);
}
void BufFormat(Buf* buf, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    BufFormatVA(buf, format, ap);
    va_end(ap);
}

// TODO: this did not actually fix it.
// TODO: Use WritLn, repeatedly to each \r, \n, or \r\n.
void FixNewlines(char* s, word n) {
#if 0
    if (n<256) {
        byte bn = (byte) n;
        for (byte i = 0; i<bn; i++) {
            if (*s == 10) *s = 13;
            s++;
        }
    } else for (word i = 0; i<n; i++) {
            if (*s == 10) *s = 13;
            s++;
    }
#endif
}

// int vsprintf(char* s, const char* format, va_list arg);

int ny_printf(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = 0;
    error e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    FixNewlines(buf.s, buf.n);
    e = Os9Write(1, buf.s, buf.n, &bytes_written); 
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int ny_eprintf(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = 0;
    error e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    FixNewlines(buf.s, buf.n);
    e = Os9Write(2, buf.s, buf.n, &bytes_written); 
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int ny_fprintf(NY_FILE* f, const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = 0;
    error e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    FixNewlines(buf.s, buf.n);
    e = Os9Write(f->fd, buf.s, buf.n, &bytes_written); 
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int ny_sprintf(char* dest, const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    memcpy(dest, buf.s, buf.n);
    BufDel(&buf);
    return buf.n;
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
