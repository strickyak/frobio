// x-logging -- sample code using logging.

#include <frob2/froblib.h>

char buf[300];

int main(int argc, char *argv[]) {
    SkipArg(&argc, &argv);
    while (GetFlag(&argc, &argv, "v:xya:b:c:d:z")) {
        const char* a = FlagArg ? FlagArg : "(none)";
        LogStatus("Flag %d=%c Arg %q\n", FlagChar, FlagChar, a);
        if (FlagChar == 'v') Verbosity = (byte)prefixed_atoi(FlagArg);
        if (FlagChar == '?') break;
    }

    while (argc) {
        LogError("extra arg: %q\n", argv[0]);
        SkipArg(&argc, &argv);
    }

    LogFatal("Stopping Now: %q = $%x", "$888", 0x888);
    return 0;
}
