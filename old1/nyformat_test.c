#include "frobio/nyformat.h"
#include "frobio/ncl/malloc.h"

char sbuf[100];

#if unix // at least, for 2022 x86_64 x86_64 x86_64 GNU/Linux
const char* want = "dec: 1234=1234 -1=-1 1234=1234 65535=4294967295\nhex: 1234=1234 -1=ffffffff 1234=1234 -1=FFFFFFFF 1234=1234 -1=ffffffffffffffff\n";
#else
const char* want = "dec: 1234=1234 -1=-1 1234=1234 65535=65535\nhex: 1234=1234 -1=ffff 1234=1234 -1=FFFF 1234=1234 -1=ffffffff\n";
#endif

int main(int argc, char* argv[]) {
    ny_printf("%s %q [stdout] %d %u %x\n", "Hello", "World", 1, 2, 3);
    ny_eprintf("%s %q [stderr] %d %u %x\n", "Hello", "World", 1, 2, 3);
    ny_sprintf(sbuf, "%s %q [sprintf] %d %u %x\n", "Hello", "World", 1, 2, 3);
    fputs(sbuf, stdout);
    ny_fprintf(stdout, "%s %q [f/stdout] %d %u %x\n", "Hello", "World", 1, 2, 3);
    ny_fprintf(stderr, "%s %q [f/stdout] %d %u %x\n", "Hello", "World", 1, 2, 3);

    Buf b;
    BufInit(&b);
    BufFormat(&b, "dec: 1234=%d -1=%d 1234=%u 65535=%u\n", 1234, -1, 1234, -1);
    BufFormat(&b, "hex: 1234=%x -1=%x 1234=%X -1=%X 1234=%lx -1=%lx\n", 0x1234, -1, 0x1234, -1, 0x1234L, -1L);
    BufFinish(&b);

    const char* got = BufTake(&b);
    printf("\nGOT <%s>\n", got);
    assert(!strcmp(got, want));
    free((void*)got);
    BufDel(&b);
}
