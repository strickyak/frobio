// f-config MY_ADDR MY_MASK MY_GATEWAY

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

static void UsageAndExit() {
    printf("Usage:  f-config -w0xFF68 ipaddr mask gateway");
    exit(1);
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv);
  while (GetFlag(&argc, &argv, "v:w:")) {
    switch (FlagChar) {
    case 'v':
      Verbosity = (byte) prefixed_atoi(FlagArg);
      break;
    case 'w':
      WizHwPort = (byte*) prefixed_atoi(FlagArg);
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
  bool has_dot = false;
  quad mask;
  for (const char*s = p; *s; s++)
    if (*s == '.') has_dot = true;
  if (has_dot) {
      mask = NyParseDottedDecimalQuad(&p);
  } else {
      byte n = (byte)atoi(p);
      unsigned quad tmp = 0xffffffffL;
      mask = ~(0xFFFFffffLU >> n);
      LogInfo("mask = %lx", mask);
  }

  p = argv[2];
  quad gateway = NyParseDottedDecimalQuad(&p);

  // Reset and configure.
  WizReset();
  WizConfigure(addr, mask, gateway);

  return 0;
}
