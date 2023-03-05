#ifndef _FROB2_BOOTROM_BOOTROM_H_
#define _FROB2_BOOTROM_BOOTROM_H_

#define VERBOSE 3

#define X220NET 1
#define LOCALNET 0

#define BR_STATIC 1
#define BR_DHCP 0

#define MULTI_SOCK 1
#define CHECKSUMS 0

#define PRINTK 1

#define WIZ_PORT  0xFF68   // Hardware port.

#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!

#if 0
#  define TFTPD_SERVER_PORT 69
#  define BR_BOOTFILE "COCOIO.BOOT"  // in DECB LOADM format
#  define BR_NAME "COCO" // 4-letter name is used for MAC and DHCP
#endif

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

extern void memcpy(char* a, const char* b, int c);
extern void memset(char* a, int b, int c);
extern void strcpy(char* a, const char*b);
extern void strncpy(char* a, const char*b, int n);
extern int strlen(const char* s);

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
  byte addr_hi;
  byte addr_lo;
  byte data;
};

#define WIZ  ((volatile struct wiz_port*)WIZ_PORT)


struct vars {
    byte hostname[8];
    byte mac_addr[6];
    byte ip_addr[4];
    byte ip_mask[4];
    byte ip_gateway[4];
    byte ip_resolver[4];
    byte ip_waiter[4];
    char waiter_domain[32];
    word flags;     // might tell us which fields above to use.
    byte hostchar;  // might tell us what config to load or remote-load.

    word vdg_ptr;

#if 0
    struct loader {
      byte state;
      byte op;
      word counter;
      word addr;
    } loader;
#endif

    // TODO -- get rid of these:
#if 0
#if WANT_PACKIN_PACKOUT
    byte packin[200];
    byte packout[200];
#endif
#endif

};
// Four of SockState immediately follow struct vars,
// but due to const initialization rules, we cannot
// make them part of the struct.
struct SockState {
  word tx_ptr;
  word tx_to_go;
};

#define VARS_RAM (CASBUF)
#define Vars ((struct vars*)VARS_RAM)

#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_LEN  0x0200
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

extern const byte HexAlphabet[];

// common.c

word StackPointer();
char PolCat(); // Return one INKEY char, or 0, with BASIC `POLCAT` subroutine.
void Delay(word n);

// checksum.c
void Checksum();

// printk.c

void PutChar(char ch);
void PutStr(const char* s);
void PutHex(word x);
void PutDec(word x);
void Fatal(const char* wut, word arg);
void ShowLine(word line);
void printk(const char* format, ...);
void Line(const char* s);
void AssertEQ(word a, word b);
void AssertLE(word a, word b);

#define print1 if (VERBOSE>=1) printk
#define print2 if (VERBOSE>=2) printk
#define print3 if (VERBOSE>=3) printk
#define print4 if (VERBOSE>=4) printk
#define print5 if (VERBOSE>=5) printk
#define print6 if (VERBOSE>=6) printk
#define print7 if (VERBOSE>=7) printk
#define print8 if (VERBOSE>=8) printk
#define print9 if (VERBOSE>=9) printk

#if VERBOSE >= 6
#  define L ShowLine(__LINE__);
#else
#  define L /**/
#endif

#endif // _FROB2_BOOTROM_BOOTROM_H_
