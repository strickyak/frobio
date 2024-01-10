// Generate "triples" for poking a sentence to the screen.

#include <stdio.h>

const char splash[] = "We are almost certainly doing it wrong, this is simply what I've run across playing with the new hardware.";

int main() {
  unsigned p = 0x500;
  for (const char* s = splash; *s; s++) {
    putchar( (int)(p >> 8) );
    putchar( (int)(p >> 0) );
    if (*s > 0x60) {
      putchar( *s - 0x60 );  // Change lower case to upper case.
    } else {
      putchar( 0x3f & *s );  // Use bright on dark.
    }
    ++p;
  }
}
