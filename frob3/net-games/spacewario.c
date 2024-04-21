
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;

#define TCP_CHUNK_SIZE 1024  // Chunk to send or recv in TCP.
//#define WIZ_PORT  0xFF68   // Hardware port.
#define WAITER_TCP_PORT  2319   // w# a# i#
#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!
#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600
#define PACKET_BUF 0x600
#define PACKET_MAX 256

#define PrintH(F,...) // nothing

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

// How to talk to the four hardware ports.
struct wiz_port {
  volatile byte command;
  volatile word addr;
  volatile byte data;
};


// Constants for the (four) Wiznet sockets' registers.
struct sock {
    word base;
    word tx_ring;
    word rx_ring;
};
extern const struct sock WizSocketFacts[4];

#define SOCK0_AND (Socks+0),
#define JUST_SOCK0 (Socks+0)

#define SOCK1_AND (Socks+1),
#define JUST_SOCK1 (Socks+1)

#define PARAM_JUST_SOCK   const struct sock* sockp
#define PARAM_SOCK_AND    const struct sock* sockp,
#define JUST_SOCK         sockp
#define SOCK_AND          sockp,

#define B    (sockp->base)
#define T    (sockp->tx_ring)
#define R    (sockp->rx_ring)
#define N    (sockp->nth)

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

#define PEEK1(A) (*(volatile byte*)(A))
#define PEEK2(A) (*(volatile word*)(A))
#define POKE1(A,X) (*(volatile byte*)(A) = (X))
#define POKE2(A,X) (*(volatile word*)(A) = (X))

enum Commands {
  CMD_POKE = 0,
  CMD_HELLO = 1,
  CMD_SUM = 194,
  CMD_PEEK2 = 195,
  CMD_LOG = 200,
  CMD_INKEY = 201,
  CMD_PUTCHAR = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_SP_PC = 205, // deprecate
  CMD_REV = 206, // deprecate
  CMD_RTI = 214,  // experimental
  CMD_JSR = 255,
};

struct proto {
  byte open_mode;
  byte open_status;
  bool is_broadcast;
  byte send_command;
};
extern const struct proto TcpProto;

byte IsThisGomar();
byte WizGet1(word reg);
word WizGet2(word reg);
void WizGetN(word reg, void* buffer, word size);
void WizPut1(word reg, byte value);
void WizPut2(word reg, word value);
void WizPutN(word reg, const void* data, word size);
word WizTicks();
byte WizTocks();
void WizReset();
void WizConfigure();
void WizIssueCommand(PARAM_SOCK_AND byte cmd);
void WizWaitStatus(PARAM_SOCK_AND byte want);

typedef word tx_ptr_t;

void WizOpen(PARAM_SOCK_AND const struct proto* proto, word local_port );
void TcpDial(PARAM_SOCK_AND const byte* host, word port);
void TcpEstablish(PARAM_JUST_SOCK);
errnum WizCheck(PARAM_JUST_SOCK);
errnum WizRecvGetBytesWaiting(PARAM_SOCK_AND word* bytes_waiting_out);
tx_ptr_t WizReserveToSend(PARAM_SOCK_AND  size_t n);
tx_ptr_t WizDataToSend(PARAM_SOCK_AND tx_ptr_t tx_ptr, const char* data, size_t n);
tx_ptr_t WizBytesToSend(PARAM_SOCK_AND tx_ptr_t tx_ptr, const byte* data, size_t n);
void WizFinalizeSend(PARAM_SOCK_AND const struct proto *proto, size_t n);
errnum WizSendChunk(PARAM_SOCK_AND  const struct proto* proto, char* data, size_t n);
errnum WizRecvChunkTry(PARAM_SOCK_AND char* buf, size_t n);
errnum WizRecvChunk(PARAM_SOCK_AND char* buf, size_t n);
errnum WizRecvChunkBytes(PARAM_SOCK_AND byte* buf, size_t n);
errnum TcpRecv(PARAM_SOCK_AND char* p, size_t n);
errnum TcpSend(PARAM_SOCK_AND  char* p, size_t n);
void UdpDial(PARAM_SOCK_AND  const struct proto *proto,
             const byte* dest_ip, word dest_port);
void WizClose(PARAM_JUST_SOCK);

void DoLineBufCommands(bool echo);
void ConfigureTextScreen(word addr, bool orange);
word StackPointer();
char PolCat(); // Return one INKEY char, or 0, with BASIC `POLCAT` subroutine.
void Delay(word n);

