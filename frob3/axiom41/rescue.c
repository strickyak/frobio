/* 
	>>> 0x246 - 0x224
	34         ( bytes of SETDP $FF advantage )
*/

////////////////////////////////////////
//   PART--BEGIN-HEADER--
////////////////////////////////////////

#define UDP_PORT 12323

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

#define _ {PrintF("L%d.", __LINE__);}

struct UdpRecvHeader {
    byte addr[4];
    word port;
    word len;
};

// How to talk to the four hardware ports.
struct wiz_port {
  volatile byte command;
  volatile word addr;
  volatile byte data;
};

#define WIZ  ((struct wiz_port*)WIZ_PORT)

#define B    (0x0500)
#define T    (0x4800)
#define R    (0x6800)

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

#define PEEK1(A) (*(volatile byte*)(A))
#define PEEK2(A) (*(volatile word*)(A))
#define POKE1(A,X) (*(volatile byte*)(A) = (X))
#define POKE2(A,X) (*(volatile word*)(A) = (X))

#define C_EMPTY '.'
#define C_WANT (63&'O')
#define C_GOT 'X'

// Returns 0 if not Gomar, 'G' if is Gomar.
byte IsThisGomar() {
#ifdef __CMOC__
  byte result;
  asm {
     CLRA    ; optional, for 16-bit return value in D.
     CLRB
     NOP     ; begin hyper sequence...
     FCB $21 ; brn...
     FCB $FF ;    results in "LDB #$47" if on Gomar.
     STB :result
  }
  return result;
#else  // for GCC
  word result;
  asm volatile("CLRA\n  CLRB\n  NOP\n  FCB $21\n  FCB $ff\n std %0"
      : "=m" (result) // the output
      : // no inputs
      : "d" // Clobbers D register.
  );
  return (byte)result;
#endif
}


void RDelay(word n) {
  if (IsThisGomar()) return;  // delay not needed if Gomar.

  while (n--) {
#ifdef __GNUC__
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
#else
    asm {
      mul
      mul
      mul
      mul
      mul
    }
#endif
  }
}

void WizReset() {
  WIZ->command = 128; // Reset
  RDelay(500);
  WIZ->command = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  RDelay(100);
}

byte WizGet1(word reg) {
  WIZ->addr = reg;
  return WIZ->data;
}
word WizGet2(word reg) {
  WIZ->addr = reg;
  byte z_hi = WIZ->data;
  byte z_lo = WIZ->data;
  return ((word)(z_hi) << 8) + (word)z_lo;
}

void WizGetN(word reg, void* buffer, word size) {
  volatile struct wiz_port* wiz = WIZ;
  byte* to = (byte*) buffer;
  wiz->addr = reg;
   for (word i=size; i; i--) {
    *to++ = wiz->data;
  }
}
void WizPut1(word reg, byte value) {
  WIZ->addr = reg;
  WIZ->data = value;
}
void WizPut2(word reg, word value) {
  WIZ->addr = reg;
  WIZ->data = (byte)(value >> 8);
  WIZ->data = (byte)(value);
}

void Command(byte cmd) {
	WizPut1(B + 1, 1);
	while (WizGet1(B + 1)) {}
}

#define PAYLOAD_SIZE 128
#define PACKET_SIZE (8 + PAYLOAD_SIZE)

// #define SPIN(ADDR) POKE1((ADDR), 1 + PEEK1((ADDR)))
#define SPIN(ADDR) do {} while (0) /* dont spin */

void ReceiveSpan(word offset, word rambuf, word size) {
	WizGetN(R+offset, (void*)rambuf, size);
}

bool ReceiveIfPacketAvailable() {
	word sz = WizGet2(B + SK_RX_RSR0);
	if (sz < PACKET_SIZE) {
		SPIN(0x05FF);
		return false;
        }

	SPIN(0x05FE);
	const word loc = WizGet2(B+SK_RX_RD0);
	const word offset = RING_MASK & loc;
	const word rambuf = 0x0400;
	if (offset + PACKET_SIZE <= RING_SIZE) {
		SPIN(0x05FD);
		ReceiveSpan(offset, rambuf, PACKET_SIZE);
	} else {
		SPIN(0x05FC);
		const word first_size = RING_SIZE - offset;
		const word second_size = PACKET_SIZE - first_size;
		ReceiveSpan(offset, rambuf, first_size);
		ReceiveSpan(offset, rambuf+first_size, second_size);
	}

	WizPut2(B+SK_RX_RD0, loc+PACKET_SIZE);
	Command(0x40 /* Receive */ );
	return true;
}

