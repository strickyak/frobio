// decb-explain < file.decb

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef unsigned char byte;

int main(int argc, char* argv[]) {
  char start[5] = { 0, 0x20, 0x00, 0x30, 0x00 };
  char finish[5] = { 255, 0, 0, 0x30, 0x02 };

  int cc = fwrite(start, 1, 5, stdout);
  assert(cc == 5);

  for (unsigned i = 0; i < 0x2000; i++) {
    putchar(getchar());
  }

  cc = fwrite(finish, 1, 5, stdout);
  assert(cc == 5);

  return 0;
}
