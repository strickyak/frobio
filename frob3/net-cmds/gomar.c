// f.ticks [-x]

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

#define streq !strcmp

static void FatalUsage() {
    LogFatal("Usage:  gomar { EXIT | EXIT n | SHOW str | FATAL }\n");
}

void ToUpper(char* s) {
	for ( ; *s; s++) {
		if ('a' <= *s && *s <= 'z') *s -= 32;
	}
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.

  while (GetFlag(&argc, &argv, "v:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      default:
        FatalUsage();
    }
  }

  if (argc == 0) {
    FatalUsage();
  }
  ToUpper(argv[0]);

  if (argc==1 && streq(argv[0], "EXIT")) {
  	LogStatus("gomar: about to EXIT");
   	asm {
		ldx #0
		nop
		brn 2+.+107
	}
  } if (argc==2 && streq(argv[0], "EXIT")) {
  	int status = prefixed_atoi(argv[1]);
  	LogStatus("gomar: about to EXIT %d", status);
   	asm {
		ldx :status
		nop
		brn 2+.+107
	}
  } else if (argc==2 && streq(argv[0], "SAY")) {
 	char* str = argv[1];
  	LogStatus("gomar: about to SHOW: %q", str);
   	asm {
		ldd :str
		nop
		brn 2+.+110
	}
  } else if (argc==1 && streq(argv[0], "FATAL")) {
  	LogStatus("gomar: about to FATAL");
   	asm {
		nop
		brn 2+.+100
	}
  } else {
  	LogStatus("gomar: Unknown command: %q", argv[0]);
  	LogStatus("gomar: about to FATAL");
   	asm {
		nop
		brn 2+.+100
	}
  }

  return 0;
}
