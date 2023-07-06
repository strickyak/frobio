#ifndef _FROB3_BOOTROM_BOOTROM_H_
#define _FROB3_BOOTROM_BOOTROM_H_

#define VERBOSE 3 // 9 // 6 // 3
#define PRINTK 1

#define X220_NET 0
#define LOCAL_NET 0
#define INTER_NET 1

#define BR_STATIC 0
#define BR_DHCP 1

#define TCP_CHUNK_SIZE 1024  // Chunk to send or recv in TCP.

#define WIZ_PORT  0xFF68   // Hardware port.
#define WAITER_TCP_PORT  2319   // w# a# i#

#define CASBUF 0x01DA // Rock the CASBUF, rock the CASBUF!

// Conventional types and constants for Frobio
typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1

typedef void (*func)();

#include "frob3/wiz/w5100s_defs.h"

//////////////////////////////////////////////////
//
//  Compiler Specific:

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

struct sock_vars {
  word tx_ptr;
  word tx_to_go;
};

struct vars {
    struct RomApi3 *rom_api_3;
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
    byte ip_dhcp[4];
    byte mask_num;
    word waiter_port;
    byte xid[4];    // Temporary packet id in DHCP

    word vdg_addr;   // mapped via SAM
    word vdg_begin;  // begin useable portion
    word vdg_end;    // end usable portion
    word vdg_ptr;    // inside usable portion

    char line_buf[64];
    char* line_ptr;
    bool need_dhcp;  // from command to DHCP.

    struct sock_vars sock_vars[4];
};

#define VARS_MAGIC 0x0756 /* ^G V global vars */

extern const char SixFFs[6];
extern const char Eight00s[8];

#define VARS_RAM (CASBUF)
#define Vars ((struct vars*)VARS_RAM)
#define WIZ  (Vars->wiz_port)

#define VDG_RAM  0x0400  // default 32x16 64-char screen
#define VDG_END  0x0600

// This was an indication of waiting in loops.
// #define LIVENESS(N) {++ *(byte*)(VDG_RAM+N);}

struct sock {
    word base;
    word tx_ring;
    word rx_ring;
    byte nth;
};
extern const struct sock Socks[4];

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
#define SV   (Vars->sock_vars + sockp->nth)
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
#if 0
extern const char ClassAMask[4];
extern const char ClassBMask[4];
extern const char ClassCMask[4];
#endif

struct UdpRecvHeader {
    byte addr[4];
    word port;
    word len;
};

extern const byte HexAlphabet[];

byte WizGet1(word reg);
word WizGet2(word reg);
void WizGetN(word reg, void* buffer, word size);
void WizPut1(word reg, byte value);
void WizPut2(word reg, word value);
void WizPutN(word reg, const void* data, word size);
word WizTicks();
void WizReset();
void WizConfigure();
void WizIssueCommand(PARAM_SOCK_AND byte cmd);
void WizWaitStatus(PARAM_SOCK_AND byte want);

void WizOpen(PARAM_SOCK_AND const struct proto* proto, word local_port );
void TcpDial(PARAM_SOCK_AND const byte* host, word port);
void TcpEstablish(PARAM_JUST_SOCK);
errnum WizCheck(PARAM_JUST_SOCK);
errnum WizRecvGetBytesWaiting(PARAM_SOCK_AND word* bytes_waiting_out);
void WizReserveToSend(PARAM_SOCK_AND  size_t n);
void WizDataToSend(PARAM_SOCK_AND const char* data, size_t n);
void WizBytesToSend(PARAM_SOCK_AND const byte* data, size_t n);
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

// hyper

#if __GOMAR__
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

#endif // _FROB3_BOOTROM_BOOTROM_H_
