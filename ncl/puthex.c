#include "frobio/nytypes.h"
#include "frobio/os9call.h"
#include "frobio/ncl/puthex.h"

char hexchar(byte i) {
  if (0 <= i && i <= 9)
    return (char) ('0' + i);
  if (10 <= i && i <= 15)
    return (char) ('A' + i - 10);
  return '?';
}

// puthex prints a prefix and a hex number, like `(p=1234)`,
// only using a small buffer.  Quick and reliable for debugging.
void puthex(char prefix, int a) {
  char buf[9];
  word x = (word) a;
  buf[8] = '\0';
  buf[7] = ')';
  buf[6] = hexchar((byte) (x & 15));
  x = (x >> 4);
  buf[5] = hexchar((byte) (x & 15));
  x = (x >> 4);
  buf[4] = hexchar((byte) (x & 15));
  x = (x >> 4);
  buf[3] = hexchar((byte) (x & 15));
  buf[2] = '=';
  buf[1] = prefix;
  buf[0] = '(';
  Os9_puts(buf);
}

asm void pc_trace(int mark, char* ptr) {
	asm {
*                           ; 10: mark; 12:ptr
		pshs y,u    ; 4: orig &; 6: orig frame pointer U.
		leas -4,s   ; 0: char 2: int (for puthex)

		ldd #$0d00   ; CR.
		std 2,s
		leax 2,s
		stx ,s
		lbsr _Os9_puts

		ldd 10,s
		std 0,s
		ldd 12,s
		std 2,s
		lbsr _puthex

		ldd #'{'
		std 0,s
		ldd #$FFF1
		std 2,s
		lbsr _puthex


PcTraceLoop	ldd #'U'
		std ,s
		stU 2,s
		lbsr _puthex

		ldd #'P'
		std ,s
		ldd 2,u
		std 2,s
		lbsr _puthex

		ldU ,u	; previous frame pointer
		tfr U,D
		addd #0
		bne PcTraceLoop

		ldd #'}'
		std 0,s
		ldd #$FFF2
		std 2,s
		lbsr _puthex

		ldd #$0d00   ; CR.
		std 2,s
		leax 2,s
		stx ,s
		lbsr _Os9_puts

		puls D,X,y,u,pc  ; D and X to undo "leas -4,s"
	}
}

// panic: print message and exit 5.
void panic(const char *s) {
  pc_trace('*', 0);
  Os9_puts(s);
  exit(5);
}
