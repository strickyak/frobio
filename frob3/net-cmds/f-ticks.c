// f.ticks [-x]

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

static void FatalUsage() {
    LogFatal("Usage:  f.ticks -wWiznetPortHex -x (for hex output)");
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  bool in_hex = false;
  while (GetFlag(&argc, &argv, "v:w:x")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      case 'w':
         WizHwPort = (byte*)prefixed_atoi(FlagArg);
         break;
      case 'x':
        in_hex = true;
        break;
      default:
        FatalUsage();
    }
  }

  if (argc != 0) {
    FatalUsage();
  }

  word ticks = WizTicks();
  if (in_hex) {
    printf("%04x\n", ticks);
  } else {
    printf("%u\n", ticks);
  }

  return 0;
}
