typedef unsigned int size_t;

// #include "frob2/decb/std4gcc.h"

void PlatformAbort(void) {
  PlatformPrint(" *ABORT* *LOOP*\n");

  while(1){}
}

void abort() {
  PlatformPrint(" *abort()* ");
  PlatformAbort();
}

void PlatformExit(int status) {
  PlatformPrint(" *EXIT* ");
  PlatformAbort();
}

void PlatformPrint(const char* s) {
    while (*s) {
        PlatformPutChar(*s);
        ++s;
    }
}

void PlatformPutChar(int c) {
#if FOR_DECB
    char ch = (char)c;
    if (ch==10) ch=13;

#if BY_GCC
    asm volatile(
        "lda %0\n\t"
        "jsr [$A002]" : : "m" (ch) : "a" );
#elif BY_CMOC
    asm {
      lda ch
      jsr [$A002]
    }
#else
    -- error --
#endif
#endif // FOR_DECB
}

char* PlatformReadLine() {
    char* LINBUF = (char*) 0x02DC; // for coco
    size_t LBUFMX = 250; // for coco

    // Start at 1, not 0.
    for (size_t i = 1; i < LBUFMX; i++) LINBUF[i]='\0';

#if BY_GCC
    // Not sure what is clobbered, so I save & restore all registers.
    asm volatile("pshs D,X,Y,U\n\tjsr $A390\n\tpuls D,X,Y,U");
#else
    -- error --
#endif
    return LINBUF + 1;
}
