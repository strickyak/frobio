// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frob2/froblib.h>

char buf[300];
File* OutFile;

void CopyLinesFromFileToStdout(File* f) {
    while (true) {
        LogInfo("x.cat: Calling FGets... buf=%x size=%x f=%x", buf, sizeof buf, f);
        word cc = FGets(buf, sizeof buf, f);
        if (cc == 0) {
          if (ErrNo) { PErrorFatal("x.cat: FGets"); }
          LogInfo("FGets returned 0, which should be on EOF.");
          break;
        }

        LogInfo("x.cat: Calling FPuts... cc=%x strlen=%x buf=%q", cc, strlen(buf), buf);
        FPuts(buf, OutFile);
        if (ErrNo) { PErrorFatal("x.cat: FPuts"); }
    }
}

int main(int argc, char *argv[]) {
    SkipArg(&argc, &argv);
    OutFile = StdOut; // by default
    while (GetFlag(&argc, &argv, "o:")) {
      // GetFlag sets FlagChar & FlagArg.
      switch (FlagChar) {
        case 'o':
           OutFile = FOpen(FlagArg, "w");
           break;
      }
      Verbosity = 9;
    }

    if (argc == 0) {
        CopyLinesFromFileToStdout(StdIn);
    } else for (int i = 0; i < argc; i++) {
      LogInfo("x.cat: Opening file %q", argv[i]);
      File* f = FOpen(argv[i], "r");
      if (!f) {
        PErrorFatal("x.cat: FOpen");
      }
      CopyLinesFromFileToStdout(f);
      LogInfo("x.cat: Closing file %q", argv[i]);
      FClose(f);
      if (ErrNo) PErrorFatal("x.cat: FClose");
    }

    if (OutFile != StdOut) {
      LogInfo("x.cat: Closing output file...");
      FClose(OutFile);
      LogInfo("x.cat: Closed output file.");
    }

    LogInfo("x.cat: Returning from main.");
    return 0;
}
