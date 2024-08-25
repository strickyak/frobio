#ifndef _FROB3_DRIVERS_LEMMANC_H_
#define _FROB3_DRIVERS_LEMMANC_H_

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

//////////////////////////////////////////////
//
// Specific to Lemma and LemMan and File Manangers

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define assert(C)                                                \
  {                                                              \
    if (!(C)) {                                                  \
      PrintHH(" *ASSERT* %s:%d *FAILED*\n", __FILE__, __LINE__); \
      HyperCoreDump();                                           \
      GameOver();                                                \
    }                                                            \
  }

enum Commands {
  CMD_POKE = 0,
  CMD_LOG = 200,
  CMD_INKEY = 201,
  CMD_PUTCHARS = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_SP_PC = 205,
  CMD_REV = 206,
  CMD_BLOCK_READ = 207,   // block device
  CMD_BLOCK_WRITE = 208,  // block device
  CMD_BLOCK_ERROR = 209,  // nack
  CMD_BLOCK_OKAY = 210,   // ack
  CMD_BOOT_BEGIN = 211,   // boot_lemma
  CMD_BOOT_CHUNK = 212,   // boot_lemma
  CMD_BOOT_END = 213,     // boot_lemma
  CMD_LEMMAN_REQUEST = 214,
  CMD_LEMMAN_REPLY = 215,
  CMD_JSR = 255,
};

struct DeviceTableEntry {
  word dt_device_driver_module;       // F$DRIV
  struct DeviceVars* dt_device_vars;  // F$STAT
  word dt_device_desc;                // F$DESC
  word dt_fileman;                    // V$FMGR
  byte dt_num_users;                  // V$USRS
  word dt_drivex;                     // V$DRIVX
  word dt_fmgrex;                     // V$FMGREX
};

struct DeviceVars {
  // Pre-Defined: 6 bytes.
  byte v_page;  // extended port addr
  word v_port;  // base port addr // REUSE FOR ALL64 BASE ADDR
  byte v_lprc;  // Last Active Process Id (not used?)
  byte v_busy;  // Active process ID (0 == not busy)
  byte v_wake;  // Process ID to wake after command completed
  // Actually a full page of 256 bytes will be alloc'ed.
  // So feel free to add more fields here.
};

struct Regs {  // n.b. 6809 not 6309 !?
  byte rcc, ra, rb, rdp;
  word rx, ry, ru, rpc;
};  // size 12
#define REGS_D(regs) (*(word*)(&(regs)->ra))

struct PathDesc {
  byte path_num;                                // PD.PD = 0
  byte mode;                                    // PD.MOD = 1
  byte link_count;                              // PD.CNT = 2
  struct DeviceTableEntry* device_table_entry;  // PD.DEV = 3
  byte current_process_id;                      // PD.CPR = 5
  struct Regs* regs;                            // PD.RGS = 6
  word buffer;                                  // PD.BUF = 8
  // offset 10 = PD.FST

  bool is_poisoned;  // TODO: error handling and path poisoning.
};                   // PathDesc must be 32 bytes or under in size.

#endif // _FROB3_DRIVERS_LEMMANC_H_
