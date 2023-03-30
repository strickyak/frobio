#include "frob2/bootrom/bootrom.h"

word StackPointer() {
  word result;
#ifdef __GNUC__
    asm ("tfr s,%0" : "=g" (result));
#else
    asm {
      tfr s,x
      stx result
    }
#endif
    return result;
}

char PolCat() {
  char inkey = 0;
#if !EMULATED
#ifdef __GNUC__
  asm volatile (R"(
    jsr [$A000]
    sta %0
  )" : "=m" (inkey) );
#else
  asm {
    jsr [$A000]
    sta :inkey
  }
#endif
#endif
  return inkey;
}

void Delay(word n) {
#if !EMULATED
  while (n--) {
#ifdef __GNUC__
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
#else
    asm {
      mul
      mul
      mul
      mul
      mul
    }
#endif
  }
#endif
}

void PutChar(char ch) {
#if EMULATED
  PrintH("CH: %x %c", ch, (' ' <= ch && ch <= '~') ? ch : '?');
  return;
#endif
    if (ch == 13) { // Carriage Return
      do {
        PutChar(' ');
      } while ((Vars->vdg_ptr & 31));
      return;
    }

    if (ch == 8) { // Backspace
      if (Vars->vdg_ptr > VDG_RAM+32) {
        *(byte*)(Vars->vdg_ptr) = 32;
        -- Vars->vdg_ptr;
      }
      return;
    }

    if (ch < 32) return;  // Ignore other control chars.

    // Only use 64-char ASCII.
    if (96 <= ch && ch <= 126) ch -= 32;
    byte codepoint = (byte)ch;

    word p = Vars->vdg_ptr;
    *(byte*)p = (0x3f & codepoint);
    p++;

    if (p>=VDG_END) {
        for (word i = VDG_RAM+32; i< VDG_END; i++) {
            if (i < VDG_END-32) {
                // Copy the line below.
                *(volatile byte*)i = *(volatile byte*)(i+32);
            } else {
                // Clear the last line.
                *(volatile byte*)i = 32;
            }
        }
        p = VDG_END-32;
    }
    Vars->vdg_ptr = p;
    *(byte*)p = 0xEF;  // display Blue Box cursor.
}

void PutStr(const char* s) {
    while (*s) PutChar(*s++);
}

void PutHex(word x) {
  if (x > 15u) {
    PutHex(x >> 4u);
  }
  PutChar( HexAlphabet[ 15u & x ] );
}
void PutDec(word x) {
  if (x > 9u) {
    PutDec(x / 10u);
  }
  PutChar('0' + (byte)(x % 10u));
}

void Fatal(const char* wut, word arg) {
    PutStr(" *FATAL* <");
    PutStr(wut);
    PutChar('$');
    PutHex(arg);
    PutStr("> ");
    while (1) continue;
}

void AssertEQ(word a, word b) {
  if (a != b) {
    PutHex(a);
    Fatal("AEQ", b);
  }
}

void AssertLE(word a, word b) {
  if (a > b) {
    PutHex(a);
    Fatal("ALE", b);
  }
}

#if PRINTK
void printk(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const char* s = format;
    while (*s) {
        if (*s == '%') {
            ++s;
            switch (*s) {
                case 'a': {  // "%a" -> IPv4 address as Dotted Quad.
                    byte* x;
                    x = va_arg(ap, byte*);
                    PutDec(x[0]);
                    PutChar('.');
                    PutDec(x[1]);
                    PutChar('.');
                    PutDec(x[2]);
                    PutChar('.');
                    PutDec(x[3]);
                }
                break;
                case 'x': {
                    word x;
                    x = va_arg(ap, word);
                    PutHex(x);
                }
                break;
                case 'u': {
                    word x = va_arg(ap, word);
                    PutDec(x);
                }
                break;
                case 's': {
                    const char* x = va_arg(ap, const char*);
                    PutStr(x);
                }
                break;
                default:
                    PutChar(*s);
            }
        } else {
            PutChar(*s);
        }
        s++;
    }
    va_end(ap);
    PutChar(';');
}

#endif
