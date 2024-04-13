#include "frob2/frobtype.h"

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
}
