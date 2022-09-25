// f.config MY_ADDR MY_MASK MY_GATEWAY

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

static void UsageAndExit() {
    printf("Usage:  f.config ipaddr mask gateway\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv);
  while (GetFlag(&argc, &argv, "vw")) {
    switch (FlagChar) {
    case 'v':
      Verbosity = prefixed_atoi(FlagArg);
      break;
    case 'w':
      wiz_hwport = (byte*)NyParseHexWord(&FlagArg);
      break;
    default:
      UsageAndExit();
    }
  }

  if (argc != 3) {
    UsageAndExit();
  }

  const char* p = argv[0];
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
