// nullboot.c
//
// A demo program to do nothing but write a distinctive
// pattern on the screen as Proof Of Concept, and return.

// Hints:
//   Linux:
//     make nullboot.wav
//   Coco:
//     CLOADM"NULLBOOT
//   Linux:
//     aplay nullboot.wav
//   Coco:
//     EXEC &H2600

unsigned delay(unsigned n) {
  unsigned sum;
  for (unsigned k = 0; k < n; k++) {
      for (unsigned i = 0; i < 0x80; i++) {
        sum += (int)i;
      }
  }
  return sum;
}
int main () {
    // Poke alphabet over the screen, with stride 3,
    // skipping the first screen line of 32 bytes.
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        *(char*)p = 0x40 | (0x3f & '$');
        delay(1);
    }
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        // Use 64-char alphabet. Use inverted colors.
        *(char*)p = 0x40 | (0x3f & (char)(p+p+p));
        delay(1);
    }
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        *(char*)p = 0x40 | (0x3f & '!');
        delay(1);
    }
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        // Use 64-char alphabet. Use inverted colors.
        *(char*)p = 0x40 | (0x3f & (char)(p+p+p));
        delay(1);
    }
    return 0;
}
