// f-arp ipaddr

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

static void FatalUsage() {
    LogFatal("Usage:  f-arp -w0xFF68 ipaddr");
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      case 'w':
         WizHwPort = (byte*)prefixed_atoi(FlagArg);
         break;
      default:
        FatalUsage();
    }
  }

  if (argc != 1) {
    FatalUsage();
  }

  const char* parse = argv[0];
  quad addr = NyParseDottedDecimalQuad(&parse);

  // Reset and configure.
  byte mac[6];
  errnum err = WizArp(addr, mac);  // int for a cmoc bug workaround!
  if (err) {
    LogFatal("*** TIMEOUT ***");
  } else {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  return 0;
}