const byte HexAlphabet[] = "0123456789abcdef";

// Global state stolen from axiom4.
struct wario_vars {
    volatile struct wiz_port* wiz_port;  // which hardware port?
    word rand_word;
    volatile word bogus;
/*
    byte transaction_id[4];
    bool got_dhcp;
    bool got_lan;
    bool use_dhcp;  // Needs second DHCP round still.
    bool launch;    // Ready to connect to Waiter.

    byte hostname[8];
    byte ip_addr[4];      // dhcp fills
    byte ip_mask[4];      // dhcp fills
    byte ip_gateway[4];   // dhcp fills
    byte ip_resolver[4];  // dhcp fills
    byte ip_dhcp[4];      // dhcp fills.  Needed?
    byte ip_waiter[4];
    byte mask_num;
    word waiter_port;
*/
    word vdg_addr;   // mapped via SAM
    word vdg_begin;  // begin useable portion
    word vdg_end;    // end usable portion
    word vdg_ptr;    // inside usable portion

    void (*lemma_loop)(void);  // Where to re-join the loop.

    char* line_ptr;

    char line_buf[80];
};
#define Vars ((struct wario_vars*)CASBUF)
#define WIZ  (Vars->wiz_port)

struct axiom4_rom_tail { // $DFC0..$DFFF
  byte rom_reserved_16[16];  // $DFC0
  byte rom_waiter[4];   // $DFD0
  byte rom_dns[4];
  byte rom_hailing[8];
  byte rom_hostname[8];  // $DFE0
  byte rom_reserved_3[3];
  byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
  byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
};
#define Rom ((struct axiom4_rom_tail*)0xDFC0)

#define GRAF 0x0600

byte Ships[] = {
#include "spacewar-ships.h"
};

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

char *strcpy(char *restrict dest, const char *restrict src) {
  void* z = dest;
  while (*src) *dest++ = *src++;
  return z;
}

char *strncpy(char *restrict dest, const char *restrict src, size_t n) {
  void* z = dest;
  int i = 0;
  while (*src) {
    *dest++ = *src++;
    i++;
    if (i>=n) break;
  }
  return z;
}

size_t strlen(const char *s) {
  const char* p = s;
  while (*p) p++;
  return p-s;
}

size_t strnlen(const char *s, size_t max) {
  const char* p = s;
  while (*p && (p-s < max)) p++;
  return p-s;
}

////////////////////////////////////////////////////////

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

