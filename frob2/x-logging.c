// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frob2/froblib.h>

char buf[300];

int main(int argc, char *argv[]) {
    int z = argc;
    // Verbosity = 9;
    SkipArg(&argc, &argv);
    while (GetFlag(&argc, &argv, "v:xya:b:c:d:z")) {
        const char* a = FlagArg ? FlagArg : "(none)";
        LogStatus("Flag %d=%c Arg %q\n", FlagChar, FlagChar, a);
        if (FlagChar == 'v') Verbosity = prefixed_atoi(FlagArg);
        if (FlagChar == '?') break;
    }

    while (argc) {
        LogError("extra arg: %q\n", argv[0]);
        SkipArg(&argc, &argv);
    }

    LogFatal("Stopping Now: %x", z);
    return 0;
}
