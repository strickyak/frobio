// f.fputs is a text-file-creating sink using fopen/fputs, mainly as debug or demo.
//
// pipeline... | f.fputs filename-to-create

#include <frobio/nytypes.h>
#include <frobio/nystdio.h>

char buf[300];

int main(int argc, char *argv[]) {
    argc--, argv++;  // Consume unused argv[0]

    if (argc != 1) {
        printf("Usage:   ... | f.fputs filename\n");
        return 2;
    }

    FILE* f = fopen(argv[0], "w");
    if (!f) { perror(argv[0]); exit(errno); }

    while (fgets(buf, sizeof buf, stdin)) {
        int cc = fputs(buf, f);
        if (cc<0) { perror(argv[0]); exit(errno); }
    }
    fclose(f);
    return 0;
}
