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

#define _ {PrintF("L%d.", __LINE__);}

// XXX #define SPIN(N) {(*(byte*)(VDG_RAM+(N))) = 0x80 | (1 + *(byte*)(VDG_RAM+(N)));}
void SPIN(byte n) {
    byte b = 1 + *(byte*)(VDG_RAM+(n));
    if ((b&15) == 0) b++;  // Skip blank patterns.
    (*(byte*)(VDG_RAM+(n))) = b | 0x80;
}

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

// Global state for axiom4.
struct axiom4_vars {
    // THE FIELD orig_s_reg MUST BE FIRST (due to how we zero the vars).
    word orig_s_reg;  // Remember original stack pointer.
    word main;        // where is axiom, rom or ram?
    word rom_sum_0;
    word rom_sum_1;
    word rom_sum_2;
    word rom_sum_3;

    volatile struct wiz_port* wiz_port;  // which hardware port?
    word rand_word;
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

    word vdg_addr;   // mapped via SAM
    word vdg_begin;  // begin useable portion
    word vdg_end;    // end usable portion
    word vdg_ptr;    // inside usable portion

    char* line_ptr;

    char line_buf[80];
};
#define Vars ((struct axiom4_vars*)CASBUF)
#define WIZ  (Vars->wiz_port)

struct axiom4_rom_tail { // $DFE0..$DFFF
  byte rom_hostname[8];
  byte rom_reserved[3];
  byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
  byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
};
#define Rom ((struct axiom4_rom_tail*)0xDFE0)

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

