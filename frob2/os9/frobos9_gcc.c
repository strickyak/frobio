#include "frob2/froblib.h"
#include "frob2/frobos9.h"

#if __SIZE_MAX__ == 65535U // if gcc 6809

char NybbleToHexDigit(byte a) {
  a &= 15;
  if (a < 10) return a + '0';
  return a + 'A' - 10;
}

void ShowHex(char ch, word a) {
  char buf[9];
  buf[0] = ':';
  buf[1] = ch;
  buf[2] = ':';
  buf[3] = NybbleToHexDigit((byte)(a >> 12));
  buf[4] = NybbleToHexDigit((byte)(a >>  8));
  buf[5] = NybbleToHexDigit((byte)(a >>  4));
  buf[6] = NybbleToHexDigit((byte)(a      ));
  buf[7] = '\r';
  buf[8] = '\0';

  word cc;
  Os9WritLn(1, buf, sizeof buf - 1, &cc);
}

errnum Os9WritLn2(int path, const char* buf, int max, int* bytes_written) {
  return 7;
}

errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written) {
  errnum status = 0;
  byte cc;

  asm volatile ("  pshs Y,U\n");
  asm volatile ("  pshs %0\n" : : "r" (max)); // max => Y
  asm volatile ("  pshs %0\n" : : "r" (buf)); // buf => X
  asm volatile ("  pshs %0\n" : : "r" (path)); // path => D
                                               //
  asm volatile ("  puls Y,X,D\n tfr B,A\n swi2\n fcb $8C\n tfr cc,%0\n" : "=r" (cc));
  // XXX that clobbered B, saving cc.

  asm volatile ("  stb %0\n" : "=m" (status) );
  asm volatile ("  puls Y,U\n");

  if (cc & 1) {
    return status;
  } else {
    return 0;
  }
}

void Os9Exit(byte status) {
  asm volatile ("ldb %0 \n\t"
                "swi2 \n\t"
                "fcb $06"
                : : "m" (status) );
  while (1) {}  // Infinite Loop.
}

#endif // gcc 6809
