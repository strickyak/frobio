#include "frob2/frobtype.h"

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

void BasicPutChar(char ch) {
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
  Delay(500);
}

int main() {
  while (1) {
    BasicClearScreen();
    for (char ch = '0'; ch <= '9'; ch++) BasicPutChar(ch);
    BasicPutChar(0);
    BasicPutChar(0);
    BasicPutChar(0);
    BasicPutChar('\r');
    for (char ch = 'A'; ch <= 'Z'; ch++) BasicPutChar(ch);
    BasicPutChar(' ');
    for (char ch = 'a'; ch <= 'z'; ch++) BasicPutChar(ch);
    BasicPutChar('.');
    BasicPutChar('\r');
    for (word ch = 128; ch <= 255; ch++) BasicPutChar((char)ch);
    BasicPutChar('.');
    BasicPutChar('\r');
    Delay(20000);
  }
  return 0;
}
