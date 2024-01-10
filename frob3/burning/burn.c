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

#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!
#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

#include "frob3/wiz/w5100s_defs.h"

enum Commands {
  CMD_POKE = 0,
  CMD_JSR = 255,
};

extern const byte HexAlphabet[];

byte WizGet1(word reg);
word WizGet2(word reg);
void WizGetN(word reg, void* buffer, word size);
void WizPut1(word reg, byte value);
void WizPut2(word reg, word value);
void WizPutN(word reg, const void* data, word size);
word WizTicks();
byte WizTocks();

//errnum WizRecvGetBytesWaiting(PARAM_SOCK_AND word* bytes_waiting_out);
//errnum WizRecvChunkTry(PARAM_SOCK_AND char* buf, size_t n);
//errnum WizRecvChunk(PARAM_SOCK_AND char* buf, size_t n);
//errnum WizRecvChunkBytes(PARAM_SOCK_AND byte* buf, size_t n);
//errnum TcpRecv(PARAM_SOCK_AND char* p, size_t n);

/////////////////////////////////////////

#define B 0x500
#define T 0x4800
#define R 0x6800
#define N 1

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
  ShowAtLine(a, 0*32);
  ShowAtLine(b, 1*32);
  ShowAtLine(c, 2*32);
}

bool DoTripleReturnDone() {
  byte hi = Recv();
  byte lo = Recv();
  byte z = Recv();
  word ptr = (hi<<8) | lo;

  if (ptr == 0) return true;

  Show(hi, lo, z);
  *(byte*)ptr = z;
  return false;
}

void ClearScreen() {
  for (word ptr = VDG_RAM; ptr < VDG_END; ptr++) {
    *(byte*)ptr = '-';
  }
}

int main() {
  ClearScreen();
  bool done;
  do { done = DoTripleReturnDone(); } while (!done);
  *(byte*)(VDG_RAM+4*32+0) = 'O' & 0x3F;
  *(byte*)(VDG_RAM+4*32+1) = 'K' & 0x3F;

  while(1) continue;
}
