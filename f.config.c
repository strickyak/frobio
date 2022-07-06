// f.config MY_ADDR MY_MASK MY_GATEWAY

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

char payload[] = "!!!!!! Frobio Frobio Frobio Frobio !!!!!!";
byte buf[1500];

static void UsageAndExit() {
    printf("Usage:  f.config ipaddr mask gateway\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  while (argc && argv[0][0]=='-') {
    switch (argv[0][1]) {
    case 'v':
      wiz_verbose = true;
      break;
    case 'w':
      p = argv[0]+2;
      wiz_hwport = (byte*)NyParseHexWord(&p);
      break;
    default:
      UsageAndExit();
    }
    argc--, argv++;  // Discard consumed flag.
  }

  if (argc != 3) {
    UsageAndExit();
  }

  p = argv[0];
  quad addr = NyParseDottedDecimalQuad(&p);
  p = argv[1];
  quad mask = NyParseDottedDecimalQuad(&p);
  p = argv[2];
  quad gateway = NyParseDottedDecimalQuad(&p);

  // Reset and configure.
  wiz_reset();
  wiz_configure(addr, mask, gateway);

  return 0;
}
