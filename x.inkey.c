/* x.inkey [n]

   n is how many keystrokes to read.  Default is 1.

   This is a demo of INKEY and what codes each key produces.
   Assumes the terminal is both stdin and stdout.
   (It also shows you the SS.Opt bytes of stdout,
    and turns off automatic key echo.)

   Try "x.inkey 9999" and type a bunch of keys.
   Try SHIFT and CTRL and ALT with a bunch of keys.
   Hit BREAK to stop.
*/

#include "frobio/nytypes.h"
#include "frobio/os9call.h"

byte options[32];

int main(int argc, char *argv[]) {
    int n = argc>1 ? atoi(argv[1]) : 1;
    int x = (int)options;
    int u = 0;
    int d = 0;

    error e = Os9GetStt(1/*=stdout*/, 0/*=SS.Opt*/, &d, &x, &u);
    if (e) { printf("GetStt<%d> ", e); return e; }

    for (byte i = 0; i < 32; i+=8) {
        for (byte j=i; j<i+8; j++) {
            printf("%02x ", options[j]);
        }
        printf(" ");
        if (i&16) printf("\n");
    }

    options[4/*=PD.EKO - $20*/] = 0/*= no echo */;
    x = (int)options;
    e = Os9SetStt(1/*=stdout*/, 0/*=SS.Opt*/, &d, &x, &u);
    if (e) { printf("SetStt<%d> ", e); return e; }

    for (int i = 0; i < n; i++) {
        char buf[1];
        int cc = 0;
        e = Os9Read(0, buf, 1, &cc);
        if (e) printf("E<%d> ", e);
        else if (cc!=1) printf("CC<%d> ", cc);
        else printf("(%d) ", buf[0]);
    }
    printf("\n");
    return 0;
}
