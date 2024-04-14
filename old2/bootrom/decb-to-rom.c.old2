// decb-explain < file.decb

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef unsigned char byte;

byte quint[5];
byte rom[8 << 10];

int main(int argc, char* argv[]) {
  memset(rom, 0xFF, sizeof rom);

  while(1) {
    for (unsigned i=0; i<5; i++) {
      int c = getchar();
      if (c == EOF) goto EXIT;
      quint[i] = (byte)c;
    }

    byte cmd = quint[0];
    unsigned n = (quint[1]<<8) | quint[2];
    unsigned p = (quint[3]<<8) | quint[4];

    if (cmd == 0) {
      fprintf(stderr, "Block n=%04x p=%04x (End=%04x)\n", n, p, n+p);

      if (p == 0xC000) assert(n < 0x80);
      else if (p == 0xC100) assert(n < 0xF0);
      else if (p == 0xC200) assert(n < 0x05F0);
      else if (p == 0xC900) assert(n < 0x1600);
      else assert(!"bad p");

      for (unsigned k=0; k<n; k++) {
        rom[p+k-0xC000] = (byte)getchar();
      }
    } else if (cmd == 255) {
      fprintf(stderr, "Jump (n=%04x) To p=%04x\n", n, p);
    } else {
      fprintf(stderr, "Unknown command %02x", cmd);
    }
  }

EXIT:
  int cc = fwrite(rom, 1, sizeof rom, stdout);
  assert(cc == sizeof rom);
  return 0;
}
