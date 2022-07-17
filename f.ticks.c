// f.ticks [-x]

#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

static void UsageAndExit() {
    printf("Usage:  f.ticks -wWiznetPortHex -x (for hex output)\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  bool in_hex = false;
  while (argc && argv[0][0]=='-') {
    p = argv[0]+2;  // In case needed for parsing.
    switch (argv[0][1]) {
    case 'v':
      wiz_verbose = true;
      break;
    case 'w':
      wiz_hwport = (byte*)NyParseHexWord(&p);
      break;
    case 'x':
      in_hex = true;
      break;
    default:
      UsageAndExit();
    }
    argc--, argv++;  // Discard consumed flag.
  }

  if (argc != 0) {
    UsageAndExit();
  }

  word ticks = wiz_ticks();
  if (in_hex) {
    printf("%04x\n", ticks);
  } else {
    printf("%u\n", ticks);
  }

  return 0;
}
