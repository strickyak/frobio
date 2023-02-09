#include "frob2/froblib.h"
#include "frob2/frobos9.h"

#if __GNUC__
#if __M6809__

byte disable_irq_count;

void DisableIrqsCounting() {
  // If you use -v9 for Verbosity, you're really debugging
  // and you really want to see all debug logs.
  // Since we skip debug logs when Irqs are disabled,
  // at -v9 we never disable Irqs.  So you're on your own
  // not to cause conflicts at -v9.
  if (Verbosity >= 9) return;

  // Disable interrupts and increment the counter.
  // if counter overflows ($7f to $80), then exit(13).
  asm volatile (
      "orcc\t#$50\t; disable interrupts\n\t"
      "inc\t%0\n\t"
      "bvc\tDisableIrqsEND\n"

      "IrqCounterOverflow\t; if overflow, re-enamble and exit(13).\n\t"
      "andcc\t#^$50\t; enable interrupts\n\t"
      "ldd\t#13\n\t"
      "lbsr _Os9Exit\n"

      "DisableIrqsEND\n"
      : "+m" (disable_irq_count) );
}
void EnableIrqsCounting() {
  if (Verbosity >= 9) return;

  // Decrement counter, and enable if it reaches zero.
  // if counter overflows ($80 to $7f), then exit(13).
  byte enabled = 0;
  asm volatile (
      "dec\t%1\n\t"
      "bne\tEnableIrqsEND\n\t"
      "bvs\tIrqCounterOverflow\n"

      "EnableIrqsEnable\n\t"
      "andcc\t#^$50\t; enable interrupts\n\t"
      "inc %0\n"

      "EnableIrqsEND"
      : "=m" (enabled) , "+m" (disable_irq_count) );
  if (enabled) {
    // fmt is NULL to flush any logging while disabled.
    Logger(NULL, 0, 0, NULL);
  }
}

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

void Os9_print(const char* str) {
  int n = strlen(str);
  while (n) {
    int cc = 0;
    // byte Os9WritLn(int path, const char* buf, int max, int* bytes_written)
    errnum e = Os9WritLn(2, str, n, &cc);
    if (e) Os9Exit(e);
    if (!cc) Os9Exit(245); // 245 = Write Error
    Assert(cc >= 0);
    Assert(cc <= n);
    n -= cc;
    str += cc;
  }
}

#endif // __M6809__
#endif // __GNUC__