struct tiny_burn {
	byte ith_record;
	byte num_records;
	byte payload_len;
	word payload_addr;
};

void MagicWrites(byte b) {
	// See https://ww1.microchip.com/downloads/en/DeviceDoc/doc0270.pdf
	POKE1(0xC000 + 0x1555, 0xAA);
	POKE1(0xC000 + 0x0AAA, 0x55);
	POKE1(0xC000 + 0x1555, b);
}

void AllowWrites() {
	RDelay(10000);
	MagicWrites(0x80);
	MagicWrites(0x20); // then wait t_WC = 10 ms
	RDelay(10000);
}
void ForbidWrites() {
	RDelay(10000);
	MagicWrites(0xA0); // then wait t_WC = 10 ms
	RDelay(10000);
}

bool Test123() {
	const word PA = 0xFF00;
	const word PB = 0xFF02;
	// Digit 0 on PB0; 1 on PB1; 2 on PB2; etc.
	// Digits 0-7 are sensed on PA4, value 0x10.
	POKE1(PB, 0xFD); // for '1'
	if (PEEK1(PA) & 0x10) return false;
	POKE1(PB, 0xFB); // for '2'
	if (PEEK1(PA) & 0x10) return false;
	POKE1(PB, 0xF7); // for '3'
	if (PEEK1(PA) & 0x10) return false;
	return true;
}

bool KeyDown() {
	const word PA = 0xFF00;
	const word PB = 0xFF02;
	POKE1(PB, 0xFE); // Key 'X' is at PB0, PA3.
	return !(PEEK1(PA) & 0x08);
}

void OpenUdpListener() {
	WizPut1(B, 0x82);  // multicast UDP mode

	WizPut2(B+4, UDP_PORT);  // source port
	WizPut2(B+0x10, UDP_PORT);  // dest port

	WizPut2(B+0x0C, 0xFFFF);  // peer broadcast IP 255.255
	WizPut2(B+0x0E, 0xFFFF);  // peer broadcast IP 255.255

	Command( 1 ); // Open Command
}

void AttemptRescue123() {
  // Rescue burning is only done if keys 1, 2, and 3 are held down.
  // If so, we do the burning, and never return.
  // Otherwise, we return immediately.
  if (!Test123()) return;

  // POKE1(0xFF90, ???);

  const word screen_start = 0x0500;
  for (word i=0; i<512; i++) {
	// POKE1(screen_start+i, C_EMPTY);
	POKE1(screen_start+i, ':');
  }

  WizReset();
  OpenUdpListener();

  const word start_of_packet_udp_header = 0x400;
  const word start_of_payload = 0x408;
  const word start_of_burn_data = 0x408 + 64;
  const struct tiny_burn* tiny = -1 + (struct tiny_burn*)(start_of_burn_data);
  const word bar_start = 0x0500;
  while (!KeyDown()) {
    SPIN(0x05E2);
    if (ReceiveIfPacketAvailable()) {
    	for (word i=0; i<512-32; i++) {
		POKE1(bar_start+i, (i < tiny->num_records) ? C_WANT : C_EMPTY);
	}
    }
  }
  AllowWrites();
  while (KeyDown()) {
    SPIN(0x05E4);
    if (ReceiveIfPacketAvailable()) {
    	byte ith = tiny->ith_record;
	if (PEEK1(bar_start + ith) == C_WANT) {
		// we want it
		for (byte i=0; i<tiny->payload_len; i++) {
			POKE1(tiny->payload_addr + i, PEEK1(start_of_burn_data+i));
		}
		POKE1(bar_start + ith, C_GOT);
	}
    }
  }
  ForbidWrites();
  POKE1(0x05E0, C_GOT);
  while (1) {
    SPIN(0x05E6);
  }
}

// This main routine is trimmed away by the
// 'sed' script.  It pins down the actual
// AttemptRescue123 so gcc -fwhole-program generates it.
int main() {
	POKE2(0, (word) AttemptRescue123);
}
