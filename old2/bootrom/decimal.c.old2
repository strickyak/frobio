#include "frob2/frobtype.h"

void putchar(byte ch) {
  asm volatile("lda %0\n ldb %0\n mul" : : "m" (ch) );
}

void PrintDec(word x) {
  byte a='0', b='0', c='0', d='0';
  while (x > 9999) x -= 10000, a++;
  while (x > 999) x -= 1000, b++;
  while (x > 99) x -= 100, c++;
  while (x > 9) x -= 10, d++;
  
  bool must = 0;
  if (a>'0') putchar(a);
  if (must || b>'0') putchar(b), must=1;
  if (must || c>'0') putchar(c), must=1;
  if (must || d>'0') putchar(d), must=1;
  putchar((byte)x);
}
