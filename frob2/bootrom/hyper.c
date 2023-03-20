#include "frob2/bootrom/bootrom.h"

#if EMULATED

#undef PrintH
void PrintH(const char* format, ...) {
#ifdef __GNUC__
  // Problem: may not be a frame pointer.
  // asm volatile ("swi\n  fcb 108" : : "m" (format));
#else
  asm {
      swi
      fcb 108
  }
#endif
}

#endif
