// Generate "triples" for poking a sentence to the screen.

#include <stdio.h>

const char splash[] = "We are almost certainly doing it wrong, this is simply what I've run across playing with the new hardware.";

int main() {
  unsigned p = 0x500;
  for (const char* s = splash; *s; s++) {
    putchar( (int)(p >> 8) );
    putchar( (int)(p >> 0) );
    putchar( *s );
    ++p;
  }
}
