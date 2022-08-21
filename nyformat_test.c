#include "frobio/nyformat.h"

int main(int argc, char* argv[]) {
    Buf b;
    BufInit(&b);
    BufFormat(&b, "dec: 1234=%d -1=%d 1234=%u 65536=%u\n", 1234, -1, 1234, -1);
    BufFormat(&b, "hex: 1234=%x -1=%x 1234=%X -1=%X 1234=%lx -1=%lx\n", 0x1234, -1, 0x1234, -1, 0x1234L, -1L);
    BufFinish(&b);
    const char* got = b.s;
    printf("\nGOT <%s>\n", got);

#if unix
    const char* expect = "dec: 1234=1234 -1=-1 1234=1234 65536=4294967295\nhex: 1234=1234 -1=ffffffff 1234=1234 -1=FFFFFFFF 1234=1234 -1=ffffffffffffffff\n";
#else
    const char* expect = "dec: 1234=1234 -1=-1 1234=1234 65536=4294967295\nhex: 1234=1234 -1=ffffffff 1234=1234 -1=FFFFFFFF 1234=1234 -1=ffffffffffffffff\n";
#endif
    assert(!strcmp(got, expect));
}
