#include "frobio/nyformat.h"
#include <stdarg.h>

#define Debug printf

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

void BufFillGap(Buf* buf, byte width, byte n, bool fill0) {
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

#if 1

// Define or Undef:
#undef LONG_DECIMAL

#ifdef LONG_DECIMAL
#define DECIMAL_TYPE long
#else
#define DECIMAL_TYPE int
#endif

byte* QFormatUnsignedInt(byte* p, unsigned DECIMAL_TYPE x) {
  if (x > 9) {
    p = QFormatUnsignedInt(p, x / 10);
    *p++ = '0' + (byte)(x % 10);
  } else {
    *p++ = '0' + (byte)x;
  }
  return (*p = 0), p;
}
byte* QFormatSignedInt(byte* p, signed DECIMAL_TYPE x) {
  //Debug("signed! ");
  if (x<0) {
    *p++ = '-';
    p = QFormatUnsignedInt(p, (unsigned DECIMAL_TYPE)-x);
  } else {
    p = QFormatUnsignedInt(p, (unsigned DECIMAL_TYPE)x);
  }
  return p;
}

byte* QFormatLongHex(byte* p, const byte* alphabet, unsigned long x) {
  Debug("(LongHex in %x < %lx) ", (unsigned)(word)p, x);
  if (x > 15) {
    p = QFormatLongHex(p, alphabet, x >> 4);
    // TODO: report bug that (byte)x did not work.
    *p++ = alphabet[ (byte)(word)x & (byte)15 ];
  } else {
    *p++ = alphabet[ (byte)x ];
  }
  Debug("(LongHex out %x >) ", (unsigned)(word)p);
  return (*p = 0), p;
}


void BufFormat(Buf* buf, const char* format, ... /*va_list arg*/) {
    // Quick Buffer for integer formatting.
    byte qbuf[20];

    va_list ap;
    va_start(ap, format);
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
            //Debug("arg(s)%s ", x);
            byte n = (byte) strlen(x);
            BufFillGap(buf, width, n, fill0);
            BufAppS(buf, x, n);
        }
        break;
        case 'q': {
            const char* x = va_arg(ap, const char*);
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

    va_end(ap);
}
#endif

// int vsprintf(char* s, const char* format, va_list arg);

#if 0
int ny_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    for () {
        // type x = va_arg(ap, type);
        int x = va_arg(ap, int);
    }

    va_end(ap, fmt);
}
#endif

#if 0
int low__FormatToStaticBuffer(Buf* buf, String s, Slice args) {
  BufferP = StaticBuffer;
  BufferEnd = StaticBuffer + sizeof(StaticBuffer);

  P__any_* a = (P__any_*)(args.base + args.offset);
  P__any_* a_end = a + (args.len / sizeof(*a));

  byte* p = (byte*)(s.base + s.offset);
  byte* p_end = p + s.len;

  for (; p < p_end; p++) {
    byte c = *p;
    if (c == 0/*EOS*/) goto END;
    if (c=='%') {
        ++p;
        c = *p;

        if (a >= a_end) {
          BPutStr(buf, "<end>");
        } else {

          if (c == 0/*EOS*/) goto END;

          if (c == 'T') {
            BPutStr(buf, a->typecode);
          } else {
            switch (a->typecode[0]) {
              case 's': // case string
              case 'S': // case Slice (actually for slice of bytes) pun as String:
                {
                  String* xp = (String*)a->pointer;
                  if (c=='q')
                    FormatQ((byte*)(xp->base + xp->offset), xp->len);
                  else
                    BPutStrN(buf, (const char*)(xp->base + xp->offset), xp->len);
                }
                break;
              case 'z': // case bool
                BPutStr(buf,  (*(P_bool*)a->pointer) ? "true" : "false");
                break;
              case 'b':  // case byte
                BPutU(buf, *(P_byte*)a->pointer);
                break;
              case 'i':  // case int
                BPutI(buf, *(P_int*)a->pointer);
                break;
              case 'u': // case uint
                BPutU(buf, *(P_uint*)a->pointer);
                break;
              case 'p': // case pointer
                BPutStr(buf,  "(*)" );
                BPutU(buf, (P_uintptr)*(void**)a->pointer);
                break;
              default: // default: Unhandled Type
                BPutStr(buf, "(percent "); BPutU(buf, c);
                BPutStr(buf, " typecode "); BPutStr(buf, a->typecode);
                BPutStr(buf, " pointer "); BPutU(buf, (P_uintptr)a->pointer);
                BPutStr(buf, " * "); BPutU(buf, ((P_uintptr*)a->pointer)[0]);
                BPutStr(buf, " * "); BPutU(buf, ((P_uintptr*)a->pointer)[1]);
                BPutStr(buf, ")");
                break;
            }
          }
      }
      a++;
    } else {
      // Not a % escape -- just a normal char
      BPutChar(buf, *p);
    }
  }  // next byte *p
END:
  return BufferP - StaticBuffer;
}  // end low__FormatToBuffer
#endif
