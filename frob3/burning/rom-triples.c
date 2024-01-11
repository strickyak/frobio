// Generate "triples" for poking a ROM image to CocoIOr EEPROM.

#include <stdio.h>

#define ROM 0xC000
#define N (8*1024) /* rom size */
char RomImage[N];

int main() {
  int c;
  // First read in the RomImage, to make sure the input
  // has exactly N bytes.
  for (int i = 0; i < N; i++ ) {
    c = getchar();
    if (c < 0) {
	fprintf(stderr, "ERROR: short input; i=%d\n", i);
	return 2;
    }
    RomImage[i] = c;
  }
  c = getchar();
  if (c >= 0) {
    fprintf(stderr, "ERROR: long input\n");
    return 2;
  }
  unsigned p = 0x500;  // Address of VDG mid-screen.
  unsigned q = N / 256; // 256 bytes in bottom half of screen.
  for (unsigned i = 0; i < N; i++ ) {
    // A triple to poke the byte to the ROM.
    putchar( (int)((ROM+i) >> 8) );
    putchar( (int)((ROM+i) >> 0) );
    putchar( RomImage[i] );
    if (i % q == (q-1)) {
      // After every q pokes to ROM, poke a progress spot on the screen.
      putchar( (int)(p >> 8) );
      putchar( (int)(p >> 0) );
      putchar( '#' );
      ++p;
    }
  }
  // Write a final record of all zeros to mark the end.
  putchar(0);
  putchar(0);
  putchar(0);
  return 0;
}
