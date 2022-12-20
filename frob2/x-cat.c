// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frob2/froblib.h>

char buf[8];

int main(int argc, char *argv[]) {
    SkipArg(&argc, &argv);
    if (argc != 0) {
        LogFatal("Usage:   ... | x.cat | ...");
        return 2;
    }

    while (FGets(buf, 1, StdIn)) {
        buf[1] = '\0';
        FPuts(buf, StdOut);
        if (ErrNo) { PError("x.cat"); exit(ErrNo); }
    }
    return 0;
}
