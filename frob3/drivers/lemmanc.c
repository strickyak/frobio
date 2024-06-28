/*** WARNING *** UNTESTED ***/

// lemc.c -- Lem file manager.
//
// 2023-09 Henry Strickland <github.com/strickyak>
//
// Copyright 2023 Henry Strickland (strickyak).  MIT License.

/*
   The MIT License (MIT)

Copyright (c) 2023 Henry Strickland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "frob3/drivers/lemmanc.h"

#include "frob3/os9/os9defs.h"

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

// "Lem_UNKNOWN_ZERO", // 0
// "Lem_Create",       // 1
// "Lem_Open",         // 2
// "Lem_MakDir",       // 3
// "Lem_ChgDir",       // 4
// "Lem_Delete",       // 5
// "Lem_Seek",         // 6
// "Lem_Read",         // 7
// "Lem_Write",        // 8
// "Lem_ReadLn",       // 9
// "Lem_WritLn",       // 10
// "Lem_GetStat",      // 11
// "Lem_SetStat",      // 12
// "Lem_Close",        // 13

// Three chars per name.
const char NAMES[] =
    "ZZZ\0Cre\0Ope\0Mkd\0Cgd\0Del\0Sek\0Rea\0Wri\0RLn\0WLn\0Get\0Set\0Clo";

/////////////////  Hypervisor Debugging Support

#define __GOMAR__ 1
#if __GOMAR__

#ifdef __GNUC__
void
#else
asm
#endif
HyperCoreDump() {
#ifdef __GNUC__
  asm volatile(
      "  nop\n"
      "  fcb 33\n"
      "  fcb 100"
      : /*out*/
      : /*in*/
      : /*clobbers*/);
#else
  asm {
    NOP
    FCB 33
    FCB 100  ; hyperCoreDump
  }
#endif
}

void PrintHH(const char* format, ...) {
  const char** fmt_ptr = &format;
#ifdef __GNUC__
  asm volatile(
      "  ldx %0\n"
      "  nop\n"
      "  fcb 33\n"
      "  fcb 100"
      : /*out*/
      : /*in*/ "m" (fmt_ptr)
      : /*clobbers*/ "x");
#else
  asm {
      ldx :fmt_ptr
      nop
      fcb 33   // BRN opcode.
      fcb 111  // PrintH2 in emu/hyper.go
  }
#endif
}

#else  // HYPER_GOMAR

#define HyperCoreDump() \
  {                     \
    while (1) {         \
    }                   \
  }
// #define PrintH(format, ...) {}

#endif  // HYPER_GOMAR

#ifdef __GNUC__
void
#else
asm
#endif
GameOver() {
#ifdef __GNUC__
  asm volatile("nop\nINF_LOOP bra INF_LOOP");
#else
  asm {
INF_LOOP bra INF_LOOP
  }
#endif
}

////////////////////////////////////////////////

void bzero(void* addr, size_t len) {
  char* p = (char*)addr;
  for (size_t i = 0; i < len; i++) {
    *p++ = 0;
  }
}

void* memcpy(void* dest, const void* src, word len) {
  for (byte i = 0; i < (byte)len; i++) {
    ((char*)dest)[i] = ((char*)src)[i];
  }
  return dest;
}

asm IrqDisable() {
  asm {
    orcc #IntMasks
    rts
  }
}

asm IrqEnable() {
  asm {
    andcc #^IntMasks
    rts
  }
}

errnum Os9SRqMem(word size, word* size_out, word* addr_out) {
  errnum err = OKAY;
  asm {
      pshs y,u
      ldd size
      swi2
      fcb F_SRQMEM
      tfr u,x
      puls y,u
      bcs SRQMEM_BAD
      stx [addr_out]
      std [size_out]
      bra SRQMEM_OK

SRQMEM_BAD
        stb err
SRQMEM_OK
  }
  return err;
}

errnum Os9SRtMem(word size, word addr) {
  errnum err = OKAY;
  asm {
      ldd size
      ldx addr
      pshs y,u
      tfr x,u
      swi2
      puls y,u
      fcb F_SRTMEM
      bcc SRTMEM_OK

      stb err
SRTMEM_OK
  }
  return err;
}

byte Os9UserTask() {
  byte task = 0;
  asm {
    LDX <D.Proc
    LDA P$Task,X
    STA task
  }
  return task;
}

