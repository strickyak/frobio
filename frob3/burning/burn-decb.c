// Burn the CocoIOr EEPROM, given a DECB binary streamed from lemma.
// The DECB binary should load in the range $C000-$DFFF.
// The final 255 quintuple (the entry address) is ignored.

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

#define CASBUF 0x01DA    // Rock the CASBUF, rock the CASBUF!

struct wiz_port {
  volatile byte command;
  volatile word addr;
  volatile byte data;
};
struct vars {
  struct wiz_port* wiz;
};
// CASBUF[0:63] is used to buffer a chunk.
// So we start the vars at CABUF+80.
#define VARS ((struct vars*)(CASBUF+80))
#define WIZ  VARS->wiz

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

#define PEEK1(A)    (*(volatile byte*)(A))
#define POKE1(A,X)  (*(volatile byte*)(A)) = (X)

#include "frob3/wiz/w5100s_defs.h"

/////////////////////////////////////////

#define ROM ((byte*)0xC000)

char PolCat() {
  char inkey = 0;

#ifdef __GNUC__
    asm volatile (R"(
      jsr [$A000]
      sta %0
    )" : "=m" (inkey) );
#else
    asm {
      jsr [$A000]
      sta :inkey
    }
#endif

  return inkey;
}

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

void Delay(word n) {
  if (IsThisGomar()) return;  // delay not needed if Gomar.

  while (n--) {
#ifdef __GNUC__
    asm volatile ("mul" : : : "d");
    asm volatile ("mul" : : : "d");
    asm volatile ("mul" : : : "d");
    asm volatile ("mul" : : : "d");
    asm volatile ("mul" : : : "d");
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

void Print(word screen, const char* s) {
  for (; *s; s++) {
  	byte ch = 63 & *s;
	POKE1(screen, ch);
	screen++;
  }
}
void Fatal(const char* s) {
  	Print(0x5C0, "***  FATAL:  ***" );
  	Print(0x5E0, s);
	while (1) {}
}

// Software Data Protection:
// See page 10 (sections 19, 20) of
// https://ww1.microchip.com/downloads/en/DeviceDoc/doc0270.pdf
// for these magic numbers.
void EnableProtection() {
  Delay(500);
  POKE1( ROM + 0x1555, 0xAA);
  POKE1( ROM + 0x0AAA, 0x55);
  POKE1( ROM + 0x1555, 0xA0);
  Delay(500);
}
void DisableProtection() {
  Delay(500);
  POKE1( ROM + 0x1555, 0xAA);
  POKE1( ROM + 0x0AAA, 0x55);
  POKE1( ROM + 0x1555, 0x80);
  POKE1( ROM + 0x1555, 0xAA);
  POKE1( ROM + 0x0AAA, 0x55);
  POKE1( ROM + 0x1555, 0x20);
  Delay(500);
}

/////////////////////////////////////////

// Register values for Socket #1 (used by Lemma)
#define B 0x500   // per-socket regs Base
#define T 0x4800  // Tx buffer
#define R 0x6800  // Rx buffer

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
void WizPut1(word reg, byte value) {
  WIZ->addr = reg;
  WIZ->data = value;
}
void WizPut2(word reg, word value) {
  WIZ->addr = reg;
  WIZ->data = (byte)(value >> 8);
  WIZ->data = (byte)(value);
}

void WizIssueCommand(byte cmd) {
  WizPut1(B+SK_CR, cmd);
  while (WizGet1(B+SK_CR)) continue;
}

word GetBytesWaiting() {
  return WizGet2(B+SK_RX_RSR0);  // Unread received size.
}

byte Recv() {
  word bytes_waiting;
  do {
    bytes_waiting = GetBytesWaiting();
  } while (bytes_waiting == 0);

  word rd = WizGet2(B+SK_RX_RD0);
  word offset = rd & RING_MASK; // offset: Beneath RING_SIZE.

  byte z = WizGet1(R+offset);
  WizPut2(B+SK_RX_RD0, rd + 1);
  WizIssueCommand(SK_CR_RECV); // Inform socket of changed SK_RX_RD.
  return z;
}

const char Hex[] = "0123456789\001\002\003\004\005\006";

void PrintHex(word loc, word val) {
	byte a = 15 & (val>>12);
	byte b = 15 & (val>>8);
	byte c = 15 & (val>>4);
	byte d = 15 & (val>>0);
	POKE1(loc+0, ' ');
	POKE1(loc+1, '(');
	POKE1(loc+2, Hex[a]);
	POKE1(loc+3, Hex[b]);
	POKE1(loc+4, Hex[c]);
	POKE1(loc+5, Hex[d]);
	POKE1(loc+6, ')');
	POKE1(loc+7, ' ');
}

void ClearScreen(byte ch) {
  for (word ptr = VDG_RAM; ptr < VDG_END; ptr++) {
    POKE1(ptr, ch);
  }
}
void Burn(word n, word p) {
	while (n) {
		PrintHex(0x520, n);
		PrintHex(0x528, p);
		// Starting at p, what is the most we can burn in a 64 byte chunk?
		byte size = 64 - (63 & (byte)p);
		// But we cannot burn more than n.
		if (n < size) size = n;
		// Recv the next n bytes and burn them.
		// Get them all first, into the CASBUF.
		for (byte i = 0; i < size; i++) {
			POKE1(CASBUF + i, Recv());
		}
		// Now copy them quickly so they burn correctly.
		for (byte i = 0; i < size; i++) {
			POKE1(p + i, PEEK1(CASBUF + i));
		}
		n -= size;
		p += size;
		Delay(500);
	}
	PrintHex(0x520, n);
	PrintHex(0x528, p);
}

void DetectWiz() {
	if (PEEK1(0xFF68) == 3) {
		WIZ = (struct wiz_port*) 0xFF68;
		Print(0x0400, "WIZNET DETECTED AT $FF68.");
	} else if (PEEK1(0xFF78) == 3) {
		WIZ = (struct wiz_port*) 0xFF78;
		Print(0x0400, "WIZNET DETECTED AT $FF78.");
	} else {
  		Fatal("CANNOT DETECT WIZNET. ");
	}
}

int main() {
  ClearScreen(':');
  DetectWiz();
  Print(0x480, "HIT X WHEN READY TO BURN.");
  while (1) {
  	byte b = PolCat();
	if (b == 'X' || b == 'x') break;
  }
  Print(0x480, "                         ");
  Print(0x4A0, "    BURNING...           ");

  DisableProtection();
  DisableProtection();
  DisableProtection();

  while (true) {
  	byte cmd = Recv();
	byte hi = Recv();
	byte lo = Recv();
	word n = ((word)hi << 8) | lo;
	hi = Recv();
	lo = Recv();
	word p = ((word)hi << 8) | lo;

	if (cmd == 255) {
		break;
	} else if (cmd == 0) {
		Burn(n, p);
	} else {
		Fatal("DATA ERROR FROM SERVER");
	}
  }

  EnableProtection();
  EnableProtection();
  EnableProtection();
  Print(0x4A0, "                         ");
  Print(0x560, "  DONE.  ");
  while(1) continue;
}
