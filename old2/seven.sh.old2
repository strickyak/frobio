#!/bin/bash
#
# seven.sh: EXPERIMENTING with compiler/assembler/linker options.

cat <<\HERE >linker.script
define basesympat s_%s
define lensympat l_%s
section start load d
section .text.startup
section .text
section code
section constructors_start
section constructors
section constructors_end
section destructors_start
section destructors
section destructors_end
section initgl_start
section initgl
section initgl_end
section rodata
section rwdata load 1
section bss,bss
section program_end
entry program_start
HERE

cat <<\HERE >z-seven.c
extern int puts(const char* str);

const char* HOWDY_GCC() { return "howdy-gcc!"; }

int main() {
  // puts("howdy gcc!");
  puts(HOWDY_GCC());
  return 7;
}
HERE

cat <<\HERE >z-libc.c
int one_hundred() { return 100; }
void bogus0() { return; }
int bogus1(int x) { return x + one_hundred(); }
int bogus2(int x, int y) { return x+x+y; }

// char PutsBuf[32];
#define PutsBuf ((char*)0x10)
#define sizeof_PutsBuf 32

int puts(const char* str) {
  unsigned i;
  unsigned n = sizeof_PutsBuf;
  for (i=0; i<n; i++) {
    if (str[i] == 0) {
      PutsBuf[i] = '\r';
      break;
    } else {
      PutsBuf[i] = str[i];
    }
  }
  char path = 1;
  char cc;

  asm volatile ("  pshs D,X,Y,U \n");
  asm volatile ("  pshs %0 \n" : : "r" (n)); // n => Y
  asm volatile ("  pshs %0 \n" : : "r" (PutsBuf)); // PutsBuf => X
  asm volatile ("  pshs %0 \n" : : "r" (path)); // path => A
  asm volatile ("  puls Y,X,A \n swi2 \n fcb $8C \n tfr cc,%0" : "=r" (cc));
  asm volatile ("  puls D,X,Y,U \n");

  if (cc & 1) {
    return -1;
  } else {
    return i+1;
  }
}

void exit(int status) {
  asm volatile ("  tfr %0,d \n"
                "  swi2 \n  fcb $06 \n"
                : : "r" (status) );
}
HERE

cat <<\HERE >z-start.s
  .area .text.startup
	.globl __start

__start:
  lbsr #_main
	lbsr #_exit
HERE

alias CC="/opt/yak/fuzix/bin/m6809-unknown-gcc-4.6.4 --std='gnu99' -f'pic' -f'no-builtin' -f'unsigned-char' -f'unsigned-bitfields' -Os"
alias ASM="lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource"

set -eux
CC -S z-seven.c
CC -S z-libc.c

ASM -o z-seven.o z-seven.s -l"z-seven.o.list"
ASM -o z-libc.o z-libc.s -l"z-libc.o.list"
ASM -o z-start.o z-start.s -l"z-start.o.list"

#rm -f libc.a
#lwar -r libc.a z-libc.o
#cp libc.a libgcc.a

#CC -e __start -L. -nostartfiles  -o raw.out z-start.s z-seven.s
#hd raw.out

# UNFORTUNATELY, -f"whole-program" CAUSES lack-of-PCR errors.

lwlink --debug --format=os9 --entry=__start -s linker.script -o z.seven -m z.seven.map z-start.o z-seven.o z-libc.o
mv z.seven z.seven.os9

# --library-path=. 
#--library-path=/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/fpic
#--library-path=/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4
#--library-path=/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/../../../../m6809-unknown/lib
#--library=gcc --library=c --library=gcc

