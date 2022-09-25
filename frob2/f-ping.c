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

  const char* parse = argv[0];
  quad addr = NyParseDottedDecimalQuad(&parse);
  // Reproduce our interpretation of the dotted quad.
  // char buffer[20];
  // NyFormatDottedDecimalQuad(buffer, addr);
  // TODO: NyFormatDottedDecimalQuad seems BROKEN?

  errnum err = wiz_ping(addr);  // int for a cmoc bug workaround!
  if (err) {
    // printf("Ping %s: TIMEOUT\n", argv[0]);
    LogError("Ping %s: TIMEOUT", argv[0]);
  } else {
    printf("Ping %s: OK\n", argv[0]);
    LogStatus("Ping %s: OK", argv[0]);
  }

  return err;
}
