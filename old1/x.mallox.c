// f.fgets is like cat, but uses fopen/fgets, mainly as debug or demo.

#include <frobio/nytypes.h>
#include <frobio/nystdio.h>
#include <frobio/ncl/malloc.h>

int main(int argc, char *argv[]) {
  word stack = 0;
  asm {
    tfr s,d
    std stack
  }
printf("stack=%x\n", stack);


printf("argc=%x\n", argc);
printf("argv=%x\n", argv);
    argc--, argv++;  // Consume unused argv[0]

    while(true) {
        char* p = malloc(1111);
        printf("malloc 1111 ==> %x\n", p);
    }

    return 0;
}