enum Commands {
  CMD_POKE = 0,
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
extern const struct proto UdpProto;
extern const struct proto BroadcastUdpProto;
#if 0
extern const char ClassAMask[4];
extern const char ClassBMask[4];
extern const char ClassCMask[4];
#endif

struct wiz_udp_recv_header {
    byte ip_addr[4];
    word udp_port;
    word udp_payload_len;
};

extern const byte HexAlphabet[];

//// Function Declarations.

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

void DoLineBufCommands();
void ConfigureTextScreen(word addr, bool orange);
word StackPointer();
char PolCat(); // Return one INKEY char, or 0, with BASIC `POLCAT` subroutine.
void Delay(word n);

void PutChar(char ch);
void PutStr(const char* s);
void PutHex(word x);
void PutDec(word x);
void Fatal(const char* wut, word arg);
void ShowLine(word line);
void PrintF(const char* format, ...);
void AssertEQ(word a, word b);
void AssertLE(word a, word b);

////////////////////////////////////////
//   PART--END-HEADER--
////////////////////////////////////////

////////////////////////////////////////
//   PART--END-HEADER--
////////////////////////////////////////

////////////////////////////////////////
//   PART-NETLIB3
////////////////////////////////////////

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
const byte HexAlphabet[] = "0123456789abcdef";

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
const char SixFFs[6] = {(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF};
const char Eight00s[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const char TwoTwos[2] = {2, 2};

////////////////////////////////////////////////////////

void PrintH(const char* format, ...) {
  const char ** fmt_ptr = &format;
#ifdef __GNUC__
  asm volatile (
      "ldx %0\n  nop\n  fcb 33\n  fcb 111"
      :                // No output
      : "m" (fmt_ptr)  // Input pointer to stack
      : "x"            // clobbers X
  );
#else
  asm {
      ldx fmt_ptr
      nop      // Step 1
      fcb 33   // Step 2: BRN...
      fcb 111  // PrintH2 in emu/hyper.go
  }
#endif
}


////////////////////////////////////////////////////////

word CpuType() {
  word result;
#ifdef __GNUC__
  asm ("  clra \n"
       "  clrb \n"
       "  ldx #$9821 \n"
       "  tfr x,w \n"
       "  tfr w,d \n"
       "  std %0 \n"
       : "=m" (result) // output
       :     // no input
       : "x", "d"  // clobbers (actually, "w" too)
       );
#else
    asm {
      clra
      clrb
      ldx #$9821
      tfr x,w
      tfr w,d
      std result
    }
#endif
    return result;
}

word StackPointer() {
  word result;
#ifdef __GNUC__
    asm ("tfr s,%0" : "=r" (result));
#else
    asm {
      tfr s,x
      stx result
    }
#endif
    return result;
}

void NativeMode() {
  asm volatile("ldmd #1");
}

char PolCat() {
  char inkey = 0;
  // Gomar does not emulate keypresses for PolCat (yet).
  if (!IsThisGomar()) {

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

  }
  return inkey;
}

void Delay(word n) {
  if (IsThisGomar()) return;
#if !__GOMAR__
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
#endif
}

void PutChar(char ch) {
#if __GOMAR__
  PrintH("CH: %x %c\n", ch, (' ' <= ch && ch <= '~') ? ch : '?');
  return;
#endif
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
void PutDec(word x) {
  if (x > 9u) {
    PutDec(x / 10u);
  }
  PutChar('0' + (byte)(x % 10u));
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

///////////////////////////////////////////////////////////

void Pia1bOn(byte x) { *(volatile byte*)0xFF22 |= x; }
void Pia1bOff(byte x) { *(volatile byte*)0xFF22 &= ~x; }
void Orange() { Pia1bOn(0x08); }
void Green() { Pia1bOff(0x08); }
#if 1
void Enable1BitSound() {
    *(volatile byte*)0xFF23 &= ~0x04;  // Clear bit 2 to enable Data Direction access
    *(volatile byte*)0xFF22 |= 0x02;   // Bit 1 and bits 3-7 are outputs.
    *(volatile byte*)0xFF23 |= 0x04;  // Clear bit 2 to enable Data Direction access
}
void Beep(byte n, byte f) {
    Enable1BitSound();
    for (byte i = 0; i < n; i++) {
        SPIN(10);
        Pia1bOn(0x02);
        Delay(f<<2);
        Pia1bOff(0x02);
        Delay(f<<2);
        Pia1bOn(0x02);
        Delay(f);
        Pia1bOff(0x02);
        Delay(f);
    }
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
  WizPutN(0x0001/*gateway*/, Vars->ip_gateway, 4);
  WizPutN(0x0005/*mask*/, Vars->ip_mask, 4);
  WizPutN(0x000f/*ip_addr*/, Vars->ip_addr, 4);
  WizPut1(0x0009/*ether_mac+0*/, 0x02);
  WizPutN(0x000A/*ether_mac+1*/, Rom->rom_mac_tail, 5);
}

errnum ValidateWizPort(struct wiz_port* p) {
    PrintF("?%x ", p);
    byte status = p->command;
    PrintF("s:%x ", status);
    if (status != 3) return 11;
    p->addr = 0x0080; // Query Version
    byte version = p->data;
    PrintF("v:%x ", version);
    if (version != 0x51) return 12;
    p->addr = 0x0009; // Hardware Addr (mac)
    p->data = 0xA1;
    p->data = 0xB2;
    p->data = 0xC3;

    p->addr = 0x0009; // Hardware Addr (mac)
    byte x = p->data;
    byte y = p->data;
    byte z = p->data;
    PrintF("(%x,%x,%x)\n", x, y, z);
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

void TcpDial(const struct sock* sockp, const byte* host, word port) {
  WizPut2(B+SK_TX_WR0, T); // does this help

  WizPutN(B+SK_DIPR0, host, 4);
  WizPut2(B+SK_DPORTR0, port);
  WizPut1(B+SK_IR, 0xFF); // Clear Interrupt bits.
  WizIssueCommand(SOCK_AND SK_CR_CONN);
}

// For Server or Client to accept/establish connection.
void TcpEstablish(PARAM_JUST_SOCK) {
  byte stuck = 250;
  while(1) {
    Delay(1000);
    PutChar('+');
    // Or we could wait for the connected interrupt bit,
    // and not the disconnected nor the timeout bit.
    byte status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("TEZ", status);

    if (status == SK_SR_ESTB) break;
    if (status == SK_SR_INIT) continue;
    if (status == SK_SR_SYNS) continue;

    Fatal("TE", status);

  };

  WizPut1(B+SK_IR, SK_IR_CON); // Clear the Connection bit.

  // XXX Experiment: always set the PSH bit.
  WizPut1(B+/*0x2F*/SK_MR2, 0x02);  // PSH all TCP.
}

////////////////////////////////////////
//   PART-LAN-WAITER
////////////////////////////////////////

#define LAN_CLIENT_PORT 12113  // L=12 A=1 M=3
#define LAN_SERVER_PORT 12114  // L=12 A=1 N=4

// Bit definitions for lan_reserved[0]
#define LAN_RES0_H6309 0x01  // if H6309 CPU
#define LAN_RES0_GOMAR 0x80  // if Gomar Emulator

struct lan_discovery_request {
  byte lan_opcode[4];  // "Q\0\0\0"
  byte lan_xid[4];
  byte lan_reserved[8];

  word orig_s_reg;     // how big is memory?
  word main;           // where is axiom, rom or ram?
  word rom_sum_0;      // what roms are visible?
  word rom_sum_1;
  word rom_sum_2;
  word rom_sum_3;
  byte mac_tail[5];    // 5 bytes from Rom.
};

struct lan_discovery_reply {
  byte lan_opcode[4];  // "R\0\0\0"
  byte lan_xid[4];
  byte lan_reserved[8];

  byte axiom_commands[80];  // ASCII commands to execute.
};

void SendLanRequest(const struct sock* sockp) {
  // second is False for Discover, True for request.
  memset((char*)PACKET_BUF, 0, PACKET_MAX);
  struct lan_discovery_request *q = (struct lan_discovery_request*) PACKET_BUF;

  q->lan_opcode[0] = 'Q';
  memcpy(&q->lan_xid, &Vars->transaction_id, 4);
  memcpy(&q->orig_s_reg, &Vars->orig_s_reg, 6*2); // six words.
  memcpy(&q->mac_tail, &Rom->rom_mac_tail, 5); // five bytes.

  if (CpuType() == 0x9821) q->lan_reserved[0] |= LAN_RES0_H6309;
  if (IsThisGomar()) q->lan_reserved[0] |= LAN_RES0_GOMAR;

  tx_ptr_t tx_ptr = WizReserveToSend(SOCK_AND sizeof *q);
  tx_ptr = WizBytesToSend(SOCK_AND tx_ptr, (byte*)q, sizeof *q);
  WizFinalizeSend(SOCK_AND &BroadcastUdpProto, sizeof *q);
}

errnum RecvLanReply(const struct sock* sockp, word plen) {
  // TODO // plen = (plen > PACKET_MAX) ? PACKET_MAX : plen;
  errnum e = WizRecvChunk(SOCK_AND  (char*)PACKET_BUF, plen);
  if (e) return e;

  struct lan_discovery_reply* r = (struct lan_discovery_reply*)PACKET_BUF;
  memcpy(Vars->line_buf, r->axiom_commands, 80);

  // TODO -- toss rest of big packet.
  return 0;
}


////////////////////////////////////////
//   PART-DHCP
////////////////////////////////////////

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define BAD_SHORT_PACKET       50
#define BAD_DHCP_OPTION_MAGIC  51
#define BAD_OFFER_LEN          52
#define BAD_BOOTP_REPLY        53

struct dhcp {
    byte opcode; // 1=request 2=response
    byte htype;  // 1=ethernet
    byte hlen;   // == 6 bytes
    byte hops;   // 1 or 0, on a LAN
//4
    byte xid[4];  // transaction id
//8
    word secs;
    word flags;  // $80 broadcast, $00 unicast.
//12
    byte ciaddr[4];      // Client IP addr
    byte yiaddr[4];      // Your IP addr
//20
    byte siaddr[4];      // Server IP addr
    byte giaddr[4];      // Gateway IP addr
//28
    byte chaddr[16];  // client hardware addr
//44
    byte sname[64];   // server name
//108
    byte bname[128];  // boot file name
//236
    // THEN byte options[...]
};


#define Z 0 // to be filled in at runtime
const byte DhcpTemplateOne[] = {
  1, // opcode: 1=request 2-response
  1, // htype: 1=ethernet
  6, // hlen: == 6 byte MAC.
  0, // hops: 1 or 0, on a LAN.

  Z, Z, Z, Z, // xid

  0, 0, // secs
  0x80, 0x00, // broadcast
};
const byte DhcpTemplateTwo[] = {
  /*0*/ 99, 130, 83, 99, // magic cookie for DHCP
  // Hereafter, a one-byte OptionType, a one-byte length, and that many payload bytes.
  /*4*/ 53, 1, 3,  // 53=OptionType 1=len 1=Discover|3=Request (overwrite for request!)
  /*7*/ 12, 8, // 12=Hostname, 8=len
  /*9*/ Z, Z, Z, Z, Z, Z, Z, Z,  // copy hostname here.
  // Next three lines ONLY for 3=Request:
  /*17*/ 50, 4, // plus 4-byte address.
  /*19*/ Z, Z, Z, Z, // address
  /*23*/ 255, 0   // 255=end, 0=len // ends the options.
};
#undef Z

void SendDhcpRequest(const struct sock* sockp, bool second) {
  struct dhcp* p = (struct dhcp*)PACKET_BUF;
  word plen = sizeof *p + (second ? 25 : 19);
  tx_ptr_t tx_ptr = WizReserveToSend(SOCK_AND plen);

  // second is False for Discover, True for request.
  memset(p, 0, PACKET_MAX);
  memcpy(p, DhcpTemplateOne, sizeof DhcpTemplateOne);

  memcpy(p->xid, Vars->transaction_id, 4);
  p->chaddr[0] = 2;
  memcpy(p->chaddr+1, Rom->rom_mac_tail, 5);
  if (second) memcpy(p->yiaddr, Vars->ip_addr, 4);
  tx_ptr = WizBytesToSend(SOCK_AND tx_ptr, (byte*)p, sizeof *p);

  byte* opt = (byte*)PACKET_BUF;
  memset((char*)PACKET_BUF, 0, PACKET_MAX);
  memcpy(opt, DhcpTemplateTwo, sizeof DhcpTemplateTwo);

  memcpy(opt + 9, Rom->rom_hostname, 8);
  if (second) {
    memcpy((char*)opt + 19, Vars->ip_addr, 4);
  } else {
    *(byte*)(opt + 6) = 1 /*Discover opcode*/;
    *(word*)(opt + 17) = 0xFF00;  // opt 255=end, len 0
  }
  tx_ptr = WizBytesToSend(SOCK_AND tx_ptr, (byte*)PACKET_BUF, plen - sizeof *p);
  WizFinalizeSend(SOCK_AND &BroadcastUdpProto, plen);
}

errnum RecvDhcpReply(const struct sock* sockp, word plen, bool second) {
  struct dhcp* p = (struct dhcp*)PACKET_BUF;
  //Delay(3000);
//_
  errnum e = WizRecvChunk(SOCK_AND  (char*)PACKET_BUF, sizeof *p);
  if (e) return e;
  memcpy(Vars->ip_addr, p->yiaddr, 4);

//_
  //Delay(3000);
  e = WizRecvChunk(SOCK_AND  (char*)PACKET_BUF, plen - sizeof *p);
  if (e) return e;
  byte* opt = (byte*)PACKET_BUF;
  if (opt[1] != 130) return 9;  // just check one of the 4 bytes: 99, 130, 83, 99.
//_
  byte* end = (byte*)PACKET_BUF + plen - sizeof *p;
  opt += 4;

//_
  while (opt < end) {
    PutChar('#');
    //PrintF("(%d:%d)", opt[0], opt[1]);
    switch (opt[0]/*option number*/) {
      case 1: memcpy(Vars->ip_mask, opt+2, 4);
      break;
      case 3: memcpy(Vars->ip_gateway, opt+2, 4);
      break;
      case 6: memcpy(Vars->ip_resolver, opt+2, 4);
      break;
      case 53:
        if (opt[2] != (second ? 5/*DHCP ACK*/ : 2/*DHCP OFFER*/)) {
          PrintF(">%x< ", opt[2]);
          return 1; // try again
        }
      break;
      case 255: return 0;
    }
    opt += 2 + opt[1]/*opt payload len*/;
  }
_
  return 5;
}


////////////////////////////////////////
//   PART-COMMANDS
////////////////////////////////////////

#define BUF (Vars->line_buf)
#define PTR (Vars->line_ptr)

void GetUpperCaseLine(char initialChar) {
  memset(BUF, 0, sizeof BUF);
  char* p = BUF;  // Rewind.

  if (33 <= initialChar && initialChar <= 126) {
    *p++ = initialChar;
     PutChar(initialChar);
  }

  while (true) {
    char ch = PolCat();
    if (!ch) continue;
    if (ch==13) break; // CR

    if (ch == 8) { // backspace
      if (p > BUF) {
        --p;
        *p = '\0';
        PutChar(8);
      }
      continue;
    }

    if (ch < 32) continue;  // no control
    if (ch > 126) continue; // only printable ASCII

    if ('a' <= ch && ch <= 'z') ch -= 32; // to UPPER case

    if (p < BUF + sizeof BUF - 2) {
      *p = ch;
      p++;
      PutChar(ch);
    }
  }
  PutChar(13);
}

void SkipWhite() {
  while (*PTR == ' ' || *PTR == '.' || *PTR == '/' || *PTR == ':') ++PTR;
}

bool GetNum2Bytes(word* num_out) {
  SkipWhite();
  bool gotnum = false;
  word x = 0;
  if (*PTR == '$') {
    ++PTR;
    while (('0' <= *PTR && *PTR <= '9') ||
           ('A' <= *PTR && *PTR <= 'F')) {
      if (*PTR <= '9') {
        x = x * 16 + (*PTR - '0');
      } else {
        x = x * 16 + (*PTR - 'A' + 10);
      }
      gotnum = true;
      ++PTR;
    }
  } else {
    while ('0' <= *PTR && *PTR <= '9') {
      x = x * 10 + (*PTR - '0');
      gotnum = true;
      ++PTR;
    }
  }
  *num_out = x;
  return gotnum;
}

bool GetNum1Byte(byte* num_out) {
  word x;
  bool b = GetNum2Bytes(&x);
  *num_out = (byte)x;
  return b;
}

bool GetAddyInXid() {
  memset(Vars->transaction_id, 0, 4);
  if (GetNum1Byte(Vars->transaction_id+0) &&
      GetNum1Byte(Vars->transaction_id+1) &&
      GetNum1Byte(Vars->transaction_id+2) &&
      GetNum1Byte(Vars->transaction_id+3) ) {
    return true;
  }
  return false;
}

byte MaskBits(int n) {
  byte z = 0;
  while (n>0) {
    z = (z >> 1) | 0x80;
    n--;
  }
  return z;
}

void SetMask(byte width) {
      if (width == 0) width = 24;
      Vars->mask_num = width;
      Vars->ip_mask[0] = MaskBits((int)width);
      Vars->ip_mask[1] = MaskBits((int)width-8);
      Vars->ip_mask[2] = MaskBits((int)width-16);
      Vars->ip_mask[3] = MaskBits((int)width-24);
}

errnum DoNetwork(byte a, byte b) {
  byte tail;
  if (!GetNum1Byte(&tail)) {
    tail = 30 + (byte)(WizTicks() % 200u);  // random 30..229
  }
  Vars->ip_addr[0] = a;
  Vars->ip_addr[1] = b;
  Vars->ip_addr[2] = 23;
  Vars->ip_addr[3] = tail;

  Vars->ip_gateway[0] = a;
  Vars->ip_gateway[1] = b;
  Vars->ip_gateway[2] = 23;
  Vars->ip_gateway[3] = 1;

  Vars->ip_waiter[0] = a;
  Vars->ip_waiter[1] = b;
  Vars->ip_waiter[2] = 23;
  Vars->ip_waiter[3] = 23;

  SetMask(24);
  return OKAY;
}

#if 0
void GetHostname() {
  byte* h = Vars->hostname;
  byte n = sizeof(Vars->hostname);

  memset(h, '_', n);
  SkipWhite();
  while ('!' <= *PTR && *PTR <= '~') {
    for (byte i = 0; i < n-1; i++) {
      h[i] = h[i+1];  // Shift name to the left.
    }
    h[n-1] = *PTR;
    ++PTR;
  }
}
#endif

void ShowNetwork() {
  if (Vars->use_dhcp) {
    PrintF("d\1\n");
  } else {
    PrintF("i\1 %a/%d %a\n", Vars->ip_addr, Vars->mask_num, Vars->ip_gateway);
  }
  PrintF("w\1 %a:%d\n", Vars->ip_waiter, Vars->waiter_port);
}

void DoOneCommand() {
  errnum e = OKAY;
  word peek_addr = 0;

  SkipWhite();
  char cmd = *PTR;
  PTR++;

  if (cmd == 'U') {
      Vars->wiz_port = (struct wiz_port*)0xFF78;
  } else if (cmd == 'D') {
      // GetHostname();
      Vars->use_dhcp = true;
  } else if (cmd == 'I') {
      if (!GetAddyInXid()) { e = 11; goto END; }
      memcpy(Vars->ip_addr, Vars->transaction_id, 4);
      memset(Vars->ip_gateway, 0, 4);
      byte width;
      if (GetNum1Byte(&width)) {
        if (GetAddyInXid()) {
          memcpy(Vars->ip_gateway, Vars->transaction_id, 4);
        }
      }
      SetMask(width);

  } else if (cmd == 'W') {
    if (!GetAddyInXid()) { e = 12; goto END; }
    memcpy(Vars->ip_waiter, Vars->transaction_id, 4);
    word port;
    if (GetNum2Bytes(&port)) {
        Vars->waiter_port = port;
    }

  } else if (cmd == 'S') {
    ShowNetwork();

  } else if (cmd == 'A') {
    e = DoNetwork(10, 23);

  } else if (cmd == 'B') {
    e = DoNetwork(176, 23);

  } else if (cmd == 'C') {
    e = DoNetwork(192, 168);

  } else if (cmd == 'X') {
    e = DoNetwork(127, 23);

  } else if (cmd == 'Y') {
    e = DoNetwork(44, 23);

  } else if (cmd == 'Z') {
    e = DoNetwork(0, 23);

  } else if (cmd == '@') {
    Vars->launch = true;
    goto END;

  } else if (cmd == 'Q') {
    *(byte*)0xFFD9 = 1;

  } else if (cmd == 'N') {
    NativeMode();

  } else if (cmd == '<') { // peek
    word tmp = peek_addr;
    if (!GetNum2Bytes(&peek_addr)) peek_addr = tmp;

    PrintF("%x: ", peek_addr);
    for (byte i = 0; i < 8; i++) {
      PrintF(" %x", *(byte*)peek_addr);
      ++peek_addr;
    }
    PutChar(13);

  } else if (cmd == '>') { // poke
      byte b;
    if (GetNum2Bytes(&peek_addr) && GetNum1Byte(&b)) {
      *(byte*)peek_addr = b;
    } else {
      PrintF("?\n");
    }

  } else if (cmd == 0 || cmd==';') {
    // end of line
  } else {
    PrintF("U\1 :upper wiznet port $FF78\n");
    PrintF("Q\1 :quick: poke 1 to $FFD9\n");
    PrintF("N\1 :native mode for H6309\n");
    PrintF("D\1 :use DHCP\n");
    PrintF("I\1 1.2.3.4/24 5.6.7.8\n");
    PrintF("  :Set IP addr, mask, gateway\n");
    PrintF("W\1 3.4.5.6:%d :set waiter\n", WAITER_TCP_PORT);
    PrintF("A\1 :preset 10.23.23.*\n");
    PrintF("B\1 :preset 176.23.23.*\n");
    PrintF("C\1 :preset 192.168.23.*\n");
    PrintF("X\1 or Y\1 or Z\1 :goofy presets\n");
    PrintF("S\1 :show settings\n");
    PrintF("Finally:  @\1 : launch!\n");
  }

END:
  if (e) {
    PrintF("*** error %d\n", e);
  }
}

////////////////////////////////////////
//   PART-MAIN
////////////////////////////////////////

void CallWithNewStack(word new_stack, func_t fn) {
  asm volatile("ldx %0\n  ldy %1\n  tfr x,s\n  jsr ,y"
      : // no outputs
      : "m" (new_stack), "m" (fn) // two inputs
  );
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

////////////////////////////////////////////////

// Only called for TCP Client.
// Can probably be skipped.
errnum WizCheck(PARAM_JUST_SOCK) {
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
      if (ir & SK_IR_TOUT) { // Timeout?
        return SK_IR_TOUT;
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        return SK_IR_DISC;
      }
      return OKAY;
}

errnum WizRecvGetBytesWaiting(const struct sock* sockp, word* bytes_waiting_out) {
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;

  *bytes_waiting_out = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
  return OKAY;
}

errnum WizRecvChunkTry(const struct sock* sockp, char* buf, size_t n) {
  word bytes_waiting = 0;
  errnum e = WizRecvGetBytesWaiting(SOCK_AND &bytes_waiting);
  if (e) return e;
  if( bytes_waiting < n) return NOTYET;

  word rd = WizGet2(B+SK_RX_RD0);
  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizGetN(R+begin, buf, first_n);
    WizGetN(R, buf+first_n, second_n);
  } else {
    WizGetN(R+begin, buf, n);
  }

  WizPut2(B+SK_RX_RD0, rd + n);
  WizIssueCommand(SOCK_AND  SK_CR_RECV); // Inform socket of changed SK_RX_RD.
  return OKAY;
}

errnum WizRecvChunk(const struct sock* sockp, char* buf, size_t n) {
  PrintH("WizRecvChunk %x...", n);
  errnum e;
  do {
    e = WizRecvChunkTry(SOCK_AND buf, n);
  } while (e == NOTYET);
  PrintH("WRC %x: %x %x %x %x %x.", n,
      buf[0], buf[1], buf[2], buf[3], buf[4]);
  return e;
}
errnum WizRecvChunkBytes(const struct sock* sockp, byte* buf, size_t n) {
  return WizRecvChunk(sockp, (char*)buf, n);
}
errnum TcpRecv(const struct sock* sockp, char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizRecvChunk(SOCK_AND  p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  return OKAY;
}

void UdpDial(const struct sock* sockp,  const struct proto *proto,
             const byte* dest_ip, word dest_port) {
  if (proto->is_broadcast) {
    // Broadcast to 255.255.255.255 to FF:FF:FF:FF:FF:FF.
    WizPutN(B+6/*Sn_DHAR*/, SixFFs, 6);
    WizPutN(B+SK_DIPR0, SixFFs, 4);
  } else {
    WizPutN(B+SK_DIPR0, dest_ip, 4);
  }
  WizPut2(B+SK_DPORTR0, dest_port);
}

// returns tx_ptr
tx_ptr_t WizReserveToSend(const struct sock* sockp,  size_t n) {
  PrintH("ResTS %x;", n);
  // Wait until free space is available.
  word free_size;
  do {
    free_size = WizGet2(B+SK_TX_FSR0);
    PrintH("Res free %x;", free_size);
  } while (free_size < n);

  return WizGet2(B+SK_TX_WR0) & RING_MASK;
}

tx_ptr_t WizBytesToSend(const struct sock* sockp, tx_ptr_t tx_ptr, const byte* data, size_t n) {
  return WizDataToSend(SOCK_AND tx_ptr, (char*)data, n);
}
tx_ptr_t WizDataToSend(const struct sock* sockp, tx_ptr_t tx_ptr, const char* data, size_t n) {

  word begin = tx_ptr;  // begin: Beneath RING_SIZE.
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  return (n + tx_ptr) & RING_MASK;
}

void WizFinalizeSend(const struct sock* sockp, const struct proto *proto, size_t n) {
  word tx_wr = WizGet2(B+SK_TX_WR0);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  WizIssueCommand(SOCK_AND  proto->send_command);
}

errnum WizSendChunk(const struct sock* sockp,  const struct proto* proto, char* data, size_t n) {
  PrintH("Ax WizSendChunk %x@%x : %x %x %x %x %x", n, data,
      data[0], data[1], data[2], data[3], data[4]);
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;
  tx_ptr_t tx_ptr = WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND tx_ptr, data, n);
  WizFinalizeSend(SOCK_AND proto, n);
  PrintH("Ax WSC.");
  return OKAY;
}
errnum TcpSend(const struct sock* sockp,  char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizSendChunk(SOCK_AND &TcpProto, p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  return OKAY;
}

void WizClose(PARAM_JUST_SOCK) {
  WizIssueCommand(SOCK_AND 0x10/*CLOSE*/);
  WizPut1(B+SK_MR, 0x00/*Protocol: Socket Closed*/);
  WizPut1(B+0x0002/*_IR*/, 0x1F); // Clear all interrupts.
}

//////////////////////////////

extern void DoKeyboardCommands(char initial_char);
extern errnum RunDhcp(const struct sock* sockp, const char* name4, word ticks);

word RomSum(word begin, word end) {
  word sum = 0;
  while (begin < end) {
    sum += *(word*)begin;
    begin += 2;
  }
  return sum;
}

// --main--

void OpenDiscoverySockets() {
  WizOpen(SOCK1_AND &BroadcastUdpProto, DHCP_CLIENT_PORT);
  UdpDial(SOCK1_AND  &BroadcastUdpProto,
          /*dest_ip=*/ (const byte*)SixFFs, DHCP_SERVER_PORT);
  WizOpen(SOCK0_AND &BroadcastUdpProto, LAN_CLIENT_PORT);
  UdpDial(SOCK0_AND  &BroadcastUdpProto,
          /*dest_ip=*/ (const byte*)SixFFs, LAN_SERVER_PORT);
}

errnum OneDiscoveryRound() {
  SPIN(4);
  errnum e;
  struct UdpRecvHeader hdr;
  if (!Vars->got_lan) {
    e = WizRecvChunkTry(SOCK0_AND  (char*)&hdr, sizeof hdr);
    PrintF(" $$$%x ", e);
    if (!e) {
      PrintF("$$$%x ", hdr.len);
      e = RecvLanReply(SOCK0_AND hdr.len);
      PrintF("$$$%x ", e);
      if (!e) {
        Vars->got_lan = true;
        PutStr(" [ ");
        PutStr(Vars->line_buf);
        PutStr(" ] ");
      }
    }
  }

  if (!Vars->got_dhcp) {
    e = WizRecvChunkTry(SOCK1_AND  (char*)&hdr, sizeof hdr);
    //PrintF("R1:%x,", e);
    if (!e) {
      //PrintF("R1=%x,", hdr.len);
      e = RecvDhcpReply(SOCK1_AND hdr.len, false);
      //PrintF("=%x;", e);
      if (!e) Vars->got_dhcp = true;
    }
  }

  if (!Vars->got_lan) {
    SendLanRequest(JUST_SOCK0);
    //PrintF("S0;");
  }

  if (!Vars->got_dhcp) {
    SendDhcpRequest(SOCK1_AND  false/*not second time*/);
    //PrintF("S1;");
  }
  return 0;
}

errnum DhcpPhaseTwo() {
  errnum e;
  struct UdpRecvHeader hdr;
  for (byte i = 0; i < 10; i++) {
      PrintF(" *%d* ", i);

      SendDhcpRequest(SOCK1_AND  true/*second time*/);
      Delay(10000);

      e = WizRecvChunkTry(SOCK1_AND  (char*)&hdr, sizeof hdr);
      if (e) continue;

      e = RecvDhcpReply(SOCK1_AND hdr.len, true);
      if (!e) return 0;
  }

  return 2;
}

#define TOCKS_PER_SECOND 39 // TOCK = 256 TICKS = 25.6 milliseconds.

char CountdownOrInitialChar() {
  PrintF("\n\nTo take control, hit space bar.\n\n");
  OpenDiscoverySockets();

  for (byte i = 0; i < 5; i++) { SPIN(0);
    byte t = WizTocks();
    PrintF("%d... ", 5-i);
    OneDiscoveryRound();
    Beep(8, 12);
    while(1) {
      SPIN(2);
      byte now = WizTocks();
      byte interval = now - t; // Unsigned Difference tolerates rollover.
      if (interval > TOCKS_PER_SECOND) break;

      char initialChar = PolCat();
      if (initialChar) {
        return initialChar;
      }
      if (IsThisGomar()) break;
    }
  }
  PrintF("0.\n");
  return '\0';
}

void ComputeRomSums() {
    Vars->rom_sum_0 = RomSum(0x8000, 0xA000);
    Vars->rom_sum_1 = RomSum(0xA000, 0xC000);
    Vars->rom_sum_2 = RomSum(0xC100, 0xC800);
    Vars->rom_sum_3 = RomSum(0xE000, 0xF000);
}


errnum LemmaClientS1() {  // old style does not loop.
  char quint[5];
  char inkey;
  errnum e;  // was bool e, but that was a mistake.
PrintH("PolC");
    inkey = PolCat();
    if (inkey) {
      memset(quint, 0, sizeof quint);
      quint[0] = CMD_INKEY;
      quint[4] = inkey;
      e = WizSendChunk(SOCK1_AND &TcpProto,  quint, sizeof quint);
      if (e) return e;
    }

PrintH("RTry");
    e = WizRecvChunkTry(SOCK1_AND quint, sizeof quint);
    if (e == OKAY) {
      word n = *(word*)(quint+1);
      word p = *(word*)(quint+3);
      switch ((byte)quint[0]) {
        case CMD_POKE:
          {
            PrintH("POKE(%x@%x)\n", n, p);
            TcpRecv(SOCK1_AND (char*)p, n);
            PrintH("POKE DONE\n");
          }
          break;
        case CMD_PUTCHAR:
          {
              PutChar(p);
          }
          break;
        case CMD_PEEK:
          {
            PrintH("PEEK(%x@%x)\n", n, p);

            quint[0] = CMD_DATA;
            WizSendChunk(SOCK1_AND &TcpProto, quint, 5);
            TcpSend(SOCK1_AND (char*)p, n);
            PrintH("PEEK DONE\n");
          }
          break;
        case CMD_JSR:
          {
            func_t fn = (func_t)p;
            PrintH("JSR(%x@%x)\n", p);
            fn();
          }
          break;
        default:
          Fatal("WUT?", quint[0]);
          break;
      } // switch
    }
//SPIN(20);
    return (e==NOTYET) ? OKAY : e;
}

struct wiz_port* DetectWizPort() {
    // The wiznet contorl port reads as 0x03, after a reset.
    // We use that as its signature, to determine which of the two
    // standard addresses the wiznet card is jumpered for.
    word ff68 = 0xff68;
    word ff78 = 0xff78;
    word result = ff68;
    byte b68 = *(volatile byte*)ff68;
    //Delay(2345);
    byte b78 = *(volatile byte*)ff78;
    if ( b68 != 3 && b78 == 3 ) result = ff78;
    PrintF(" wiz<%x %x %x> ", b68, b78, result);

    for (byte i = 0; i < 4; i++) {
        byte a=*(volatile byte*)(ff68+i);
        //Delay(2345);
        byte b=*(volatile byte*)(ff78+i);
        //Delay(2345);
        PrintF("<%x %x> ", a, b);
    }
    return (struct wiz_port*) result;
}

void WaitForLink() {
    while (1) {
        byte phys = WizGet1(0x003C); // PHYSR0: physical register 0
        if ((phys & 1) == 1) {
            // link up
            PrintF(" LINK=%x ", phys);
            return;
        }
        PrintF("down=%x ", phys);
        Delay(5000);
    }
}

extern int main();
void main2() {
    Orange();
    // Clear our global variables to zero, except first 4 bytes,
    // which are orig_s_reg and address of main.
    memset(((char*)Vars) + 4, 0, sizeof *Vars - 4);

    // Set VDG 32x16 text screen to dots.
    // Preserve the top line.
    Vars->vdg_addr = VDG_RAM;
    Vars->vdg_begin = VDG_RAM+32; // Skip top status line.
    Vars->vdg_ptr = VDG_RAM+32;
    Vars->vdg_end = VDG_END;

    memset((byte*)VDG_RAM, ' ', VDG_END-VDG_RAM);
    strcpy((char*)VDG_RAM+26, "AXIOM\x74");

    if (ValidateWizPort((struct wiz_port*)0xFF68)==OKAY) {
        Vars->wiz_port = (struct wiz_port*)0xFF68;
    } else if (ValidateWizPort((struct wiz_port*)0xFF78)==OKAY) {
        Vars->wiz_port = (struct wiz_port*)0xFF78;
    } else {
        Fatal("nowiz", 9);
    }
    PrintF("Wiznet found at %x\n", Vars->wiz_port);

    Vars->rand_word = WizTicks();
    memcpy(Vars->transaction_id, Rom->rom_mac_tail+1, 4);
    memcpy(Vars->hostname, Rom->rom_hostname, 8);

    PrintF("?=%x V=%x M=%x:%x:%x\n",
        Vars->rand_word,
        sizeof(*Vars),
        *(byte*)(Rom->rom_mac_tail+0),
        *(word*)(Rom->rom_mac_tail+1),
        *(word*)(Rom->rom_mac_tail+3));

    ComputeRomSums();
    PrintF("S=%x E=%x R=%x:%x:%x:%x CPU=",
      Vars->orig_s_reg,
      Vars->main,
      Vars->rom_sum_0,
      Vars->rom_sum_1,
      Vars->rom_sum_2,
      Vars->rom_sum_3);

    if (CpuType() == 0x9821) {
      PrintF("HITACHI ");
    } else {
      PrintF("MOTOROLA ");
    }

    PutChar('\"');
    for (byte i = 0; i<8; i++) {
        char ch = Vars->hostname[i];
        if (' ' <= ch && ch <= '~') PutChar(ch);
    }
    PutChar('\"');

    /////////////////
    if (IsThisGomar()) {
      // FOR EMULATOR
      memcpy(Vars->ip_addr, "\x7f\x00\x00\x01", 4);
      memcpy(Vars->ip_waiter, "\x7f\x00\x00\x01", 4);
      memcpy(Vars->ip_mask, "\xff\xff\xff\x00", 4);
      Vars->waiter_port = 2319;
      Vars->use_dhcp = 0; // Just so ShowNetwork will show network.

    } else {
      // FOR HUMANS

      // Preset defaults for Waiter.
      memcpy(Vars->ip_waiter, waiter_default, 4);
      Vars->waiter_port = WAITER_TCP_PORT;

      WizReset();
      WaitForLink();
      WizConfigure();
      char initial_char = CountdownOrInitialChar();
      if (!initial_char) {
        if (Vars->got_lan) {
          PrintF("USE LAN;");
        } else if (Vars->got_dhcp) {
          PrintF("USE DHCP;");
          Vars->use_dhcp = true;
        }
      }

      if (!initial_char && Vars->got_lan) {
          Beep(32, 4);
          DoLineBufCommands();
      } else if (!Vars->use_dhcp) {
          Beep(8, 16);
          DoKeyboardCommands(initial_char);
      } else {
          Beep(16, 8);
      }

      if (Vars->use_dhcp) {
        errnum e = OKAY;
        e = DhcpPhaseTwo();
        if (e) Fatal("Dhcp2", e);
      }

    }
    /////////////////

    WizReset();
    WizConfigure();

    WizOpen(SOCK1_AND &TcpProto, Vars->rand_word);
    PrintF("tcp dial %a:%u; ", Vars->ip_waiter, Vars->waiter_port);

    Vars->use_dhcp = 0; // Just so ShowNetwork will show network.
    ShowNetwork();
    TcpDial(SOCK1_AND Vars->ip_waiter, Vars->waiter_port);

    TcpEstablish(JUST_SOCK1);
    PrintF(" CONN; ");
    Green();
    Delay(10000);

    while (1) {
        errnum e = LemmaClientS1();
        if (e) Fatal("S1", e);
    }
}

void DoLineBufCommands() {
  PTR = BUF;
  do {

    PutChar('[');
    for (const char* s = PTR; *s; s++) PutChar(*s);
    PutChar(']');
    PutChar('\n');

    DoOneCommand();
    SkipWhite();
  } while (*PTR);
}

void DoKeyboardCommands(char initial_char) {
  // Set up defaults.
  SetMask(24);

  PrintF("Enter H\001 for HELP.\n");

  while (true) {
    PrintF(">axiom> ");
    GetUpperCaseLine(initial_char);
    initial_char = 0;

    DoLineBufCommands();
    if (Vars->launch) break;
  }
}

int main() {
    // Remembering the original S register
    // can give us some idea how big RAM isn't.
    Vars->orig_s_reg = StackPointer();
    Vars->main = (word)&main;
    //_

    // Now to make all platforms the same,
    // we're dropping down into the first 4K of RAM
    // and using the second floppy buffer 0x0700:0x0800 for stack.
    CallWithNewStack(0x0800, &main2);
}
