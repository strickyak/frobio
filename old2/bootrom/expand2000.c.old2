#include <stdio.h>
#include <assert.h>
#include <memory.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef size_t un;

struct expect {
  word page;
  word sum;
  word xor;
};

byte buf[0x2000];

word swap(word x) {
  byte a = (byte)(x>>8);
  byte b = (byte)x;
  word z = (b<<8) | a;
  return z;
}

void MakeChecksum() {
  struct expect* expect = (struct expect*) (buf+sizeof buf);
  for (word a = 0x0000; a < 0x1F00; a+=0x0100) {
    word sum = 0;
    word xor = 0;
    for (word b=0; b < 256; b+=2) {
      word w = swap(*(word*)(buf+a+b));
      sum += w;
      xor ^= w;
    }
    expect = (struct expect*) (((un)expect) - 6);
    expect->page = swap(a);
    expect->sum = swap(sum);
    expect->xor = swap(xor);

    fprintf(stderr, "%x: %x %x\n", a, sum, xor);
  }
}

int main(int argc, char* argv[]) {
  word zero = (word) getchar(); assert(zero==0);
  word hi = (word) getchar();
  word lo = (word) getchar();
  word n = (hi<<8) + lo;
  fprintf(stderr, "zero=%x h=%x l=%x n=%x\n", zero, hi, lo, n);
  hi = (word) getchar();
  lo = (word) getchar();

  putchar(0); putchar(0x20); putchar(0x00); putchar(hi); putchar(lo);

  for (word i = 0; i < n; i++) buf[i] = (byte) getchar();

  zero = (word) getchar(); assert(zero == 255);

  MakeChecksum();

  for (word i = 0; i < 0x2000; i++) putchar(buf[i]);

  putchar(0xFF);
  putchar(0x00);
  putchar(0x00);
  putchar(0x30);
  putchar(0x00);
}
