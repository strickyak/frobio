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
//#include "frob2/froblib.h"
#include "frob2/frobjot/frobjot.h"

const byte HexAlphabet[] = "0123456789ABCDEF";

bool JotNiceCharP(char c) {
    return (' ' <= c && c <= '~' || c=='\r');
}

// To initialize a Jot: struct j62 = {0};

void JotAppC(Jot* j, char c) {
  if (c=='\n') c='\r';
  if (JotNiceCharP(c)) {
    if (j->len < JOT_MAX) {
      j->s[j->len++] = c;
      j->s[j->len] = '\0';
    } else {
      j->s[JOT_MAX-1] = '#'; // Overflow char.
    }
  } else {
    JotAppC(j, '\\');
    byte b = (byte)c;
    byte b3 = b>>3;
    JotAppC(j, '0'+ (7 & (b3>>3)));
    JotAppC(j, '0'+ (7 & (b3)));
    JotAppC(j, '0'+ (7 & b));
  }
}

void JotAppU(Jot* j, word x) {
  if (x > 9) {
    JotAppU(j, x/10);
  }
  JotAppC(j, '0' + (char)(x % 10));
}

void JotAppX(Jot* j, word x) {
  if (x > 15) {
    JotAppX(j, x>>4);
  }
  JotAppC(j,  HexAlphabet[ x & 15 ]);
}

void JotAppS(Jot* j, const char* x) {
  while (*x)
    JotAppC(j, *x++);
}

void JotAppJot(Jot* j, const Jot* x) {
  for (byte i = 0; i < x->len; i++)
    JotAppC(j, x->s[i]);
}

void JotFormatVA(Jot* j, const char* format, va_list ap) {
  for (const char* s = format; *s; s++) {
    if (*s != '%') {
      JotAppC(j, *s);
      continue;
    }
    s++;
    switch (*s) {
      case 'x': 
        // word x = va_arg(ap, word);
        JotAppX(j, va_arg(ap, word));
               
        break;

      case 'd': 
      case 'u': 
        // word u = va_arg(ap, word);
        JotAppU(j, va_arg(ap, word));
               
        break;

      case 's': 
        // const char* s = va_arg(ap, const char*);
        JotAppS(j, va_arg(ap, const char*));
               
        break;

      default:
        JotAppC(j, *s);
    }
  }
}

void JotPrintf(Jot* j, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    JotFormatVA(j, format, ap);
    va_end(ap);
}
