
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;

#ifdef unix
typedef unsigned long word;
#define Debug(...) printf(__VA_ARGS__)
#else
typedef unsigned int word;
typedef unsigned int size_t;
#define Debug(...) // nothing
#endif

#define NUM_SHIPS 3

#define SPACE_WAR_PORT 3210

struct body {
	word x, y;
	int r, s;
	word score;
	byte direction;
	byte ttl;
};

#ifdef unix

#include <stdio.h>

struct wario_vars {
    struct body ship[NUM_SHIPS];
    struct body missile[NUM_SHIPS];

} WarioVars;
#define Vars (&WarioVars)

byte vdg_ram[8 * 1024];
#define VDG_RAM ((word)(size_t)vdg_ram)
#define PEEK(ADDR) (*(volatile byte*)(size_t)(ADDR))
#define POKE(ADDR, X) ( (*(volatile byte*)(size_t)(ADDR)) = (byte)(X) )
#define PXOR(ADDR, X) ( (*(volatile byte*)(size_t)(ADDR)) ^= (byte)(X) )
void Delay(word n) {}

#else

#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!
#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600
#define PACKET_BUF 0x600
#define PACKET_MAX 256

#define PrintH(F,...) // nothing

#define PEEK(ADDR) (*(volatile byte*)(ADDR))
#define POKE(ADDR, X) ( (*(volatile byte*)(ADDR)) = (byte)(X) )
#define PXOR(ADDR, X) ( (*(volatile byte*)(ADDR)) ^= (byte)(X) )

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

struct wiz_udp_recv_header {
    byte ip_addr[4];
    word udp_port;
    word udp_payload_len;
};

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
void WizConfigure(word rando);
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
    byte mode;

    word displayed_score[NUM_SHIPS];
    struct body ship[NUM_SHIPS];
    struct body missile[NUM_SHIPS];

    word vdg_addr;   // mapped via SAM
    word vdg_begin;  // begin useable portion
    word vdg_end;    // end usable portion
    word vdg_ptr;    // inside usable portion

    void (*lemma_loop)(void);  // Where to re-join the loop.

    char buffer[80];
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

char Digits[] =
		" 0 "
		"0 0"
		"0 0"
		"0 0"
		" 0 "

		" 1 "
		" 1 "
		" 1 "
		" 1 "
		" 1 "

		"22 "
		"  2"
		" 2 "
		"2  "
		"222"

		"33 "
		"  3"
		"33 "
		"  3"
		"33 "

		"4 4"
		"4 4"
		"444"
		"  4"
		"  4"

		"555"
		"5  "
		"55 "
		"  5"
		"55 "

		" 66"
		"6  "
		"666"
		"6 6"
		"666"

		"777"
		"  7"
		"  7"
		"  7"
		"  7"

		"888"
		"8 8"
		"888"
		"8 8"
		"888"

		"999"
		"9 9"
		"999"
		"  9"
		"  9"
;

////////////////////////////////////////////////////////

void SPIN(byte n) {
    byte b = 1 + *(byte*)(VDG_RAM+(n));
    if ((b&15) == 0) b++;  // Skip blank patterns.
    (*(byte*)(VDG_RAM+(n))) = b | 0x80;
}

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

