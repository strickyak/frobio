// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frob2/froblib.h>

char buf[8];

void DoCat(File* f) {
    while (true) {
        LogInfo("x.cat: Calling FGets...");
        word cc = FGets(buf, 1, f);
        if (cc == 0) {
          if (ErrNo) { PError("x.cat: FGets: "); exit(ErrNo); }
          break;
        }
        buf[1] = '\0';
        LogInfo("x.cat: Calling FPuts...");
        FPuts(buf, StdOut);
        if (ErrNo) { PError("x.cat: FPuts: "); exit(ErrNo); }
    }
}
int main(int argc, char *argv[]) {
    SkipArg(&argc, &argv);
    Verbosity = 9;

    if (argc == 0) {
        DoCat(StdIn);
    } else for (int i = 0; i < argc; i++) {
      LogInfo("x.cat: Opening file %q", argv[i]);
      File* f = FOpen(argv[i], "r");
      if (!f) {
        PError("FOpen");
        LogFatal("Cannot open %q", argv[i]);
      }
      DoCat(f);
      LogInfo("x.cat: Closing file %q", argv[i]);
      FClose(f);
    }

    return 0;
}
