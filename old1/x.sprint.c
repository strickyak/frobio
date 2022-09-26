// For debugging why cmoc's sprintf HCFs on OS9.

#include "frobio/nytypes.h"

/*
Here's why:

They're mixing up ,Y and ,PCR opcodes.
    CHROUT should be ,Y
    chrtomem should be ,PCR

Why is this listing different from the source?

$ cat -n /opt/build/cmoc-0.1.77/src/stdlib/sprintf.asm
    11	* int sprintf(char *str, const char *format, ...);
    12	*
    13	_sprintf
    14		ldx	CHROUT,pcr
    15		stx	sprintf_oldCHROUT,pcr	preserve initial output routine address
    16		leax	chrtomem,pcr		install chrtomem as destination of printf()
    17		stx	CHROUT,pcr

But here it's all ,Y:

$ cat -n /home/strick/COCO/build/cmoc-rebuild/src/stdlib/sprintf.os9_o.list
    20	000E                  (sprintf.asm.os9as):00020         _sprintf
    21	000E AEA90000         (sprintf.asm.os9as):00021                 ldx     CHROUT,Y
    22	0012 AFA90000         (sprintf.asm.os9as):00022                 stx     sprintf_oldCHROUT,Y     preserve initial output routine address
    23	0016 30A90000         (sprintf.asm.os9as):00023                 leax    chrtomem,Y              install chrtomem as destination of printf()
    24	001A AFA90000         (sprintf.asm.os9as):00024                 stx     CHROUT,Y
    25	                      (sprintf.asm.os9as):00025
*/

char buf[111];

int main(int argc, char *argv[]) {
    unsigned stack;
    asm {
      sts stack
    }
    printf("main %04x argc %04x argv1 %04x stack %04x\n", main, argc, argv[1], stack);
    sprintf(buf, "one %d three %x five %s seven", 2, 4, "SIX");
    printf("< %s >\n", buf);
    return 0;
}
