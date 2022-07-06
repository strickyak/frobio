// f.ping ipaddr

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

static void UsageAndExit() {
    printf("Usage:  f.ping -wWiznetPortHex -cCount -iInterval ipaddr\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  int interval = 10;
  int count = 5;
  while (argc && argv[0][0]=='-') {
    p = argv[0]+2;  // In case needed for parsing.
    switch (argv[0][1]) {
    case 'c':
      count = NyParseDecimalWord(&p);
      break;
    case 'i':
      interval = NyParseDecimalWord(&p);
      break;
    case 'v':
      wiz_verbose = true;
      break;
    case 'w':
      wiz_hwport = (byte*)NyParseHexWord(&p);
      break;
    default:
      UsageAndExit();
    }
    argc--, argv++;  // Discard consumed flag.
  }

  if (argc != 1) {
    UsageAndExit();
  }

  p = argv[0];
  quad addr = NyParseDottedDecimalQuad(&p);
  // Reproduce our interpretation of the dotted quad.
  // char buffer[20];
  // NyFormatDottedDecimalQuad(buffer, addr);
  // TODO: NyFormatDottedDecimalQuad seems BROKEN?

  // Reset and configure.
  int err = wiz_ping(addr);  // int for a cmoc bug workaround!
  if (err) {
    printf("Ping %s: TIMEOUT\n", argv[0]);
  } else {
    printf("Ping %s: OK\n", argv[0]);
  }

  return err;
}
