#ifndef _FROB2_BOOTROM_BOOTROM_H_
#define _FROB2_BOOTROM_BOOTROM_H_

#define VERBOSE 3 // 9 // 6 // 3
#define EMULATED 0
#define PRINTK 1

#define X220NET 1
#define LOCALNET 0

#define BR_STATIC 1
#define BR_DHCP 0

#define MULTI_SOCK 1
#define TCP_CHUNK_SIZE 1024  // Chunk to send or recv in TCP.

#define WIZ_PORT  0xFF68   // Hardware port.

#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!

// Conventional types and constants for Frobio
typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0

typedef void (*func)();

#include "frob2/wiz/w5100s_defs.h"

//////////////////////////////////////////////////
//
//  Compiler Specific:

#ifdef __GNUC__

#include <stdarg.h>
typedef unsigned int size_t;

void abort(void);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcpy(char *restrict dest, const char *restrict src);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
size_t strlen(const char *s);

#else

#include <cmoc.h>
#include <stdarg.h>

#define volatile /* not supported by cmoc */

#endif

//  END Compiler Specific.
//
//////////////////////////////////////////////////

struct wiz_port {
  byte command;
  word addr;
  byte data;
};

struct vars {
    word vars_magic;
    word s_reg;  // Original Stack Pointer, for idea of mem size.
    volatile struct wiz_port* wiz_port;
    byte hostname[8];
    byte mac_addr[6];
    byte ip_addr[4];
    byte ip_mask[4];
    byte ip_gateway[4];
    byte ip_resolver[4];
    byte ip_waiter[4];
    char waiter_domain[32]; // unused
    word flags;     // unused: might tell us which fields above to use.
    byte hostchar;  // unused: might tell us what config to load or remote-load.

    word vdg_addr;   // mapped via SAM
    word vdg_begin;  // begin useable portion
    word vdg_end;    // end usable portion
    word vdg_ptr;    // inside usable portion
};

#define VARS_MAGIC 0x4756 /* 'G' 'V' global vars */

// Four of SockState immediately follow struct vars,
// but due to const initialization rules, we cannot
// make them part of the struct.
struct SockState {
  word tx_ptr;
  word tx_to_go;
};
extern const char SixFFs[6];
extern const char Eight00s[8];

#define VARS_RAM (CASBUF)
#define Vars ((struct vars*)VARS_RAM)
#define WIZ  (Vars->wiz_port)

#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600

#if MULTI_SOCK

struct sock {
    word base;
    word tx_ring;
    word rx_ring;
    word ss; // struct SockState* ss;
    byte nth;
};

extern const struct sock Socks[4];

#define SOCK1_AND (Socks+1),
#define JUST_SOCK1 (Socks+1)

#define PARAM_JUST_SOCK   const struct sock* sockp
#define PARAM_SOCK_AND    const struct sock* sockp,
#define JUST_SOCK         sockp
#define SOCK_AND          sockp,

#define B    (sockp->base)
#define T    (sockp->tx_ring)
#define R    (sockp->rx_ring)
#define SS   ((struct SockState*)sockp->ss)
#define N    (sockp->nth)

#else

#define SOCK1_AND
#define JUST_SOCK1 

  #define PARAM_JUST_SOCK
  #define PARAM_SOCK_AND
  #define JUST_SOCK
  #define SOCK_AND

  #define B    0x500
  #define T    0x4800
  #define R    0x6800
  #define N    1
  #define SS   ((struct SockState*)(VARS_RAM+sizeof(struct vars)+sizeof(struct SockState)*N))

#endif

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

extern const byte BR_ADDR    [4];
extern const byte BR_MASK    [4];
extern const byte BR_GATEWAY [4];
extern const byte BR_RESOLV  [4];
extern const byte BR_WAITER  [4];

enum Commands {
  CMD_POKE = 0,
  CMD_LOG = 200,
  CMD_INKEY = 201,
  CMD_PUTCHARS = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_SP_PC = 205,
  CMD_REV = 206,
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

extern const char RevDate[16];
extern const char RevTime[16];
extern const byte HexAlphabet[];

byte WizGet1(word reg);
word WizGet2(word reg);
void WizGetN(word reg, void* buffer, word size);
void WizPut1(word reg, byte value);
void WizPut2(word reg, word value);
void WizPutN(word reg, const void* data, word size);
word WizTicks();
void WizReset();
void WizConfigure(const byte* ip_addr, const byte* ip_mask, const byte* ip_gateway);
void WizIssueCommand(PARAM_SOCK_AND byte cmd);
void WizWaitStatus(PARAM_SOCK_AND byte want);

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

// hyper

#if EMULATED
void PrintH(const char* format, ...);
#else
#define PrintH(FMT,...) /*nothing*/
#endif

// vanishing PrintF's

#define print1 if (VERBOSE>=1) PrintF
#define print2 if (VERBOSE>=2) PrintF
#define print3 if (VERBOSE>=3) PrintF
#define print4 if (VERBOSE>=4) PrintF
#define print5 if (VERBOSE>=5) PrintF
#define print6 if (VERBOSE>=6) PrintF
#define print7 if (VERBOSE>=7) PrintF
#define print8 if (VERBOSE>=8) PrintF
#define print9 if (VERBOSE>=9) PrintF

#endif // _FROB2_BOOTROM_BOOTROM_H_
