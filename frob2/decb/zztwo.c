#include "frob2/frobtype.h"
#include "frob2/frobjot/frobjot.h"

extern void ZZPutChar(char ch);
extern void *memset(void *s, int c, size_t n);

word bogus_var_for_delay;
void Delay(word n) {
    for (word i = 0; i < n; i++) {
      bogus_var_for_delay += i;
    }
}

void BasicClearScreen() {
#ifdef __GNUC__
  asm volatile (R"(
    jsr $A928
  )");
#else
  asm {
    jsr $A928
  }
#endif
}

void ZZPutChar(char ch) {
#ifdef __GNUC__
  asm volatile (R"(
    lda %0
    jsr [$A002]
  )" : : "m" (ch) );
#else
  asm {
    lda :ch
    jsr [$A002]
  }
#endif
  Delay(100);
}

char BasicGetChar() {
  char ch;
#ifdef __GNUC__
  asm volatile (R"(
    jsr [$A000]
    sta %0
  )" : "=m" (ch) );
#else
  asm {
    jsr [$A000]
    sta :ch
  }
#endif
  return ch;
}

Jot j;

int main() {
  do {
    BasicClearScreen();
    for (small ch = '0'; ch <= '9'; ch++) ZZPutChar(ch);
    ZZPutChar('\r');
    for (small ch = 'A'; ch <= 'Z'; ch++) ZZPutChar(ch);
    ZZPutChar(' ');
    for (small ch = 'a'; ch <= 'z'; ch++) ZZPutChar(ch);
    ZZPutChar('.');
    ZZPutChar('\r');
    for (word ch = 128; ch <= 255; ch++) ZZPutChar((char)ch);
    ZZPutChar('\r');
  } while (0);

  while (1) {
    char ch = BasicGetChar();
    if (ch) {
      memset(&j, 0, sizeof j);
      JotPrintf(&j, "(%u) ", 255&ch);
      for (small i=0; i<j.len; i++) ZZPutChar(j.s[i]);
    }
  }

  return 0;
}
