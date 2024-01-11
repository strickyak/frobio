// Generate "triples" for poking a sentence to the screen.

#include <stdio.h>

const char splash[] = "So raise your joysticks, raise your keyboards, Let CRTs illuminate the way, With every line, with every byte, For COCO, we pledge allegiance, this day!";

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
  putchar(0);  // Write a final record of all zeros to mark the end.
  putchar(0);
  putchar(0);
}
