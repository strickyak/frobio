#include "frob2/froblib.h"
#include "frob2/os9/new_caller.h"

int main() {
  struct new_caller nc;
  memset(&nc, 0, sizeof nc);

  nc.os9num = 0x84;  // I$Open
  nc.dab.ab.a = 0x1; // read access
  nc.x = "startup";

  NewCall(&nc);

  LogStatus("  => err=%d  a=%d  b=%d", nc.err, nc.dab.ab.a, nc.dab.ab.b);
}
