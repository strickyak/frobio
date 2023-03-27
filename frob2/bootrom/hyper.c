#include "frob2/bootrom/bootrom.h"

#if EMULATED

#undef PrintH
void PrintH(const char* format, ...) {
#ifdef __GNUC__
#if 0
  const char ** fmt_ptr = &format;
  // TODO: ldx fmt_ptr
  asm volatile ("swi\n  fcb 108" : : "m" (fmt_ptr));
#endif
#else
  asm {
    // TODO: ldx fmt_ptr
      swi
      fcb 108
  }
#endif
}

#endif
