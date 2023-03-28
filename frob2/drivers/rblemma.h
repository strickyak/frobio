#ifndef _FROB2_DRIVERS_RBLEMMA_H_
#define _FROB2_DRIVERS_RBLEMMA_H_

#define VERBOSE 3 // 9 // 6 // 3
#define EMULATED 1

#define X220NET 0
#define LOCALNET 1

#define BR_STATIC 1
#define BR_DHCP 0

#define CHECKSUMS 0

#define WIZ_PORT  0xFF68   // Hardware port.

#define printk PrintH

// Conventional types and constants for Frobio
typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define E_NOT_YET (errnum)1

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

struct wiz_ports {
  byte command;
  word addr;
  byte data;
};

#define WIZ  ((volatile struct wiz_ports*)WIZ_PORT)
#define BASE    0x500
#define TX_RING    0x4800
#define RX_RING    0x6800

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

void PutChar(char ch);
void PutStr(const char* s);
void PutHex(word x);
void PutDec(word x);
void Fatal(const char* wut, word arg);
void ShowLine(word line);
void Line(const char* s);
void AssertEQ(word a, word b);
void AssertLE(word a, word b);

#if EMULATED
// HyperLogging with Emulator's PrintH command.
void PrintH(const char* format, ...);
#else
// These turn to nothing, if not emulated.
#define PrintH(FMT,...) /*nothing*/
#endif

// vanishing PrintH's

#define print1 if (VERBOSE>=1) PrintH
#define print2 if (VERBOSE>=2) PrintH
#define print3 if (VERBOSE>=3) PrintH
#define print4 if (VERBOSE>=4) PrintH
#define print5 if (VERBOSE>=5) PrintH
#define print6 if (VERBOSE>=6) PrintH
#define print7 if (VERBOSE>=7) PrintH
#define print8 if (VERBOSE>=8) PrintH
#define print9 if (VERBOSE>=9) PrintH

#if VERBOSE >= 6
#  define L ShowLine(__LINE__);
#else
#  define L /**/
#endif

#endif // _FROB2_DRIVERS_RBLEMMA_H_
