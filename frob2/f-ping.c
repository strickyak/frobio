// f.ping ipaddr

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

static void FatalUsage() {
    LogFatal("Usage:  f.ping -wWiznetPortHex -cCount -iInterval ipaddr");
}

int main(int argc, char* argv[]) {
  // Default flag arguments:
  int interval = 10;
  int count = 5;

  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "c:i:v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'c':
         count = NyParseDecimalWord(&FlagArg);
         break;
      case 'i':
         interval = NyParseDecimalWord(&FlagArg);
         break;
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      case 'w':
         wiz_hwport = (byte*)NyParseHexWord(&FlagArg);
         break;
      default:
        FatalUsage();
    }
  }

  if (argc != 1) {
    FatalUsage();
  }

  const char* host = argv[0];
  const char* parse = host;
  quad addr = NyParseDottedDecimalQuad(&parse);

  errnum err = wiz_ping(addr);
  if (err) {
    LogError("Ping %q: TIMEOUT", host);
  } else {
    LogStatus("Ping %q: OK", host);
  }

  return err;
}
