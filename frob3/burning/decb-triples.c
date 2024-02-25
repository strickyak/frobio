// Generate "triples" for poking a ROM image to CocoIOr EEPROM.

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef unsigned char byte;

byte Q[5];

void WriteTriples(unsigned n, unsigned p) {
  for (unsigned i=0; i<n; i++, p++) {
  	byte hi = (byte)(p>>8);
  	byte lo = (byte)(p>>0);
	byte datum = getchar();
        // A triple to poke the byte to the ROM.
	putchar(hi);
	putchar(lo);
	putchar(datum);
  }
}

int main() {
  int c;
  while (1) {
  	size_t cc = fread(Q, 1, sizeof Q, stdin);
	fprintf(stderr, "(%u, %u)\n", (unsigned)sizeof Q, (unsigned)cc);
	assert(cc == sizeof Q);
	switch (Q[0]) {
	case 0:
		unsigned n = (Q[1]<<8) | Q[2];
		unsigned p = (Q[3]<<8) | Q[4];
		WriteTriples(n, p);
		break;
	case 255:
		goto done;
	default:
		assert(false);
	}
  }
done:
  // Write a final record of all zeros to mark the end.
  putchar(0);
  putchar(0);
  putchar(0);
  return 0;
}
