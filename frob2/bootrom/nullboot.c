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

typedef unsigned int word;

void Delay(word n) {
  while (n--) {
#ifdef __GNUC__
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
#else
    asm {
      mul
      mul
      mul
      mul
      mul
    }
#endif
  }
}

#if WHOLE_PROGRAM
#define RomMain main  // Whole Program Optimization requires 'main' instead of 'RomMain'.
#endif
int RomMain () {
    // Poke alphabet over the screen, with stride 3,
    // skipping the first screen line of 32 bytes.
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        *(char*)p = 0x40 | (0x3f & '$');
        Delay(100);
    }
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        // Use 64-char alphabet. Use inverted colors.
        *(char*)p = 0x40 | (0x3f & (char)(p+p+p));
        Delay(100);
    }
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        *(char*)p = 0x40 | (0x3f & '!');
        Delay(100);
    }
    for (unsigned p = 0x0420; p < 0x0600; p++) {
        // Use 64-char alphabet. Use inverted colors.
        *(char*)p = 0x40 | (0x3f & (char)(p+p+p));
        Delay(100);
    }
    return 0;
}
