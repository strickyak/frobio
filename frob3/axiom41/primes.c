////////////////////////////////////////
//   PART--BEGIN-HEADER--
////////////////////////////////////////

// Conventional types and constants for Frobio

#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
typedef void (*func_t)();

#define TCP_CHUNK_SIZE 1024  // Chunk to send or recv in TCP.
#define WIZ_PORT  0xFF68   // Hardware port.
#define WAITER_TCP_PORT  2319   // w# a# i#
#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!
#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600
#define PACKET_BUF 0x600
#define PACKET_MAX 256
const byte waiter_default [4] = {
      134, 122, 16, 44 }; // lemma.yak.net


#include "frob3/wiz/w5100s_defs.h"

#ifdef __GNUC__

#include <stdarg.h>
typedef unsigned int size_t;
#define restrict
void abort(void);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcpy(char *restrict dest, const char *restrict src);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
size_t strlen(const char *s);
#undef restrict

#elif defined(__CMOC__)

#include <cmoc.h>
#include <stdarg.h>

#define volatile /* not supported by cmoc */

#else
void WhichCompilerAreYouUsingQuestionMark();
#endif

////////////////////////////////////////////////////////
///
///  GCC Standard Library Routines.

void* memcpy(void* dest, const void* src, size_t n) {
  char* d = (char*)dest;
  char* s = (char*)src;
  for (size_t i = 0; i < n; i++) *d++ = *s++;
  return dest;
}

void *memset(void *s, int c, size_t n) {
  char* dest = (char*)s;
  for (size_t i = 0; i < n; i++) *dest++ = c;
  return s;
}

////////////////////////////////////////////////////////
const byte HexAlphabet[] = "0123456789abcdef";
////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////

void Pia1bOn(byte x) { *(volatile byte*)0xFF22 |= x; }
void Pia1bOff(byte x) { *(volatile byte*)0xFF22 &= ~x; }
void SetOrangeScreen() { Pia1bOn(0x08); }
void SetGreenScreen() { Pia1bOff(0x08); }

void Delay() {
  volatile byte* p = (byte*) CASBUF;
  for (word i=0; i<1000; i++) {  // was 30.
    *p = i;
  }
}

#define PEEK1(A) (*(volatile byte*)(A))
#define POKE1(A,X) (*(volatile byte*)(A)) = (X)

void PrimesMain() {
    SetOrangeScreen();

    memset((byte*)VDG_RAM+2, 'P', VDG_END-VDG_RAM);  // inverse letter P
    *(byte*)(0x400) = '0';
    *(byte*)(0x401) = '1';

    while (1) {
	    for (word a=2; a<512; a++) {
	    	byte tmp = PEEK1(0x400+a);
	    	POKE1(0x400+a, 128+3*16-1); // blue box
		Delay();
		for (word b=a+a; b<512; b+=a) {
			POKE1(0x400+b, 255-32); // buff box
			Delay();
			POKE1(0x400+b, 63&'C');  // letter C
		}
	    	POKE1(0x400+a, tmp);  // restore what was there
	    }
    }
}

// This main routine is trimmed away by the
// 'sed' script.  It pins down the actual
// AxiomMain so gcc -fwhole-program generates it.
int main() {
	POKE2(0, (word)PrimesMain);
}
