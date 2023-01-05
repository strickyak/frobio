// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frob2/froblib.h>

char buf[300];

void CopyLinesFromFileToStdout(File* f) {
    while (true) {
        LogInfo("x.cat: Calling FGets...");
        word cc = FGets(buf, sizeof buf, f);
        if (cc == 0) {
          if (ErrNo) { PErrorFatal("x.cat: FGets"); }
          break;
        }

        LogInfo("x.cat: Calling FPuts... cc=%x strlen=%x buf=%q", cc, strlen(buf), buf);
        FPuts(buf, StdOut);
        if (ErrNo) { PErrorFatal("x.cat: FPuts"); }
    }
}
int main(int argc, char *argv[]) {
    SkipArg(&argc, &argv);
    Verbosity = 9;

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
      if (PError) PErrorFatal("x.cat: FClose");
    }

    return 0;
}
