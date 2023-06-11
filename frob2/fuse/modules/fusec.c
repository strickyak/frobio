// fusec.c -- FUSE file manager.
//
// Copyright 2023 Henry Strickland (strickyak).  MIT License.
//
// This is CMOC source for a Fuse File Manager for NitrOS9 Level II.
// This is a Work In Progress, but basic functionaly works as long as
// things are small (use well under 256 bytes per message) and one
// thing happens at a time.
//
// ( See https://en.wikipedia.org/wiki/Filesystem_in_Userspace )
//
// This compiles with CMOC to assembly, and is then included in fuseman.asm
// as the file _generated_from_fusec_.a
// There is also fuser.asm (the driver) and fuse.asm (the device descriptor).
//
// This won't do much alone.  You need one or more specially-writen daemons,
// which are user mode processes that function as back-end services to implement
// virtual devices for Fuse.  The clients use the normal OS9 I$... filesystem
// operations to communicate with Fuse.
//
// cf. https://en.wikipedia.org/wiki/Filesystem_in_Userspace

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

#define HYPER_GOMAR 0

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
typedef unsigned int size_t;

#include "frob2/os9/os9defs.h"
#include "frob2/fuse/fuse.h"

// Porability Warning -- This is specific to the Coco3 DAT Tables.
#define DAT_MAP_NUM_WORDS 8 // in the mapping for one process.

#define OKAY 0
#define TRUE 1
#define FALSE 0
#define NULL ((void*)0)

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define assert(C) { if (!(C)) { PrintH(" *ASSERT* %s:%d *FAILED*\n", __FILE__,  __LINE__); HyperCoreDump(); GameOver(); } }

