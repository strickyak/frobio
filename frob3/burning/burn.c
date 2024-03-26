// Pokes according to input from TCP Lemma, slowly.
// Designed to be used for coco1/coco2 to burn CocoIOr EEPROM.
// But burn-splash is a demo that just pokes to the VDG screen, instead.
// This uses a "triples" byte format, not the Quint format.
// Triples are ( hi-addr  lo-addr  data-byte ).

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

#define WIZ_PORT  0xFF68   // Hardware port.
struct wiz_port {
  volatile byte command;
  volatile word addr;
  volatile byte data;
};
#define WIZ  ((struct wiz_port*)WIZ_PORT)

#define CASBUF 0x01DA    // Rock the CASBUF, rock the CASBUF!
#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

#include "frob3/wiz/w5100s_defs.h"

/////////////////////////////////////////

#define ROM ((byte*)0xC000)

void Delay() {
  volatile byte* p = (byte*) CASBUF;
  for (word i=0; i<3000; i++) {  // was 30.
    *p = i;
  }
}

// Software Data Protection:
// See page 10 (sections 19, 20) of
// https://ww1.microchip.com/downloads/en/DeviceDoc/doc0270.pdf
// for these magic numbers.
void EnableProtection() {
  Delay();
  Delay();
  volatile byte* p;
  p = ROM + 0x1555, *p = 0xAA;
  p = ROM + 0x0AAA, *p = 0x55;
  p = ROM + 0x1555, *p = 0xA0;
  Delay();
  Delay();
}
void DisableProtection() {
  Delay();
  Delay();
  volatile byte* p;
  p = ROM + 0x1555, *p = 0xAA;
  p = ROM + 0x0AAA, *p = 0x55;
  p = ROM + 0x1555, *p = 0x80;
  p = ROM + 0x1555, *p = 0xAA;
  p = ROM + 0x0AAA, *p = 0x55;
  p = ROM + 0x1555, *p = 0x20;
  Delay();
  Delay();
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

void ShowAtLine(byte a, byte offset) {
  byte* p = (byte*)((word)VDG_RAM + (word)offset);
  for (byte i = 0; i < 8; i++) {
    *p++ = '0' + ((a&0x80U)!=0);
    a <<= 1;
  }
}

void Show(byte a, byte b, byte c) {
  ShowAtLine(a, 0*32+2);
  ShowAtLine(b, 1*32+2);
  ShowAtLine(c, 2*32+2);
}

bool DoTripleReturnDone() {
  byte hi = Recv();
  byte lo = Recv();
  byte z = Recv();
  word ptr = (hi<<8) | lo;

  if (ptr == 0) return true;

  Show(hi, lo, z);
  *(byte*)ptr = z;
  Delay();
  return false;
}

void ClearScreen() {
  for (word ptr = VDG_RAM; ptr < VDG_END; ptr++) {
    *(byte*)ptr = '-';
  }
  *(byte*)(VDG_RAM) = 'H';
  *(byte*)(VDG_RAM+32) = 'L';
  *(byte*)(VDG_RAM+64) = 'D';
}

int main() {
  ClearScreen();
  DisableProtection();
  DisableProtection();
  DisableProtection();

  bool done;
  do { done = DoTripleReturnDone(); } while (!done);
  *(byte*)(VDG_RAM+4*32+0) = 'O' & 0x3F;
  *(byte*)(VDG_RAM+4*32+1) = 'K' & 0x3F;

  EnableProtection();
  EnableProtection();
  EnableProtection();
  while(1) continue;
}
