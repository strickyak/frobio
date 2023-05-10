// ny: NitroYak: Yak Libs for NitrOS9

#include "frob3/froblib.h"

#define ARGC (*p_argc)
#define ARGV (*p_argv)
#define SKIP (--ARGC, ++ARGV)

//chop

char FlagChar;
const char *FlagArg;

//chop

// Call SkipArg() once before using GetFlag,
// to skip the unused argv0 cell.
void SkipArg(int* p_argc, char*** p_argv) {
    SKIP;
}

// Expects argc, argv to have had the initial argv0 removed (see SkipArg).
// When called, the next option will be found in arg[0].
// Recognizes "--" as a separator at the end of options.
// Recognizes "-" as not an option.
//
// Returns false if no more options.
// The argc non-option args are left starting at argv[0].
//
// Returns true if it consumed an option, or found an error:
//   Sets FlagChar to the option char, or to 1 if error.
//   Sets FlagArg if there is an argument, or to NULL.
bool GetFlag(int* p_argc, char*** p_argv, const char* flagDesc) {
    FlagChar = 0;
    FlagArg = NULL;
    if (!ARGC) return false;
    const char* s = ARGV[0];

    if (s[0] != '-') return false;
    if (!strcmp(s, "-")) return false;
    if (!strcmp(s, "--")) {
        SKIP;
        return false;
    }

    FlagChar = CharDown(s[1]);  // The option char.
    
    // Search for the FlagChar in the flagDesc.
    const char* desc = flagDesc;
    for (; *desc; desc++) if (*desc == FlagChar) break;

    // If option not found, call usage_fn and Fatal.
    if (!desc[0]) {
        SKIP;
        LogError("Bad FlagChar: -%c", FlagChar);
        FlagChar = '?';  // '?' means usage error.
        return true;
    }

    if (desc[1] == ':') {
        // If option takes an argument
        if (s[2]) {
            FlagArg = s+2;
        } else {
            SKIP;
            if (!ARGC) {
                LogError("Missing arg for option: -%c", FlagChar);
                FlagChar = 1;  // 1 means usage error.
                return true;
            }
            FlagArg = ARGV[0];
        }
    } else {
        if (s[2]) {
            LogError("arg not allowed after option: %s", s);
            FlagChar = 1;  // 1 means usage error.
            return true;
        }
    }
    SKIP;
    return true;
}
