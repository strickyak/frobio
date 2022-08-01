// f.fgets is like cat, but uses fopen/fgets, mainly as debug or demo.

#include <frobio/nytypes.h>
#include <frobio/nystdio.h>
#include <frobio/ncl/malloc.h>

char buf[300];

int main(int argc, char *argv[]) {
  word stack = 0;
  asm {
    tfr s,d
    std stack
  }
printf("stack=%x\n", stack);


printf("argv=%x\n", argv);
    argc--, argv++;  // Consume unused argv[0]

    if (argc) {
        for (int i = 0; i < argc; i++) {
printf("arg=%x\n", argv[i]);
            #if 0
            FILE* f = fopen(argv[i], "r");
            if (!f) { perror(argv[i]); exit(errno); }

            while (fgets(buf, sizeof buf, f)) {
                int cc = fputs(buf, stdout);
                if (cc<0) { perror(argv[i]); exit(errno); }
            }
            fclose(f);
            #endif
        }
    } else {
            while (fgets(buf, sizeof buf, stdin)) {
                fputs(buf, stdout);
            }
    }
    return 0;
}
