

void PutChar(char ch);
void PutStr(const char* s);
void PutHex(word x);
void PutDec(word x);
void Fatal(const char* wut, word arg);
void ShowLine(word line);
void PrintF(const char* format, ...);
void AssertEQ(word a, word b);
void AssertLE(word a, word b);

word tty_ptr = 0x0400;
word tty_begin = 0x0400;
word tty_end = 0x0600;

void PutChar(char ch) {

  // PrintH("CH: %x %c", (byte)ch, (' ' <= ch && ch <= '~') ? (byte)ch : (byte)'?');

    if (ch == 13 || ch == 10) { // Carriage Return
      do {
        PutChar(' ');
      } while ((tty_ptr & 31));
      return;
    }

    word p = tty_ptr;
    if (ch == 8) { // Backspace
      if (p > tty_begin) {
        *(byte*)p = 32;
        --p;
      }
      goto END;
    }

    if (ch == 1) { // Hilite previous char.
      if (p > tty_begin) {
        *(byte*)(p-1) ^= 0x40;  // toggle the inverse bit.
      }
      goto END;
    }

    if (ch < 32) return;  // Ignore other control chars.

    // Only use 64-char ASCII.
    if (96 <= ch && ch <= 126) ch -= 32;
    byte codepoint = (byte)ch;

    *(byte*)p = (0x3f & codepoint);
    p++;

    if (p>=tty_end) {
        for (word i = tty_begin; i< tty_end; i++) {
            if (i < tty_end-32) {
                // Copy the line below.
                *(volatile byte*)i = *(volatile byte*)(i+32);
            } else {
                // Clear the last line.
                *(volatile byte*)i = 32;
            }
        }
        p = tty_end-32;
    }
END:
    *(byte*)p = 0xEF;  // display Blue Box cursor.
    tty_ptr = p;
}

void PutStr(const char* s) {
    while (*s) PutChar(*s++);
}

char HexAlphabet[] = "0123456789ABCDEF";

void PutHex(word x) {
  if (x > 15u) {
    PutHex(x >> 4u);
  }
  PutChar( HexAlphabet[ 15u & x ] );
}
byte DivMod10(word x, word* out_div) { // returns mod
	word div = 0;
	while (x >= 10000) x-=10000, div+=1000;
	while (x >= 1000) x-=1000, div+=100;
	while (x >= 100) x-=100, div+=10;
	while (x >= 10) x-=10, div++;
	*out_div = div;
	return (byte)x;
}
void PutDec(word x) {
  word div;
  if (x > 9u) {
    // eschew div // PutDec(x / 10u);
    DivMod10(x, &div);
    PutDec(div);
  }
  // eschew mod // PutChar('0' + (byte)(x % 10u));
  PutChar('0' + DivMod10(x, &div));
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
    Fatal(":EQ", b);
  }
}

void AssertLE(word a, word b) {
  if (a > b) {
    PutHex(a);
    Fatal(":LE", b);
  }
}

void PrintF(const char* format, ...) {
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
                case 'd': {
                    int x = va_arg(ap, int);
                    if (x<0) {
                      PutChar('-');
                      x = -x;
                    }
                    PutDec((word)x);
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
}