struct DeviceTableEntry {
  word dt_device_driver_module;       // F$DRIV
  struct DeviceVars *dt_device_vars;  // F$STAT
  word dt_device_desc;                // F$DESC
  word dt_fileman;   // V$FMGR
  byte dt_num_users; // V$USRS
  word dt_drivex;    // V$DRIVX
  word dt_fmgrex;    // V$FMGREX
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

struct Regs {
  byte rcc, ra, rb, rdp;
  word rx, ry, ru, rpc;
};  // size 12
#define REGS_D(regs) (*(word*)(&(regs)->ra))

// Client will store these on the kernel mode stack
// while the Daemon is responding to its request.
struct ClientTmp {
  byte task_num; // Temporary num to access Client space.
  word task_map[DAT_MAP_NUM_WORDS]; // How to access Client space.
  byte client_op;
  word opening_original_rx;
  word opening_pathname_size;
  struct FuseReply reply;
};

// Special arg for Client OP_CREATE & OP_OPEN.
struct Opening {
  word begin1;  // second word in pathname
  word end1;
  word begin2;  // third word in pathname
  word end2;
  word original_rx;
};

struct PathDesc {
  byte path_num;              // PD.PD = 0
  byte mode;                  // PD.MOD = 1
  byte link_count;            // PD.CNT = 2
  struct DeviceTableEntry*
        device_table_entry;   // PD.DEV = 3
  byte current_process_id;    // PD.CPR = 5
  struct Regs *regs;          // PD.RGS = 6
  word unused_daemon_buffer;  // PD.BUF = 8
  // offset 10 = PD.FST
  bool is_daemon;
  bool paused_proc_id;  // 0 if not paused.
  struct PathDesc* peer;  // Daemon's client or Client's daemon.
  struct ClientTmp* client_tmp;  // Points into Client's kernel stack.
  bool is_poisoned;  // TODO: error handling and path poisoning.
}; // Must be 32 bytes or under in size.

#define DAEMON_NAME_OFFSET_IN_PD 33
#define DAEMON_NAME_MAX 28

/////////////////  Hypervisor Debugging Support

#if HYPER_GOMAR

#ifdef __GNUC__
void
#else
asm
#endif
HyperCoreDump() {
#ifdef __GNUC__
  asm volatile (
      "  swi\n"
      "  fcb 100"
      : /*out*/
      : /*in*/
      : /*clobbers*/);
#else
  asm {
    SWI
    FCB 100  ; hyperCoreDump
  }
#endif
}

void PrintH(const char* format, ...) {
#if __CMOC__
  asm {
      swi
      fcb 108
  }
#endif
}

#else // HYPER_GOMAR

#define HyperCoreDump() {while (1) {}}
#define PrintH(format, ...) {}

#endif // HYPER_GOMAR


#ifdef __GNUC__
void
#else
asm
#endif
GameOver() {
#ifdef __GNUC__
  asm volatile ("nop\nINF_LOOP bra INF_LOOP");
#else
  asm {
INF_LOOP bra INF_LOOP
  }
#endif
}

////////////////////////////////////////////////

void bzero(void* addr, size_t len) {
  char* p = (char*) addr;
  for (size_t i=0; i<len; i++) {
    *p++ = 0;
  }
}

byte ToUpper(byte b) {
  b = b&127; // remove high bit
  if ('a'<= b && b<='z') return b-32;
  return b;
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

byte* Os9PathDescBaseTable() {
  byte* base;
  asm {
    LDD <D.PthDBT
    STD base
  }
  return base;
}

word Os9CurrentProcAddr() {
  word addr = 0;
  asm {
    LDX <D.Proc
    STX addr
  }
  return addr;
}

byte Os9CurrentProcessId() {
  byte id = 0;
  asm {
    LDX <D.Proc
    LDA P$ID,X
    STA id
  }
  return id;
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

errnum Os9ReserveTask(byte* task_num_out) {
  errnum err = 0;
  asm {
    os9 F$ResTsk
    bcc RESTSK_OK

    stb err
    bra RESTSK_END

RESTSK_OK
    stb [task_num_out]
RESTSK_END
  }
  return err;
}

errnum Os9ReleaseTask(byte task_num) {
  errnum err = 0;
  asm {
      ldb task_num
      os9 F$RelTsk
      bcc RELTSK_OK
      stb err
RELTSK_OK
  }
  return err;
}

void Os9SetDatMapForTask(byte task_num, word* mapping) {
  PrintH("SET DAT FOR TASK %x <- %x\n", task_num, mapping);
  asm {
    LDB task_num
    LDX <D.TskIPt ; X -> task table
    ABX
    ABX           ; X -> correct task entry

    LDD mapping
    STD ,X        ; set the mapping at X
  }
}

void Os9CopyCurrentUserDatMapTo(word* dest) {
  word* current = 0;
  asm {
    LDX <D.Proc   ; current user proc
    LDB P$Task,X  ; its task num
    LDX <D.TskIPt ; task table
    ABX
    ABX

    LDD ,X       ; same MAX(B)=63 assumption as rb1773 driver.
    STD current
  }
  for (byte i = 0; i < DAT_MAP_NUM_WORDS; i++) {
    *dest++ = *current++;
  }
}

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

errnum Os9Move(word count, word src, byte srcMap, word dest, byte destMap) {
  PrintH("Os9Move c=%x s=%x/%x d=%x/%x\n", count, src, srcMap, dest, destMap);

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

errnum Os9Send(byte to_pid, byte signal) {
  PrintH(" Send from=%x to=%x signal=%x\n", Os9CurrentProcessId(), to_pid, signal);
  errnum err;
  asm {
    PSHS Y,U       ; save frame
    lda to_pid
    ldb signal
    SWI2
    FCB F$Send
    PULS Y,U       ; restore frame
    BCS SendBad

    CLRB
SendBad
    stb err
  }
  PrintH(" Sent...err=%x\n", err);
  return err;
}

void Os9AwakenIOQN(struct PathDesc* pd) {
  errnum err;
  byte to_pid;
  asm {
    PSHS Y,U       ; save frame

    ldx <D.Proc
    lda P$IOQN,x
    sta to_pid
    beq NoOneToWake

    ldb #1         ; Signal 1 is Wake Up
    os9 F$Send

NoOneToWake
    clra
    clrb
    PULS Y,U       ; restore frame
  }
  if (to_pid) {
    PrintH("Os9AwakenIOQN: Sent wakeup from=%x to=%x\n", Os9CurrentProcessId(), to_pid);
  } else {
    PrintH("Os9AwakenIOQN: Zero PD.CPR.");
  }
}

errnum Os9Awaken(struct PathDesc* pd) {
  byte to_pid = pd->paused_proc_id;
  assert(to_pid > 0); // TODO: no need to wake, if 0.
  return Os9Send(to_pid, 1);  // Wakeup Signal.
}

errnum Os9Sleep(word num_ticks) {
  PrintH("Sleep(pid=%x,n=%x)\n", Os9CurrentProcessId(), num_ticks);

  errnum err;
  asm {
    PSHS Y,U       ; save frame
    ldx num_ticks
    SWI2
    FCB F$Sleep
    PULS Y,U       ; restore frame
    BCS SleepBad

    CLRB
SleepBad
    stb err
  }
  PrintH("UnSleep(pid=%x,n=%x) =>err=%x\n", Os9CurrentProcessId(), num_ticks, err);
  return err;
}

void Os9Pause(struct PathDesc* pd) {
  PrintH(" Pausing path=%x\n", pd->path_num);
  pd->paused_proc_id = Os9CurrentProcessId();
  PrintH(" paused_proc_id=%x current_process_id=%x\n", 
                 pd->paused_proc_id ,  pd->current_process_id);
  Os9Sleep(0);  // until signalled.
  pd->paused_proc_id = 0;
  PrintH(" Un-Paused path=%x\n", pd->path_num);
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

// 64-byte block routines.
errnum Os9All64(word base, word* base_out, word* block_addr, byte* block_num) {
    errnum err = OKAY;
    asm {
        pshs y,u
        ldx base

        swi2
        fcb F_ALL64
        bcs ALL64BAD

        stx [base_out]
        sty [block_addr]
        sta [block_num]
        bra ALL64OK

ALL64BAD
        stb err
ALL64OK
        puls y,u
    }
    return err;
}

errnum Os9Find64(byte block_num, word base, word* block_addr) {
    errnum err = OKAY;
    asm {
        pshs y,u
        lda block_num
        ldx base

        swi2
        fcb F_FIND64
        bcs FIND64BAD

        sty [block_addr]
        bra FIND64OK

FIND64BAD
        stb err
FIND64OK
        puls y,u
    }
    return err;
}

errnum Os9Ret64(byte block_num, word base) {
    errnum err = OKAY;
    asm {
        pshs y,u
        lda block_num
        ldx base

        swi2
        fcb F_RET64
        bcc RET64OK

        stb err
RET64OK
        puls y,u
    }
    return err;
}

void memcpy(char* dest, const char* src, word len) {
  asm {
    pshs d,x,y,u
  }
  for (byte i=0; i<(byte)len; i++) {
    dest[i] = src[i];
  }
  asm {
    puls d,x,y,u
  }
  return;
  asm {
memcpy bra _memcpy
  }
}

errnum Os9PrsNam(word ptr, word* eow_out, word*next_name_out) {
  errnum err;
  asm {
      LDX ptr
      LEAS -6,S  // room for post-U (8,S) post-D (6,S), post-Y (+4,S)
      PSHS Y,U   // after pre-Y (2,S) and pre-U (0,S)
                 //
      SWI2
      FCB $10 ; F$PrsNam

      STY 4,S
*     STD 6,S * not needed
*     STU 8,S * not needed
      PULS U,Y

      STX [eow_out]         ; post-X
      LDX 0,S               ; post-Y
      STX [next_name_out]

      LEAS 6,S
      BCS PrsNamBad    ; D & Carry still available.
      CLRB    ; OK status
PrsNamBad
      STB err
  }
  return err;
}

////////////////////////////////////////////////

errnum CopyParsedName(word begin, word end, char* dest, word max_len) {
  // max_len counts null termination.
  assert (end-begin < max_len-1);
  byte task = Os9UserTask();
  byte i = 0;
  for (word p = begin; p < end; p++) {
    byte ch = 0;
    errnum err = Os9LoadByteFromTask(task, p, &ch);
    if (err) return err;
    *dest++ = ToUpper(ch);
  }
  *dest = 0; // terminate
  return OKAY;
}

void ShowParsedName(word current, word begin, word end) {
  byte task = Os9UserTask();
  for (word p = current; p <= end; p++ ) {
    byte ch = 0;
    errnum err = Os9LoadByteFromTask(task, p, &ch);
    assert(!err);
  }
}

// Ignoring case is assumed, when Parsed Name is used.
// The string from begin to end is in the current
// process's task, and may contain high bits.
// The other string s is an upper-case 0-terminated
// C string, in normal memory, with no high bits.
bool ParsedNameEquals(word begin, word end, const char*s) {
  PrintH("ParsedNameEquals(b=%x, e=%x, s=%x)\n", begin, end, s);
  byte task = Os9UserTask();
  word p = begin;
  for (; *s && p < end; p++, s++) {
    byte ch = 0;
    errnum err = Os9LoadByteFromTask(task, p, &ch);
    assert(!err);

    if (ToUpper(ch) != *s) {
      PrintH("  ->FALSE");
      return FALSE;  // does not match.
    }
  }

  // If both termination conditions are true,
  // strings are equal.
  bool z = (p==end) && ((*s)==0);
  PrintH("  ->%x (%x,%x)", z, (p==end) ,((*s)==0));
  return z;
}

// FindDaemon traverses all path descriptors looking for
// one that is (1) open (path_num is set),
// (2) has the given device_table_entry (it is a /fuse),
// (3) is a daemon path (the parent_pd is null),
// (4) has the name indicated by begin/end.
struct PathDesc* FindDaemon(struct DeviceTableEntry* dte, word begin, word end) {
  struct PathDesc* got = NULL;
  byte* table = Os9PathDescBaseTable();
  for (byte i = 0; i < 64; i++) {  // how big can it get?
    byte page = table[i];
    if (!page) continue;
    word addr = (word)page << 8;
    for (byte j = 0; j<4; j++) {
      if (i!=0 || j!=0) {
        struct PathDesc* pd = (struct PathDesc*)addr;
        if (pd->path_num && pd->device_table_entry == dte) {
          assert(pd->path_num == 4*i+j);

          if (pd->is_daemon) {
            if (ParsedNameEquals(begin, end, (char*)pd + DAEMON_NAME_OFFSET_IN_PD)) {
              got = pd; // return pd;
              goto GOT_IT;
            }
          }
        }
      }
      addr += 64;
    }
  }
  // PrintH("FindDaemon => NOT FOUND\n");
  return NULL;

GOT_IT:
  // PrintH("FindDaemon => got\n");
  return got; // NULL;
}

void SwitchProcess(struct PathDesc* from, struct PathDesc* to) {
  byte timeout = 50;
  while (to->paused_proc_id == 0) {
    PrintH("SwitchProcess: sleeping 1 tick: try %x", timeout);
    Os9Sleep(1);
    --timeout;
    assert(timeout > 0);
  }
  // Awaken the "to" process.
  Os9Awaken(to);
  // Now go to sleep.
  Os9Pause(from);
}

void CheckClient(struct PathDesc* cli) {
  assert(cli);
  assert(((word)cli & 63) == 0);
  assert(!cli->is_daemon);
}

void CheckDaemon(struct PathDesc* dae) {
  assert(dae);
  assert(((word)dae & 63) == 0);
  assert(dae->is_daemon);
}

void SetPeer(struct PathDesc* subject, struct PathDesc* object) {
  assert(subject->peer == 0);
  object->link_count++;
  subject->peer = object;
}

void CheckPeer(struct PathDesc* subject, struct PathDesc* object) {
  assert(subject->peer == object);
}

void ClearPeer(struct PathDesc* subject, struct PathDesc* object) {
  assert(subject->peer == object);
  object->link_count--;
  subject->peer = NULL;
}

///////////  DAEMON  ///////////////////////////

errnum OpenDaemon(struct PathDesc* dae, word begin2, word end2) {
  PrintH("OpenDeamon dae=%x entry\n", dae);

  // Must not already be a daemon on this name.
  struct PathDesc *already = FindDaemon(dae->device_table_entry, begin2, end2);
  if (already) {
    PrintH("\nBAD: OpenDeamon already open => err %x\n", E_SHARE);
    return E_SHARE;
  }

  dae->is_daemon = 1;
  dae->peer = NULL;
  dae->current_process_id = Os9CurrentProcessId();

  CopyParsedName(begin2, end2, (char*)dae + DAEMON_NAME_OFFSET_IN_PD, DAEMON_NAME_MAX);

  PrintH("OpenDaemon OK: dae=%x %s\n", dae, (char*)dae + DAEMON_NAME_OFFSET_IN_PD);
  dae->current_process_id = 0; // PD.CPR: Nobody owns me.
  return OKAY;
}

// The Daemon process has called Read to get the next operation
// being called by a Client process.
errnum DaemonReadC(struct PathDesc* dae) {
  assert(dae);
  dae->current_process_id = Os9CurrentProcessId();
  errnum err = OKAY;

  // PAUSE FOR REQUEST FROM CLIENT.
  dae->paused_proc_id = Os9CurrentProcessId();
  PrintH("DaemonReadC: Pausing.\n");
  Os9Sleep(0);
  // DAEMAN HAS WOKEN UP.
  PrintH("DaemonReadC: UnPaused.\n");
  dae->paused_proc_id = 0;
  byte dtask = Os9UserTask();
  byte stask = Os9SystemTask();

  struct PathDesc* cli = dae->peer;
  CheckClient(cli);
  CheckPeer(cli, dae);
  struct ClientTmp *t = cli->client_tmp;
  PrintH("ClientTmp: t-> task=%x op=%x orig_rx=%x pn_size=%x",
       t->task_num, t->client_op, t->opening_original_rx, t->opening_pathname_size);

  struct FuseRequest req = {
    /*operation*/ t->client_op,
    /*path_num*/ cli->path_num,
    /*a_reg*/ cli->regs->ra,
    /*b_reg*/ cli->regs->rb,
    /*size*/ 0,
  };

  switch (t->client_op) {
    case OP_CREATE:
    case OP_OPEN:
      {
        assert(dae->regs->ry > sizeof req);
        word sz = MIN(t->opening_pathname_size, dae->regs->ry - sizeof req);
      req.size = sz;
      dae->regs->ry = sz + sizeof req;
        // Copy the pathname, sz bytes.
        err = Os9Move(sz,
            t->opening_original_rx, t->task_num,
            dae->regs->rx + sizeof req, dtask);
        if (err) goto FINISH;
      }
      break;

    case OP_CLOSE:
      dae->regs->ry = sizeof req;
      break;

    case OP_READ:
    case OP_READLN:
      dae->regs->ry = sizeof req;
      req.size = cli->regs->ry;
      break;

    case OP_WRITE:
    case OP_WRITLN:
      {
        assert(dae->regs->ry > sizeof req);
        word sz = MIN(cli->regs->ry, dae->regs->ry - sizeof req);
        req.size = sz;
        dae->regs->ry = req.size + sizeof req;
        // Copy the payload, sz bytes.
        err = Os9Move(sz, cli->regs->rx, t->task_num,
            dae->regs->rx + sizeof req, dtask);
        if (err) goto FINISH;
      }
      break;
    default:
      dae->regs->ry = sizeof req;
      break;
  }

  // Copy the request.
  err = Os9Move(sizeof req, (word)(&req), stask,
            dae->regs->rx, dtask);

FINISH:
  dae->current_process_id = 0; // PD.CPR: Nobody owns me.
  PrintH("DaemonReadC returns");
  return err;
}

// The Daemon process has called Write to return the results
// of the operation called by the Client process.
errnum DaemonWriteC(struct PathDesc* dae) {
  PrintH("DaemonWriteC: dae=%x ", dae);
  CheckDaemon(dae);
  struct PathDesc* cli = dae->peer;
  CheckClient(cli);
  dae->current_process_id = Os9CurrentProcessId();
  CheckPeer(cli, dae);

  errnum err = OKAY;
  // TODO: Poison on failure, and check for poison.
  // TODO: check a sequence number between cli & dae,
  // to make sure neither has died and been recycled,
  // or else poison the device.

  struct ClientTmp *t = cli->client_tmp;

  byte dtask = Os9UserTask();
  byte stask = Os9SystemTask();
  PrintH("DaemonWriteC: tmp=%x dtask=%x stask=%x", t, dtask, stask);
  err = Os9Move(sizeof t->reply,
      dae->regs->rx, dtask,
      (word)(&t->reply), stask);
  assert(!err);

  PrintH("DaemonWriteC: reply.status=%x client_op=%x", t->reply.status, t->client_op);
  if (t->reply.status) {
    err = t->reply.status;

  } else switch (t->client_op) {

    case OP_CREATE:
    case OP_OPEN:
    case OP_CLOSE:
      break;

    case OP_READ:
    case OP_READLN:
      word sz = MIN(t->reply.size, cli->regs->ry);
      cli->regs->ry = sz;
      dae->regs->ry = sz;
      err = Os9Move(sz,
          dae->regs->rx + sizeof t->reply, dtask,
          cli->regs->rx, t->task_num);
      break;

    case OP_WRITE:
    case OP_WRITLN:
      cli->regs->ry = t->reply.size;
      break;
  };

  // Allow other clients to use the daemon.
  // But the client still remembers its daemon.
  ClearPeer(dae, cli);
  dae->current_process_id = 0; // PD.CPR: Nobody owns me.

  // Time for the client to wake up.
  Os9Awaken(cli);
  Os9AwakenIOQN(cli);

  PrintH("DaemonWriteC: return OKAY");
  return OKAY;
}

/////////// CLIENT ////////////////////////

errnum ClientOperationC(struct PathDesc* cli, byte op, struct Opening* opening) {
  cli->current_process_id = Os9CurrentProcessId();
  cli->is_daemon = 0;

  struct PathDesc *dae = NULL;

  if (opening) {
    // Daemon must already be open.
    dae = FindDaemon(cli->device_table_entry, opening->begin1, opening->end1);
    if (!dae) {
      PrintH("BAD: daemon not open yet => err %x\n", E_SHARE);
      return E_SHARE;
    }
    assert(dae->is_daemon);
    PrintH("ClOp: found daemon %x", dae);

    SetPeer(cli, dae);  // Client links to Daemon.
  } else {
    dae = cli->peer;
    CheckDaemon(dae);
    CheckPeer(cli, dae);  // Client is already linked to Daemon.
  }
  SetPeer(dae, cli);  // Daemon links to Client during the client operation.
  PrintH("ClOp op=%x cli=%x dae=%x entry\n", op, cli, dae);



  // We are counting on this variable being on the Client's stack
  // throughout the operation.  It will hold a copy of the Client's
  // DAT MAP in a temporary task, and be used for communicating
  // requests and replies to the deamon process.
  struct ClientTmp tmp;
  cli->client_tmp = &tmp;  // so daemon can find it.

  // Allocate a temporary task num.
  errnum e = Os9ReserveTask(&tmp.task_num);
  PrintH("\nReserveTask -> Task# %x (e %x)\n", tmp.task_num, e);
  if (e) return e;

  Os9CopyCurrentUserDatMapTo(tmp.task_map);
  PrintH("ClOp Task=%x Map@%x = %x %x  %x %x  %x %x  %x %x\n",
      tmp.task_num,
      tmp.task_map,
      tmp.task_map[0],
      tmp.task_map[1],
      tmp.task_map[2],
      tmp.task_map[3],
      tmp.task_map[4],
      tmp.task_map[5],
      tmp.task_map[6],
      tmp.task_map[7]);
Os9SetDatMapForTask(tmp.task_num, tmp.task_map);

  tmp.client_op = op;

  if (opening) {
    tmp.opening_original_rx = opening->original_rx;
    tmp.opening_pathname_size = cli->regs->rx - opening->original_rx;
    PrintH("OPENING: orig=%x cli.rx=%x size=%x\n",
        opening->original_rx,
        cli->regs->rx,
        tmp.opening_pathname_size);
  } else {
    tmp.opening_original_rx = 0;
    tmp.opening_pathname_size = 0;
  }

  ////////////////////////
  // Now we switch and let the daemon run.
  PrintH("\nClientOperationC switch-to-daemon\n");
  SwitchProcess(cli, dae);
  PrintH("\nClientOperationC un-switch-from-deamon\n");

  PrintH("\n RETAINED task-> %x (e %x)", tmp.task_num, e);
  PrintH(" RETAINED Task#=%x Map=%x %x  %x %x  %x %x  %x %x\n",
      tmp.task_num,
      tmp.task_map[0],
      tmp.task_map[1],
      tmp.task_map[2],
      tmp.task_map[3],
      tmp.task_map[4],
      tmp.task_map[5],
      tmp.task_map[6],
      tmp.task_map[7]);

  // When we return, the daemon has given us a reply.
  ////////////////////////

  PrintH("ClientOperationC op=%x reply.status=%x size=%x ret\n", op, tmp.reply.status, tmp.reply.size);
  PrintH("\nReleasingTask# %x\n", tmp.task_num);
  Os9ReleaseTask(tmp.task_num); // Free the temporary task num.
  cli->current_process_id = 0;
  return tmp.reply.status;
}

////////////////////////////////////////////////
///
///  GENERIC "C" FUNCTIONS: could be Daemon or Client.

errnum CreateOrOpenC(struct PathDesc* pd, struct Regs* regs) {
  pd->device_table_entry->dt_num_users = 16; // ARTIFICIALLY KEEP THIS OPEN.
  assert(pd->regs == regs);
  word original_rx = regs->rx;

  PrintH("CreateOrOpenC: pd=%x regs=%x\n", pd, regs);
  PrintH("@ CreateOrOpenC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);

  // Split the path to find 2nd (begin1/end1) and
  // 3rd (begin2/end2) words.  Ignore the 1st ("FUSE").
  word begin1=0, end1=0, begin2=0, end2=0;
  errnum err = OKAY;
  byte i = 0;
  for (; i<99; i++) {
    word begin = 0, end = 0;
    word current = regs->rx;
    err = Os9PrsNam(current, &begin, &end);
    regs->rx = end; // important to update rx
    if (err) break;

    PrintH("Parse[%d]: begin=%x end=%x ", i, begin, end);
    ShowParsedName(current, begin, end);
    PrintH("\n");
SWITCH:
    switch (i) {
      case 0:
        if (!ParsedNameEquals(begin, end, "FUSE")) {
          // Starts with something other than FUSE,
          // so treat the starting name as the second name,
          // which chooses the daemon.
          i = 1;
          goto SWITCH;
        }
        break;
      case 1:
        begin1 = begin;
        end1 = end;
        break;
      case 2:
        // TODO:  more than one name.
        begin2 = begin;
        end2 = end;
        break;
      default:
        {}// Ignore extra names for now.
    }
  }

  errnum z;
  if (i==3 && ParsedNameEquals(begin1, end1, "DAEMON")) {
    z = OpenDaemon(pd, begin2, end2);
  } else {
    struct Opening o = { begin1, end1, begin2, end2, original_rx };
    z = ClientOperationC(pd, OP_OPEN, &o);
  }
  PrintH("@ CreateOrOpenC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum CloseC(struct PathDesc* pd, struct Regs* regs) {
  errnum z = 0;
  PrintH("@ CloseC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);

  if (pd->link_count) {
    Os9AwakenIOQN(pd);
    PrintH("@ CloseC/retEarly pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
    return OKAY;
  }

  if (pd->is_daemon) {
    struct PathDesc* dae = pd;
    CheckDaemon(dae);
    bzero(DAEMON_NAME_OFFSET_IN_PD + (char*)dae, DAEMON_NAME_MAX);  // Wipe the daemon name.
    z = OKAY;
  } else {
    struct PathDesc* cli = pd;
    CheckClient(cli);
    z = ClientOperationC(cli, OP_CLOSE, NULL);
    ClearPeer(cli, cli->peer); // Finally unlink the daemon when client is closed.
  }
  PrintH("@ CloseC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum ReadLnC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ ReadLnC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_BMODE;  // Daemon must use Read not ReadLn.
  } else {
    z = ClientOperationC(pd, OP_READLN, NULL);
  }
  PrintH("@ ReadLnC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum WritLnC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ WritLnC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_BMODE; // Daemon must use Write not WritLn.
  } else {
    z = ClientOperationC(pd, OP_WRITLN, NULL);
  }
  PrintH("@ WritLnC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum ReadC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ ReadC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = DaemonReadC(pd);
  } else {
    z = ClientOperationC(pd, OP_READ, NULL);
  }
  PrintH("@ ReadC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum WriteC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ WriteC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = DaemonWriteC(pd);
  } else {
    z = ClientOperationC(pd, OP_WRITE, NULL);
  }
  PrintH("@ WriteC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum GetStatC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ GetStatC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_UNKSVC;
  } else {
    z = ClientOperationC(pd, OP_GETSTAT, NULL);
  }
  PrintH("@ GetStatC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum SetStatC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ SetStatC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_UNKSVC;
  } else {
    z = ClientOperationC(pd, OP_SETSTAT, NULL);
  }
  PrintH("@ SetStatC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum MakDirC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ MakDirC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_UNKSVC;
  } else {
    z = ClientOperationC(pd, OP_MAKDIR, NULL);
  }
  PrintH("@ MakDirC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum ChgDirC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ ChgDirC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_UNKSVC;
  } else {
    z = ClientOperationC(pd, OP_CHGDIR, NULL);
  }
  PrintH("@ ChgDirC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum DeleteC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ DeleteC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_UNKSVC;
  } else {
    z = ClientOperationC(pd, OP_DELETE, NULL);
  }
  PrintH("@ DeleteC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

errnum SeekC(struct PathDesc* pd, struct Regs* regs) {
  errnum z;
  PrintH("@ SeekC/ent pd=%x links=%x cpr=%x\n", pd, pd->link_count, pd->current_process_id);
  assert(pd->regs == regs);
  if (pd->is_daemon) {
    z = E_UNKSVC;
  } else {
    z = ClientOperationC(pd, OP_SEEK, NULL);
  }
  PrintH("@ SeekC/ret pd=%x links=%x cpr=%x z=%x\n", pd, pd->link_count, pd->current_process_id, z);
  return z;
}

/////////////// Assembly-to-C Relays

asm CreateOrOpenA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _CreateOrOpenC  ; Call C function to do the work.

; Shared by all `asm ...A()` functions:
; Returning from the XxxxC() routines,
; the status is in the B register.
BackToAssembly
    CLRA     ; clear the carry bit.
    TSTB     ; we want to set carry if B nonzero.
    BEQ SkipComA  ; skip the COMA
    COMA     ; sets the carry bit.
SkipComA
    PULS PC,U,Y
  }
}
asm CloseA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _CloseC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm ReadLnA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _ReadLnC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm WritLnA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _WritLnC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm ReadA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _ReadC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm WriteA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _WriteC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm GetStatA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _GetStatC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm SetStatA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _SetStatC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm MakDirA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _MakDirC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm ChgDirA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _ChgDirC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm DeleteA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _DeleteC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
asm SeekA() {
  asm {
    PSHS Y,U ; Push U=regs then Y=pathdesc, as args to C fn, and to restore Y and U later.
    LDU #0   ; Terminate frame pointer chain for CMOC.
    LBSR _SeekC  ; Call C function to do the work.
    LBRA BackToAssembly
  }
}
