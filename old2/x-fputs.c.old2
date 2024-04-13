// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frob2/froblib.h>

char buf[300];

int main(int argc, char *argv[]) {
    SkipArg(&argc, &argv);
    if (argc != 1) {
        LogFatal("Usage:   ... | f.fputs filename");
        return 2;
    }

    File* f = FOpen(argv[0], "w");
    if (!f) { PError(argv[0]); exit(ErrNo); }

    while (FGets(buf, sizeof buf, StdIn)) {
        FPuts(buf, f);
        if (ErrNo) { PError(argv[0]); exit(ErrNo); }
    }
    FClose(f);
    return 0;
}
