#ifndef _FROB3_DRIVERS_LEMMANC_H_
#define _FROB3_DRIVERS_LEMMANC_H_

#define VERBOSE 3 // 9 // 6 // 3
#define EMULATED 0

#define WIZ_PORT  0xFF68   // Hardware port.

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

#include "frob3/wiz/w5100s_defs.h"

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

#if EMULATED
// HyperLogging with Emulator's PrintH command.
void PrintH(const char* format, ...);
#else
// These turn to nothing, if not emulated.
#define PrintH(FMT,...) /*nothing*/
#endif

#endif // _FROB3_DRIVERS_LEMMANC_H_