void Enable1BitSound() {
    *(volatile byte*)0xFF23 &= ~0x04;  // Clear bit 2 to enable Data Direction access
    *(volatile byte*)0xFF22 |= 0x02;   // Bit 1 and bits 3-7 are outputs.
    *(volatile byte*)0xFF23 |= 0x04;  // Clear bit 2 to enable Data Direction access
}
void Beep(byte n, byte f) {
    Enable1BitSound();

    for (byte i = 0; i < n; i++) {
    	Pia1bOn(0x02);
    	Delay(f);
    	Pia1bOff(0x02);
    	Delay(f);
    }
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

byte Zeros[4] = { 0,0,0,0 };

void WizConfigure(word rando) {
  WizPutN(0x0001/*gateway*/, Zeros, 4);

  WizPut1(0x0005/*mask+0*/, 255);
  WizPutN(0x0006/*mask+1*/, Zeros, 4-1);

  byte ip[4] = { 10, 10, (byte)(rando>>8), (byte)rando };
  WizPutN(0x000F/*ip_addr*/, ip, 4);

  byte mac[6] = { 2, 2, 2, 2, (byte)(rando>>8), (byte)rando};
  WizPutN(0x0009/*ether_mac*/, mac, 6);
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

//////////////////////////////////////////////////////////////////
const char SixFFs[6] = {(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF};

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
  // PrintH("ResTS %x;", n);
  // Wait until free space is available.
  word free_size;
  do {
    free_size = WizGet2(B+SK_TX_FSR0);
    // PrintH("Res free %x;", free_size);
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
  // PrintH("Ax WizSendChunk %x@%x : %x %x %x %x %x", n, data, data[0], data[1], data[2], data[3], data[4]);
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;
  tx_ptr_t tx_ptr = WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND tx_ptr, data, n);
  WizFinalizeSend(SOCK_AND proto, n);
  // PrintH("Ax WSC.");
  return OKAY;
}

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
//////////////////////////////////////////////////////////////////

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
  // PrintH("WizRecvChunk %x...", n);
  errnum e;
  do {
    e = WizRecvChunkTry(SOCK_AND buf, n);
  } while (e == NOTYET);
  // PrintH("WRC %x: %x %x %x %x %x.", n, buf[0], buf[1], buf[2], buf[3], buf[4]);
  return e;
}
errnum WizRecvChunkBytes(const struct sock* sockp, byte* buf, size_t n) {
  return WizRecvChunk(sockp, (char*)buf, n);
}
//////////////////////////////////////////////////////////////////
#endif

// gcc6809 -f'whole-program' doesn't like libgcc runtime library calls.
// So here are ShiftRight and ArithShiftRight tht avoid library calls.
word ShiftLeft(word x, byte count) {
	for (byte i = 0; i < count; i++) {
		x <<= 1;
	}
	return x;
}

byte ByteShiftLeft(byte x, byte count) {
	for (byte i = 0; i < count; i++) {
		x <<= 1;
	}
	return x;
}

int ShiftRight(int x, byte count) {
	word z = (word)x;
	for (byte i = 0; i < count; i++) {
		z >>= 1;
	}
	return (int)z;
}

int ArithShiftRight(int x, byte count) {
	word z = (word)x;
	for (byte i = 0; i < count; i++) {
		z >>= 1;
		if (x & 0x8000) z |= 0x8000;
	}
	return (int)z;
}

/////////////////////////////////////////////////////////////

// We only scan this keyboard row 3:
#define KEY_X  (1<<0)
#define KEY_Y  (1<<1)
#define KEY_Z  (1<<2)
#define KEY_UP   (1<<3)
#define KEY_DOWN (1<<4)
#define KEY_LEFT (1<<5)
#define KEY_RIGHT (1<<6)
#define KEY_SPACE (1<<7)

// Returns a bitmap of the above bits, 1 if key down, otherwise 0.
byte RelevantKeysDown() {
  const byte row3 = (1<<3);
  byte z = 0;
  for (byte b = 0x80; b; b>>=1) {
  	POKE(0xFF02, 0xFF ^ b);  // Key sense is active low.
	byte c = PEEK(0xFF00);
	if ((c & row3) == 0) z |= b;
  }
  POKE(0xFF02, 0xFF);  // turn off the current.
  return z;
}

/////////////////////////////////////////////////////////////

struct broadcast_payload {
	byte magic_aa;
	byte ship_num;
	word score;
	struct body ship, missile;
};
void BroadcastShip(int ship_num) {
	if (Vars->mode == 'S') return;

	struct broadcast_payload p = {
		.magic_aa= 0xAA,
		.ship_num= ship_num,
		.score = 0x2345,
		.ship= Vars->ship[ship_num],
		.missile= Vars->missile[ship_num],
	};
	
	word plen = sizeof(p);
  	tx_ptr_t tx_ptr = WizReserveToSend(SOCK0_AND plen);
  	tx_ptr = WizBytesToSend(SOCK0_AND tx_ptr, (byte*)(&p), plen);
  	WizFinalizeSend(SOCK0_AND &BroadcastUdpProto, plen);
}

byte Ships[] = {
#include "spacewar-ships.h"
};

byte Gravity[] = {
#include "spacewar-gravity.h"
};

#define GRAF_LEN 0xC00  // (i.e. 3072 bytes) for G3CMode

// G3CMode is 128 columns, 96 rows, 4 colors,
// using a buffer size of 3072 bytes.
// Rows are 32 bytes with 4 pixels per byte.
// Color set C=0: enum { Green, Yellow, Blue, Red } border Green.
// Color set C=1: enum { Buff, Cyan, Magenta, Orange } border Buff.
// Color set is bit 3 of $FF22.
void G3CMode() {
#if !defined(unix)
	POKE(0xFFC5, 0);
	POKE(0xFFC2, 0);
	POKE(0xFFC0, 0);
	POKE(0xFF22, 0xC8);  // C0 for color0, C8 for color1.
#endif
}

#define W 128u
#define H 96u

void PeekScreen() {
#ifdef unix
	for (int y = 0; y < H; y++) {
		for (int x = 0; x < 32; x++) {
			int i = x + y*32;
			printf("%2x ", vdg_ram[i]);
		}
		printf("\n");
	}
	printf("------------------------------------------------------------------------------------\n");
#endif
}

void ComputeGravity(word x, word y, int* gx_out, int* gy_out) {
	*gx_out = *gy_out = 0;
	x >>= 8;  // No fractional part.
	y >>= 8;  // No fractional part.

	// tx: table x, reduced to one quadrant, 0..W/2-1.
 	word tx = (x > W/2) ? W-1-x : x;
	// ty: table y, reduced to one quadrant, 0..H/2-1.
 	word ty = (y > H/2) ? H-1-y : y;
Debug("G: x,y=%d,%d tx,ty=%d,%d\n", x, y, tx, ty);

	int abs_gx, abs_gy;
	if (tx >= W/2-W/8 && ty >= H/2-H/8) {
		// High resolution
		word ix = (tx - (W/2-W/8));  // index x
		word iy = (ty - (H/2-H/8));  // index y
		word index = /* 2*(H/2)*(W/2) + */ (ix<<2) + (iy<<(2+3));  // four-byte records.
		abs_gx = (Gravity[index]<<8) + Gravity[index+1];
		abs_gy = (Gravity[index+2]<<8) + Gravity[index+3];
Debug("G:HI:    ix,iy=%d,%d index=%d  abs=%d,%d\n", ix, iy, index, abs_gx, abs_gy);
	} else {
		// Low resolution
#define ANTTI_GRAVITY 0 // 4
#if 1
		byte tiny_grav = 3;
		if (tx + ty < H/4) tiny_grav = 1;
		else if (tx + ty < H/2) tiny_grav = 2;

		abs_gx = abs_gy = (tiny_grav << ANTTI_GRAVITY);
#else
		word ix = (tx>>1);  // index x
		word iy = (ty>>1);  // index y
		word index = (ix<<1) + (iy<<(1+5));  // two-byte records.
		abs_gx = Gravity[index];
		abs_gy = Gravity[index+1];
#endif
Debug("G:LO:    abs=%d,%d\n", abs_gx, abs_gy);
	}

#define GL 1000
	abs_gx = (abs_gx > GL) ? GL : abs_gx;
	abs_gy = (abs_gy > GL) ? GL : abs_gy;

	abs_gx = ArithShiftRight(abs_gx, ANTTI_GRAVITY);
	abs_gy = ArithShiftRight(abs_gy, ANTTI_GRAVITY);
	*gx_out = (x<W/2) ? abs_gx : -abs_gx;
	*gy_out = (y<H/2) ? abs_gy : -abs_gy;
Debug("G:RET    out = %d,%d\n", *gx_out, *gy_out);
}

#if 0
void ShowShips() {
	word ship = 0; // Index into Ships[] char gen to display.

	// c is column number.  We can make 8 columns, each 4 bytes
	// wide, which is 16 pixels wide.  The two bytes in the middle
	// will be populated with space ships.  The two bytes on the
	// edge of each column just show a border line, one pixel back.
	for (word c = 0; c < 8; c++) {
		// row is graphics screen row number.
		// Shifted left 5, it is a RAM offset for the row (32 bytes).
		for (word row = 0; row < 96; row++) {
			// v is the video address, of the 4-byte slot.
			word v = VDG_RAM + (row<<5) + (c<<2);

			POKE(v, 8); // border
			POKE(v+1, Ships[ship++]);
			POKE(v+2, Ships[ship++]);
			POKE(v+3, 12<<2); // border
		}
	}
}
#endif

void DrawSpot(byte x, byte y, byte color) {
	byte xshift = x & 3;    // mod 4
	byte xdist = x >> 2;    // div 4
	word v = (size_t)VDG_RAM + xdist + ((word)y << 5);
	PXOR(v, ByteShiftLeft(color, (3-xshift)<<1));
}

void DrawDigit(byte x, byte y, byte color, byte digit) {
	char* pattern = Digits + (digit<<4) - digit; // that is, 15*digit.
	for (int i = 0; i < 5; i++) {
	  for (int j = 0; j < 3; j++) {
	    char spot = *pattern++;
	    if (spot != ' ') {
	    	DrawSpot(x+j, i+y, color);
	    }
	  }
	}
}

void DrawDecimal(byte x, byte y, byte color, word val) {
	byte a=0, b=0, c=0, d=0, e=0;
	while (val > 10000) { a++; val -= 10000; }
	while (val > 1000) { b++; val -= 1000; }
	while (val > 100) { c++; val -= 100; }
	while (val > 10) { d++; val -= 10; }
	bool show = false;
	if (a) { DrawDigit(x, y, color, a); show = true; }
	if (b || show) { DrawDigit(x+4, y, color, b); show = true; }
	if (c || show) { DrawDigit(x+8, y, color, c); show = true; }
	if (d || show) { DrawDigit(x+12, y, color, d); show = true; }
	DrawDigit(x+16, y, color, (byte)val);
}

void DrawShip(struct body* p, word ship, bool isaMissile) {
	if (!p->ttl) return;

	byte mask = 0xFF;  // four of color "11": 11111111
	switch (ship) {
	case 0:
		mask = 0x55;  // four of color "01": 01010101
		break;
	case 1:
		mask = 0xAA;  // four of color "10": 10101010
		break;
	}

	word xshift = (p->x >> 8) & 3;  // mod 4 // pixel offset within byte
	word xdist = (p->x >> 8) >> 2;  // div 4
	word index = (xshift * 5*2) + (p->direction * 5*4*2);
	word yval = p->y >> 8;

	if (isaMissile) {
		word v = (size_t)VDG_RAM + xdist + (yval << 5);
		word c0 = 0xC0;
                byte spot = ShiftRight(c0&mask , xshift);
		PXOR(v + 2, spot);
	} else {
		for (byte row = 0; row < 5*32; row+=32) {
			word v = (size_t)VDG_RAM + xdist + (yval << 5);
			if (v > (size_t)VDG_RAM + 3*1024) v -= 3*1024;   // wrap
			PXOR(v + row + 0, mask & Ships[index++]);
			PXOR(v + row + 1, mask & Ships[index++]);
		}
	}
}
#define XWRAP (word)(W*256u)
#define YWRAP (word)(H*256u)

word GraphicsAddr(word x, word y) {
	return VDG_RAM + (x>>2) + (y<<5);
}

void DrawSun() {
	byte* p = (byte*)VDG_RAM + 1024 + 512;
	p[17] += 64;
}

#define ACCEL 8 // 3
int AccelR[16] = {
  5*ACCEL, 4*ACCEL, 3*ACCEL, 1*ACCEL,
  0*ACCEL, -1*ACCEL, -3*ACCEL, -4*ACCEL,
  -5*ACCEL, -4*ACCEL, -3*ACCEL, -1*ACCEL,
  0*ACCEL, 1*ACCEL, 3*ACCEL, 4*ACCEL,
};
int AccelS[16] = {
  0*ACCEL, 1*ACCEL, 3*ACCEL, 4*ACCEL,
  5*ACCEL, 4*ACCEL, 3*ACCEL, 1*ACCEL,
  0*ACCEL, -1*ACCEL, -3*ACCEL, -4*ACCEL,
  -5*ACCEL, -4*ACCEL, -3*ACCEL, -1*ACCEL,
};

void ProcessIncomingPacket() {
  // struct broadcast_payload {
  // 	byte magic_aa;
  // 	byte ship_num;
  // 	word score;
  // 	struct body ship, missile;
  // };
  struct broadcast_payload *p = (void*)Vars->buffer;
  if (p->magic_aa != 0xAA) return;
  byte n = p->ship_num;
  if (n >= NUM_SHIPS) return;

  Vars->ship[n] = p->ship;
  Vars->missile[n] = p->missile;
}

void CheckIncomingPackets() {
	if (Vars->mode == 'S') return;

	  errnum e = WizRecvChunkTry(SOCK0_AND Vars->buffer, 8);
	  if (e == OKAY) {
		struct wiz_udp_recv_header* h = (void*)Vars->buffer;
		word len = h->udp_payload_len;
		if (h->udp_port == SPACE_WAR_PORT && len == sizeof(struct broadcast_payload)) {
	  		WizRecvChunkBytes(SOCK0_AND (void*)Vars->buffer, len);
			ProcessIncomingPacket();
		} else {
			// Bad incoming packet.
	  		WizRecvChunkBytes(SOCK0_AND (void*)0x000, len);
		}
	  }
}

void FireMissile(byte direction, byte who) {
	struct body* s = Vars->ship + who;
	struct body* m = Vars->missile + who;
	*m = *s; // copy ship's position and momentum
#define M_SPEED 30
	m->x += M_SPEED * AccelR[direction];
	m->y -= M_SPEED * AccelS[direction];
	m->r += M_SPEED * AccelR[direction];
	m->s -= M_SPEED * AccelS[direction];
	m->ttl = 148;
}

bool DetectHits(struct body* my_missile, byte my_num) {
	for (byte i = 0; i < NUM_SHIPS; i++) {
		if (i == my_num) continue;  // dont count self-hits
		struct body* p = Vars->ship + i;
		if (!p->ttl) continue;  // that ship does not exist
		word dx = (my_missile->x > p->x) ? (my_missile->x - p->x) : (p->x - my_missile->x);
		word dy = (my_missile->y > p->y) ? (my_missile->y - p->y) : (p->y - my_missile->y);
		word dist = dx + dy;
#define NEARBY 0x0600
		if (dist < NEARBY) {
			Vars->ship[my_num].score ++;
			my_missile->ttl = 0;  // expire the missile.
			return true;
		}
	}
	return false;
}

void AdvanceBody(struct body* p, int ship, bool useGravity) {
#define SLOW 3
			int new_x = (int)(p->x) + ArithShiftRight(p->r , SLOW);
			int new_y = (int)(p->y) + ArithShiftRight(p->s , SLOW);
			// Wrap to stay on torus.
			while (new_x < 0) { new_x += XWRAP; }
			while (new_y < 0) { new_y += YWRAP; }
			while (new_x >= XWRAP) { new_x -= XWRAP; }
			while (new_y >= YWRAP) { new_y -= YWRAP; }
Debug("xy=%d,%d  new=%d,%d  r,s=%d,%d\n", p->x, p->y, new_x, new_y, p->r, p->s);

			if (useGravity) {
				int gx, gy;
				ComputeGravity(new_x, new_y, &gx, &gy);
				p->r += gx;
				p->s += gy;
Debug("grav=%d,%d  r,s=%d,%d\n", gx, gy, p->r, p->s);
			}

#define SpeedLimit 6000

			p->r = (p->r < -SpeedLimit) ? -SpeedLimit : (p->r > SpeedLimit) ? SpeedLimit : p->r;
			p->s = (p->s < -SpeedLimit) ? -SpeedLimit : (p->s > SpeedLimit) ? SpeedLimit : p->s;
			p->x = (word)new_x;
			p->y = (word)new_y;
}

void DrawScores() {
	for (byte i = 0; i < NUM_SHIPS; i++) {
		struct body* p = Vars->ship + i;
		if (p->score != Vars->displayed_score[i]) {
			DrawDecimal(100, i<<3, i+1, p->score);  // erase old score
			DrawDecimal(100, i<<3, i+1, Vars->displayed_score[i]);  // add new score
			Vars->displayed_score[i] = p->score;
		}
	}
}

void DrawAll() {
		for (byte ship = 0; ship < NUM_SHIPS; ship++) {
			struct body* p = Vars->ship+ship;
			DrawShip(p, ship, false);
			struct body* m = Vars->missile+ship;
			DrawShip(m, ship, true);
		}
}

void Run() {
	byte my_num = Vars->mode == 'S' ? 1 : Vars->mode-'1';
	struct body* my = Vars->ship + my_num;
	struct body* my_missile = Vars->missile + my_num;

	// // Draw a constellation around the Strong Gravity Zone.
	// PXOR(GraphicsAddr( W/2-W/8, H/2-H/8 ), 0x02); // 0x80);
	// PXOR(GraphicsAddr( W/2-W/8, H/2+H/8 ), 0x02); // 0x80);
	// PXOR(GraphicsAddr( W/2+W/8, H/2-H/8 ), 0x02);
	// PXOR(GraphicsAddr( W/2+W/8, H/2+H/8 ), 0x02);

	if (0) {  // background stars
	  word x = 4, y = 7;
	  for (word i = 0; i < 8; i++) {
		PXOR(GraphicsAddr( x, y ), 0x02);
		x += 47 + 64;
		y += 33;
		while (x > W) x -= W;
		while (y > H) y -= H;
	  }
	}

	// byte direction = 0;
	word g = 0;
	word embargo = 0;
	for (word i = 0; i < NUM_SHIPS; i++) {
		volatile struct body* p = Vars->ship+i;
		p->x = (i<<13);
		p->y = (i<<12);
		p->r = 0; //31+7*i;
		p->s = 0; //17+3*i;
		p->ttl = (i == my_num) ? 255 : 0;

		Vars->displayed_score[i] = p->score;
		DrawDecimal(100, i<<3, i+1, p->score);
	}
LOOP:
	while (1) {
		if (true || g&1) { // every time was to toucy.
			// Check keyboard.
			byte keys = RelevantKeysDown();
			if (keys & KEY_Z) POKE(0xFF22, 0xC8);  // C0 for color0, C8 for color1.
			if (keys & KEY_X) POKE(0xFF22, 0xC0);  // C0 for color0, C8 for color1.
			if (keys & KEY_LEFT) my->direction = (my->direction+1)&15;
			if (keys & KEY_RIGHT) my->direction = (my->direction-1)&15;
			if (keys & KEY_UP) {
#define THRUST 3
				my->r += THRUST * AccelR[my->direction];
				my->s -= THRUST * AccelS[my->direction];
			}
			if (keys & KEY_SPACE && embargo < g) {
				FireMissile(my->direction, my_num);
			}
		}

		// Advance the ships.
		// byte modeship = Vars->mode-'1';
		// if (modeship < NUM_SHIPS) {
			// Vars->ship[modeship].direction = direction;
		// }
		for (word ship = 0; ship < NUM_SHIPS; ship++) {
			AdvanceBody(Vars->ship+ship, ship, true);
			AdvanceBody(Vars->missile+ship, ship, false);
		}

		if (my_missile->ttl && embargo < g) {
			bool hit = DetectHits(my_missile, my_num);
			if (hit) {
				embargo = g+100;
				Beep(30, 30);
			}
		}
		if (g == embargo - 1) {
			Beep(10, 90);
		}
DRAW:
		DrawAll();
		DrawScores();
// PeekScreen();
WORK:
		{
			if ((Vars->mode != 'S') && ((g&15)==0)) {
				BroadcastShip(Vars->mode - '1'); 
			} else {
				Delay(1000);
			}

			if ((g&3)==0) {
			  DrawSun();
			}
		}
UNDRAW:
		DrawAll();

CHECK:
		// Must UNDRAW before checking and depreciating.
		CheckIncomingPackets();

DEPRECIATE:
		if (Vars->mode == 'S') {
			  	for (byte i = 0; i < NUM_SHIPS; i++) {
  			  		Vars->ship[i].ttl = 255;  // keep everyone alive!
				}
		} else {
			  	for (byte i = 0; i < NUM_SHIPS; i++) {
					if (Vars->mode - '1' == i) {
  			  		  Vars->ship[i].ttl = 255;  // keep myself alive!
					} else {
  			  		    if (Vars->ship[i].ttl) Vars->ship[i].ttl--;   // depreciate others.
					}
				}
		}
		for (byte i = 0; i < NUM_SHIPS; i++) {
				if (Vars->missile[i].ttl) Vars->missile[i].ttl--;
		}

		g++;
#if 0
		if ((g&63)==0) {  // automatic direction turning
			direction = (1 + direction) & 15;
		}
		if ((g&255)==3) {  // automatic missile firing
			byte who = 1;
			if (Vars->mode != 'S') who = Vars->mode - '1';
			FireMissile(direction, who);
		}
#endif
	}
}

int main() {
#if !defined(unix)
    memset(((char*)Vars), 0, sizeof *Vars);
    Vars->bogus = (word)Ships;

    // Set VDG 32x16 text screen to dots.
    // Preserve the top line.
    Vars->vdg_addr = VDG_RAM;
    Vars->vdg_begin = VDG_RAM+32; // Skip top status line.
    Vars->vdg_ptr = VDG_RAM+32;
    Vars->vdg_end = VDG_END;

    memset((byte*)VDG_RAM, '-', VDG_END-VDG_RAM);
    strcpy((char*)VDG_RAM+20, "SPACEWARIO");

    if (ValidateWizPort((struct wiz_port*)0xFF68)==OKAY) {
        Vars->wiz_port = (struct wiz_port*)0xFF68;
    } else if (ValidateWizPort((struct wiz_port*)0xFF78)==OKAY) {
        Vars->wiz_port = (struct wiz_port*)0xFF78;
    } else {
        Fatal("nowiz", 9);
    }
    PrintF("Wiznet found at %x\n", Vars->wiz_port);

    PrintF("HOSTNAME=\"");
    for (byte i = 0; i<8; i++) {
        char ch = Rom->rom_hostname[i];
        if (' ' <= ch && ch <= '~') PutChar(ch);
    }
    PrintF("\"\n");

    {
    	// Reset before asking for input.
        WizReset();
        WaitForLink();
    }

    Vars->mode = 'S';
#if 1
    PrintF("\n\nChoose player number 1, 2, 3\n");
    PrintF("or for solitaire, hit S.\n");
    byte mode;
    while (1) {
    	mode = PolCat();
	if (mode > 96) mode -= 32;  // ToUpper
	if ('1' <= mode && mode <= '4' || mode=='S') break;
    }
    PrintF("MODE == %d\n\n", mode);
    Vars->mode = mode;
#endif
    // Delay(2500);

    {
    	// Get rand_word after asking for input,
	// so the timing will add randomness.
	Vars->rand_word = WizTicks();

        WizConfigure(Vars->rand_word);
  	WizOpen(SOCK0_AND &BroadcastUdpProto, SPACE_WAR_PORT);
  	UdpDial(SOCK0_AND  &BroadcastUdpProto,
          /*dest_ip=*/ (const byte*)SixFFs, SPACE_WAR_PORT);
    }

    G3CMode();
    memset((byte*)VDG_RAM, 0, GRAF_LEN);
    // ShowShips();
    // Delay(2500);
    memset((byte*)VDG_RAM, 0, GRAF_LEN);
#endif
    Run();
}