void Delay(word n) {
  // if (IsThisGomar()) return;  // delay not needed if Gomar.

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

void PutChar(char ch) {

  PrintH("CH: %x %c", (byte)ch, (' ' <= ch && ch <= '~') ? (byte)ch : (byte)'?');

    if (ch == 13 || ch == 10) { // Carriage Return
      do {
        PutChar(' ');
      } while ((Vars->vdg_ptr & 31));
      return;
    }

    word p = Vars->vdg_ptr;
    if (ch == 8) { // Backspace
      if (p > Vars->vdg_begin) {
        *(byte*)p = 32;
        --p;
      }
      goto END;
    }

    if (ch == 1) { // Hilite previous char.
      if (p > Vars->vdg_begin) {
        *(byte*)(p-1) ^= 0x40;  // toggle the inverse bit.
      }
      goto END;
    }

    if (ch < 32) return;  // Ignore other control chars.

    // Only use 64-char ASCII.
    if (96 <= ch && ch <= 126) ch -= 32;
    byte codepoint = (byte)ch;

    *(byte*)p = (0x3f & codepoint);
    p++;

    if (p>=Vars->vdg_end) {
        for (word i = Vars->vdg_begin; i< Vars->vdg_end; i++) {
            if (i < Vars->vdg_end-32) {
                // Copy the line below.
                *(volatile byte*)i = *(volatile byte*)(i+32);
            } else {
                // Clear the last line.
                *(volatile byte*)i = 32;
            }
        }
        p = Vars->vdg_end-32;
    }
END:
    *(byte*)p = 0xEF;  // display Blue Box cursor.
    Vars->vdg_ptr = p;
}

void PutStr(const char* s) {
    while (*s) PutChar(*s++);
}

void PutHex(word x) {
  if (x > 15u) {
    PutHex(x >> 4u);
  }
  PutChar( HexAlphabet[ 15u & x ] );
}
byte DivMod10(word x, word* out_div) { // returns mod
	word div = 0;
	while (x >= 10000) x-=10000, div+=1000;
	while (x >= 1000) x-=1000, div+=100;
	while (x >= 100) x-=100, div+=10;
	while (x >= 10) x-=10, div++;
	*out_div = div;
	return (byte)x;
}
void PutDec(word x) {
  word div;
  if (x > 9u) {
    // eschew div // PutDec(x / 10u);
    DivMod10(x, &div);
    PutDec(div);
  }
  // eschew mod // PutChar('0' + (byte)(x % 10u));
  PutChar('0' + DivMod10(x, &div));
}

void Fatal(const char* wut, word arg) {
    PutStr(" *FATAL* <");
    PutStr(wut);
    PutChar('$');
    PutHex(arg);
    PutStr("> ");
    while (1) continue;
}

void AssertEQ(word a, word b) {
  if (a != b) {
    PutHex(a);
    Fatal(":EQ", b);
  }
}

void AssertLE(word a, word b) {
  if (a > b) {
    PutHex(a);
    Fatal(":LE", b);
  }
}

void PrintF(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const char* s = format;
    while (*s) {
        if (*s == '%') {
            ++s;
            switch (*s) {
                case 'a': {  // "%a" -> IPv4 address as Dotted Quad.
                    byte* x;
                    x = va_arg(ap, byte*);
                    PutDec(x[0]);
                    PutChar('.');
                    PutDec(x[1]);
                    PutChar('.');
                    PutDec(x[2]);
                    PutChar('.');
                    PutDec(x[3]);
                }
                break;
                case 'x': {
                    word x;
                    x = va_arg(ap, word);
                    PutHex(x);
                }
                break;
                case 'u': {
                    word x = va_arg(ap, word);
                    PutDec(x);
                }
                break;
                case 'd': {
                    int x = va_arg(ap, int);
                    if (x<0) {
                      PutChar('-');
                      x = -x;
                    }
                    PutDec((word)x);
                }
                break;
                case 's': {
                    const char* x = va_arg(ap, const char*);
                    PutStr(x);
                }
                break;
                default:
                    PutChar(*s);
            }
        } else {
            PutChar(*s);
        }
        s++;
    }
    va_end(ap);
}

////////////////////////////////////////////////////////

const struct sock Socks[2] = {
    { 0x400, 0x4000, 0x6000 },
    { 0x500, 0x4800, 0x6800 },
    // not needed // { 0x600, 0x5000, 0x7000 },
    // not needed // { 0x700, 0x5800, 0x7800 },
};

const struct proto TcpProto = {
  SK_MR_TCP+SK_MR_ND, // TCP Protocol mode with No Delayed Ack.
  SK_SR_INIT,
  false,
  SK_CR_SEND,
};
const struct proto UdpProto = {
  SK_MR_UDP, // UDP Protocol mode.
  SK_SR_UDP,
  false,
  SK_CR_SEND,
};
const struct proto BroadcastUdpProto = {
  SK_MR_UDP, // UDP Protocol mode.
  SK_SR_UDP,
  true,
  SK_CR_SEND+1,
};
void Pia1bOn(byte x) { *(volatile byte*)0xFF22 |= x; }
void Pia1bOff(byte x) { *(volatile byte*)0xFF22 &= ~x; }
void SetOrangeScreen() { Pia1bOn(0x08); }
void SetGreenScreen() { Pia1bOff(0x08); }
#if 1
void Enable1BitSound() {
    *(volatile byte*)0xFF23 &= ~0x04;  // Clear bit 2 to enable Data Direction access
    *(volatile byte*)0xFF22 |= 0x02;   // Bit 1 and bits 3-7 are outputs.
    *(volatile byte*)0xFF23 |= 0x04;  // Clear bit 2 to enable Data Direction access
}
void Beep(byte n, byte f) {
    Enable1BitSound();

    Pia1bOn(0x02);
    Delay(10);
    Pia1bOff(0x02);
    Delay(10);
}
#endif

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
void WizPutN(word reg, const void* data, word size) {
  volatile struct wiz_port* wiz = WIZ;
  const byte* from = (const byte*) data;
  wiz->addr = reg;
  for (word i=size; i; i--) {
    wiz->data = *from++;
  }
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    return WizGet2(0x0082/*TCNTR Tick Counter*/);
}
byte WizTocks() {
    return WizGet1(0x0082/*TCNTR Tick Counter*/);
}

void WizReset() {
  WIZ->command = 128; // Reset
  Delay(500);
  WIZ->command = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  Delay(100);

  // GLOBAL OPTIONS FOR SOCKETLESS AND ALL SOCKETS:

  // Interval until retry: 1 second.
  WizPut2(RTR0, 10000 /* Tenths of milliseconds. */ );
  // Number of retries: 10 x 1sec = 10sec.
  // Sometimes reset-to-carrier takes over 5 seconds.
  WizPut1(RCR, 10);
}

void WizConfigure() {
  // WizPutN(0x0001/*gateway*/, Vars->ip_gateway, 4);
  // WizPutN(0x0005/*mask*/, Vars->ip_mask, 4);
  // WizPutN(0x000f/*ip_addr*/, Vars->ip_addr, 4);
  // WizPut1(0x0009/*ether_mac+0*/, 0x02);
  // WizPutN(0x000A/*ether_mac+1*/, Rom->rom_mac_tail, 5);
}

errnum ValidateWizPort(struct wiz_port* p) {
    // PrintF("?%x ", p);
    byte status = p->command;
    // PrintF("s:%x ", status);
    if (status != 3) return 11;
    p->addr = 0x0080; // Query Version
    byte version = p->data;
    // PrintF("v:%x ", version);
    if (version != 0x51) return 12;
    p->addr = 0x0009; // Hardware Addr (mac)
    p->data = 0xA1;
    p->data = 0xB2;
    p->data = 0xC3;

    p->addr = 0x0009; // Hardware Addr (mac)
    byte x = p->data;
    byte y = p->data;
    byte z = p->data;
    // PrintF("(%x,%x,%x)\n", x, y, z);
    if (x!=0xA1 || y!=0xB2 || z!=0xC3) return 13;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

void WizIssueCommand(const struct sock* sockp, byte cmd) {
  WizPut1(B+SK_CR, cmd);
  while (WizGet1(B+SK_CR)) {
    // ++ *(char*)0x401;
  }
  // if (cmd == 0x40) PutChar('<');
  // else if (cmd == 0x20) PutChar('>');
  // else PutChar('!');
}

void WizWaitStatus(const struct sock* sockp, byte want) {
  byte status;
  byte stuck = 200;
  do {
    // ++ *(char*)0x400;
    status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("W", status);
  } while (status != want);
}

void WizOpen(const struct sock* sockp, const struct proto* proto, word local_port ) {
  WizPut1(B+SK_MR, proto->open_mode);
  WizPut2(B+SK_PORTR0, local_port); // Set local port.
  WizPut1(B+SK_IR, 0xFF); // Clear all interrupts.
  WizIssueCommand(SOCK_AND SK_CR_OPEN);

  WizWaitStatus(SOCK_AND proto->open_status);
}

void WaitForLink() {
    PrintF("Link: ");
    while (1) {
        byte phys = WizGet1(0x003C); // PHYSR0: physical register 0
        if ((phys & 1) == 1) {
            // link up
            PrintF("UP=%x ", phys);
            return;
        }
        PrintF("down=%x ", phys);
        Delay(10000);
    }
}

int main() {
#if 1
    memset(((char*)Vars), 0, sizeof *Vars);
    Vars->bogus = (word)Ships;

    // Set VDG 32x16 text screen to dots.
    // Preserve the top line.
    Vars->vdg_addr = VDG_RAM;
    Vars->vdg_begin = VDG_RAM+32; // Skip top status line.
    Vars->vdg_ptr = VDG_RAM+32;
    Vars->vdg_end = VDG_END;
#endif

    memset((byte*)VDG_RAM, '+', VDG_END-VDG_RAM);
    while (1) {
    	strcpy((char*)VDG_RAM+20, "SPACEWARIO");
    }
#if 0
	

    if (ValidateWizPort((struct wiz_port*)0xFF68)==OKAY) {
        Vars->wiz_port = (struct wiz_port*)0xFF68;
    } else if (ValidateWizPort((struct wiz_port*)0xFF78)==OKAY) {
        Vars->wiz_port = (struct wiz_port*)0xFF78;
    } else {
        Fatal("nowiz", 9);
    }
    PrintF("Wiznet found at %x\n", Vars->wiz_port);

    Vars->rand_word = WizTicks();
    PrintF("HOSTNAME=\"");
    for (byte i = 0; i<8; i++) {
        char ch = Rom->rom_hostname[i];
        if (' ' <= ch && ch <= '~') PutChar(ch);
    }
    PrintF("\"\n");

    WizReset();
    WaitForLink();
    WizConfigure();
#endif
}
