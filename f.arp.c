// f.arp ipaddr

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

static void UsageAndExit() {
    printf("Usage:  f.arp -wWiznetPortHex ipaddr\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  while (argc && argv[0][0]=='-') {
    p = argv[0]+2;  // In case needed for parsing.
    switch (argv[0][1]) {
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
  byte mac[6];
  int err = wiz_arp(addr, mac);  // int for a cmoc bug workaround!
  if (err) {
    printf("*** TIMEOUT ***\n");
  } else {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  return err;
}