byte Os9SystemTask() {
  byte task = 0;
  asm {
    LDX <D.SysPrc
    LDA P$Task,X
    STA task
  }
  return task;
}

#if 0
errnum Os9LoadByteFromTask(byte task, word addr, byte* out) {
  errnum err;
  asm {
    LDB task
    LDX addr
    PSHS Y,U
    SWI2
    FCB F$LDABX
    PULS Y,U
    STA [out]
    BCS LoadByteBad

    clrb
LoadByteBad
    stb err
  }
  return err;
}

errnum Os9StoreByteToTask(byte task, word addr, byte in) {
  errnum err;
  asm {
    LDA in
    LDB task
    LDX addr
    PSHS Y,U
    SWI2
    FCB F$STABX
    PULS Y,U
    BCS StoreByteBad

    clrb
StoreByteBad
    stb err
  }
  return err;
}
#endif

errnum Os9Move(word count, word src, byte srcMap, word dest, byte destMap) {
  // PrintHH("Os9Move c=%x s=%x/%x d=%x/%x\n", count, src, srcMap, dest,
  // destMap);

  errnum err;
  word dreg = ((word)srcMap << 8) | destMap;
  asm {
    PSHS Y,U

    LDD dest
    PSHS D    ;  temporarily push the U arg.

    LDD dreg
    LDX src
    LDY count

    PULS U    ; must change U last, because it was frame pointer for formal parameters.
    SWI2
    FCB F_MOVE
    PULS Y,U
    bcs MoveBad

    clrb
MoveBad
    stb err
  }
  return err;
}

//////////////////////////////////////////////////////////////////

byte WizGet1(word reg) {  // YAK
  WIZ->addr = reg;
  byte z = WIZ->data;
  return z;
}
word WizGet2(word reg) {
  WIZ->addr = reg;
  byte z_hi = WIZ->data;
  byte z_lo = WIZ->data;
  word z = ((word)(z_hi) << 8) + (word)z_lo;
  return z;
}
void WizGetN(word reg, void* buffer, word size) {
  byte* to = (byte*)buffer;
  WIZ->addr = reg;
  for (word i = size; i; i--) {
    *to++ = WIZ->data;
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
  const byte* from = (const byte*)data;
  WIZ->addr = reg;

  for (word i = size; i; i--) {
    WIZ->data = *from++;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void WizIssueCommand(byte cmd) {
  WizPut1(BASE + SK_CR, cmd);
  while (WizGet1(BASE + SK_CR)) {
  }
}

void WizWaitStatus(byte want) {
  byte status;
  do {
    status = WizGet1(BASE + SK_SR);
  } while (status != want);
}

struct proto {
  byte open_mode;
  byte open_status;
};
const struct proto TcpProto = {
    SK_MR_TCP + SK_MR_ND,  // TCP Protocol mode with No Delayed Ack.
    SK_SR_INIT,
};
const struct proto UdpProto = {
    SK_MR_UDP,  // UDP Protocol mode.
    SK_SR_UDP,
};

errnum TcpCheck() {
  byte ir = WizGet1(BASE + SK_IR);  // Socket Interrupt Register.
  if (ir & SK_IR_TOUT) {            // Timeout?
    return 246 /* ERROR: Not Ready */;
  }
  if (ir & SK_IR_DISC) {  // Disconnect?
    return 240 /* ERROR: Unit Error*/;
  }
  return 0;
}

errnum TcpRecvDataTry(char* buf, size_t n) {
  errnum e = TcpCheck();
  if (e) return e;

  word bytes_waiting = WizGet2(BASE + SK_RX_RSR0);  // Unread Received Size.
  word rd = WizGet2(BASE + SK_RX_RD0);
  word wr = WizGet2(BASE + SK_RX_WR0);

  word begin = rd & RING_MASK;  // begin: Beneath RING_SIZE.
  word end = begin + n;         // end: Sum may not be beneath RING_SIZE.
  // PrintHH("TTn=%x^*%x^r%x^w%x^b/%x^e/%x", n, bytes_waiting, rd, wr, begin,
  // end);

  if (bytes_waiting < n) return E_NOT_YET;

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizGetN(RX_RING + begin, buf, first_n);
    WizGetN(RX_RING, buf + first_n, second_n);
  } else {
    WizGetN(RX_RING + begin, buf, n);
  }

  WizPut2(BASE + SK_RX_RD0, rd + n);
  WizIssueCommand(SK_CR_RECV);  // Inform socket of changed SK_RX_RD.
  return OKAY;
}
errnum TcpRecvData(char* buf, size_t n) {
  errnum e;
  do {
    // PrintHH("TcpRecvData(%x,%x)...", buf, n);
    e = TcpRecvDataTry(buf, n);
    // PrintHH(" ->%d.", e);
  } while (e == E_NOT_YET);
  return e;
}

errnum WizReserveToSend(size_t n) {
  // PrintHH("ResTS %x", n);
  // Wait until free space is available.
  word free_size;
  do {
    // TcpCheck();
    free_size = WizGet2(BASE + SK_TX_FSR0);
    // PrintHH("Res free %x", free_size);
  } while (free_size < n);
  return OKAY;
}

errnum WizDataToSend(char* data, size_t n) {
  word begin = WizGet2(BASE + SK_TX_WR0) & RING_MASK;
  word end = begin + n;  // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(TX_RING + begin, data, first_n);
    WizPutN(TX_RING, data + first_n, second_n);
  } else {
    WizPutN(TX_RING + begin, data, n);
  }
  return OKAY;
}

errnum WizFinalizeSend(size_t n) {
  word tx_wr = WizGet2(BASE + SK_TX_WR0);
  tx_wr += n;
  WizPut2(BASE + SK_TX_WR0, tx_wr);
  byte send_command = SK_CR_SEND;
  WizIssueCommand(send_command);
  return OKAY;
}

errnum WizSend(char* data, size_t n) {
  errnum e = TcpCheck();
  if (e) return e;
  e = WizReserveToSend(n);
  if (e) return e;
  e = WizDataToSend(data, n);
  if (e) return e;
  e = WizFinalizeSend(n);
  return e;
}

#define PD_BUF 8  // offset from path_desc to buffer pointer

struct quint {
  byte command;
  word n;
  word p;
};

////////////////////////////////////////////////

struct req {
  struct quint quint;
  byte fileman_op;
  struct Regs regs;
  word payload_size;
  // Payload data follows this struct.
};

struct reply {
  struct quint quint;
  byte fileman_status;
  struct Regs regs;
  word payload_size;
  // Payload data follows this struct.
};

////////////////////////////////////////////////
///
///  GENERIC "C" FUNCTIONS: could be Daemon or Client.

#define MAX_PATH_LEN 255

errnum Bridge2(word fileman_op, word unused_x, struct PathDesc* pd,
               struct Regs* regs) {
  // pd->device_table_entry->dt_num_users = 16; // ARTIFICIALLY KEEP THIS OPEN.
  assert(pd->regs == regs);

  {
    word i = fileman_op << 2;

    PrintHH(
        "<><><> FILEMAN_OP=%s=%d, proc=%d pd=%x (count=%x pid=%x) dte=%x #u=%x "
        "(pg=%x po=%x l=%x bu=%x wk=%x) a=%x b=%x",

        NAMES + i, fileman_op, **(byte**)(0x50 /* D.Proc */), pd,
        pd->link_count, pd->current_process_id, pd->device_table_entry,
        pd->device_table_entry->dt_num_users,
        pd->device_table_entry->dt_device_vars->v_page,
        pd->device_table_entry->dt_device_vars->v_port,
        pd->device_table_entry->dt_device_vars->v_lprc,
        pd->device_table_entry->dt_device_vars->v_busy,
        pd->device_table_entry->dt_device_vars->v_wake, regs->ra, regs->rb);
  }

  struct req req;
  bzero((char*)&req, sizeof req);
  req.quint.command = CMD_LEMMAN_REQUEST;
  req.quint.p = (word)pd;
  req.fileman_op = (byte)fileman_op;
  req.payload_size = 0;
  memcpy((char*)&req.regs, (char*)regs, sizeof req.regs);
  word orig_rx = regs->rx;

  byte user_task = Os9UserTask();
  byte sys_task = Os9SystemTask();

  switch (req.fileman_op) {
    case 1:  // Create
    case 2:  // Open
    case 3:  // MakDir
    case 4:  // ChgDir
    case 5:  // Delete
    {
      if (!pd->buffer) {
        // -- errnum Os9SRqMem(word size, word* size_out, word* addr_out) --
        word size_out;
        errnum e = Os9SRqMem(256, &size_out, &pd->buffer);
        if (e) return e;
        if (size_out < 256) return E_NORAM;
        if (!pd->buffer) return E_NORAM;
      }

      req.payload_size = MAX_PATH_LEN;
      assert(orig_rx < 0xFE80U);  // dont allow overflow into IO page $FF
      {
        errnum e =
            Os9Move(MAX_PATH_LEN, orig_rx, user_task, pd->buffer, sys_task);
        if (e) return e;
      }

      {
        // Just Hyper Verbosity.
        // We borrow buffer at 192 for up to 60 chars.
        char* buf = (char*)pd->buffer;
        byte i = 0;
        for (; i < 60; i++) {
          buf[192 + i] = (buf[i] == '\r') ? 0 : buf[i];
        }
        buf[i] = 0;
        PrintHH("<><><> PATHNAME [%s]", buf + 192);
      }

      const char* buf = (const char*)pd->buffer;
      PrintHH("<><><> %x %x %x %x %x %x %x %x %x %x", buf[0], buf[1], buf[2],
              buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
    } break;
    case 8:   // Write
    case 10:  // WritLn
    {
      assert(regs->ry <= 256);  // dont overflow pd->buffer

      {
        errnum e = Os9Move(regs->ry, orig_rx, user_task, pd->buffer, sys_task);
        if (e) return e;
      }

      {
        word n = regs->ry;
        if (req.fileman_op == 10) {  // WritLn: stop at \r
          byte* bp = pd->buffer;
          for (word i = 0; i < n; i++) {
            if (*bp++ == '\r') {
              n = i + 1;
              break;
            }
          }
        }
        req.payload_size = n;
      }
    } break;
    case 12:  // SetStt
    {
      PrintHH("<><><> SetStt a=%x b=%x", regs->ra, regs->rb);
      // A = path; B = SetStt op

      switch (regs->rb) {
        default:
          return E_UNKSVC;
      }
    } break;
    case /*Close*/ 13: {
      PrintHH("<><><> Close a=%x count=%x", regs->ra, pd->link_count);
      if (!pd->link_count) return 0;
    } break;
    default: {
      req.payload_size = 0;
    } break;
  }

  req.quint.n = sizeof req - sizeof req.quint + req.payload_size;
  // -- errnum WizSend(char* data, size_t n) --
  {
    errnum e = WizSend((char*)&req, sizeof req);
    if (e) return e;
  }

  if (req.payload_size) {
    errnum e = WizSend((char*)pd->buffer, req.payload_size);
    if (e) return e;
  }

  // Expect a response.
  struct reply reply;
  // PrintHH("gonna Recv Reply");
  errnum e = TcpRecvData((char*)&reply, sizeof reply);
  if (e) return e;
  assert(reply.quint.n >= sizeof reply - sizeof reply.quint);
  // PrintHH("reply.quint.n = %d", reply.quint.n);

  if (reply.payload_size) {
    errnum e = TcpRecvData((char*)pd->buffer, reply.payload_size);
    if (e) return e;

    switch (fileman_op) {
      case /*Read*/ 7:
      case /*ReadLn*/ 9: {
        errnum e = Os9Move(reply.payload_size, pd->buffer, sys_task, orig_rx,
                           user_task);
        if (e) return e;
      } break;

      case /*GetStt*/ 11:
        PrintHH("<><><> GetStt a=%x b=%x", regs->ra, regs->rb);
        {
          // A = path; B = GetStt op
          switch (regs->rb) {
            case 0x20:  // SS.FDInf (file descriptor info)
            {
              errnum e = Os9Move(255 & regs->ry, pd->buffer, sys_task, orig_rx,
                                 user_task);
              if (e) return e;
            } break;
            default:
              return E_UNKSVC;  // unknown service code.
          }
        }
        break;

      case /*SetStt*/ 12: {
        return E_UNKSVC;  // unknown service code.
      } break;

      case /*Close*/ 13: {
        // -- errnum Os9SRtMem(word size, word addr) --
        if (pd->buffer) {
          errnum e = Os9SRtMem(256, pd->buffer);
          if (e) return e;
        }
      } break;
    }
  }

  memcpy((char*)regs, (const char*)&reply.regs, sizeof reply.regs);

  return reply.fileman_status;
}
errnum Bridge(word fileman_op, word unused_x, struct PathDesc* pd,
              struct Regs* regs) {
  errnum status = Bridge2(fileman_op, unused_x, pd, regs);
  PrintHH("<><><> == status == %x", status);
  return status;
}
