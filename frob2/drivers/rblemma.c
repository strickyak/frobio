#include "frob2/drivers/rblemma.h"

////////////////////////////////////////////////////////////


#if EMULATED
void PrintH(const char* format, ...) {
#ifdef __GNUC__
  //# const char ** fmt_ptr = &format;
  //# // TODO: ldx fmt_ptr
  //# asm volatile ("swi\n  fcb 108" : : "m" (fmt_ptr));
#else
  asm {
    // TODO: ldx fmt_ptr
      swi
      fcb 108
  }
#endif
}
#endif

void Delay(word n) {
#if !EMULATED
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


void Fatal(const char* wut, word arg) {
    PrintH(" *FATAL* <%s$%x> ", wut, arg);
    while (1) continue;
}

void AssertEQ(word a, word b) {
  if (a != b) {
    PrintH("%x", a);
    Fatal("!EQ", b);
  }
}

void AssertLE(word a, word b) {
  if (a > b) {
    PrintH("%x", a);
    Fatal("!LE", b);
  }
}

// Debug Trace by Line Number
void ShowLine(word line) {
#if VERBOSE >= 6
    printk("%u", line);
#endif
}

void Line(const char* s) {
#if VERBOSE >= 6
  PrintH(s);
  PrintH(" ");
#endif
}

////////////////////////////////////////////////////////////

enum Commands {
  CMD_POKE = 0,
  CMD_LOG = 200,
  CMD_INKEY = 201,
  CMD_PUTCHARS = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_SP_PC = 205,
  CMD_REV = 206,
  CMD_BLOCK_READ = 207,  // block device
  CMD_BLOCK_WRITE = 208,  // block device
  CMD_BLOCK_ERROR = 209,  // block device
  CMD_BOOT_BEGIN = 211,  // boot_lemma
  CMD_BOOT_CHUNK = 212,  // boot_lemma
  CMD_BOOT_END = 213,  // boot_lemma
  CMD_JSR = 255,
};

const char Rev[] = __DATE__ " " __TIME__;

static byte WizGet1(word reg) {
  *(word*)(&WIZ->addr_hi) = reg;
  byte z = WIZ->data;
  if (reg < 0x1000) {
    print7("%x->%x", reg, z);
  }
  return z;
}
static word WizGet2(word reg) {
  *(word*)(&WIZ->addr_hi) = reg;
  byte z_hi = WIZ->data;
  byte z_lo = WIZ->data;
  word z = ((word)(z_hi) << 8) + (word)z_lo;
  if (reg < 0x1000) {
    print7("%x=>%x", reg, z);
  }
  return z;
}
static void WizGetN(word reg, void* buffer, word size) {
  byte* to = (byte*) buffer;
  *(word*)(&WIZ->addr_hi) = reg;
  for (word i=size; i; i--) {
    *to++ = WIZ->data;
  }
  print7("%x[%x]>", reg, size);
}
static void WizPut1(word reg, byte value) {
  *(word*)(&WIZ->addr_hi) = reg;
  WIZ->data = value;
  if (reg < 0x1000) {
    print7("%x<-%x", reg, value);
  }
}
static void WizPut2(word reg, word value) {
  *(word*)(&WIZ->addr_hi) = reg;
  WIZ->data = (byte)(value >> 8);
  WIZ->data = (byte)(value);
  if (reg < 0x1000) {
    print7("%x<=%x", reg, value);
  }
}
static void WizPutN(word reg, const void* data, word size) {
  const byte* from = (const byte*) data;
  *(word*)(&WIZ->addr_hi) = reg;

  for (word i=size; i; i--) {
    WIZ->data = *from++;
  }
  print7("%x[%x]<", reg, size);
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    return WizGet2(0x0082/*TCNTR Tick Counter*/);
}

/////////////////////////////////////////////////////////////////////////////////////

void WizIssueCommand(PARAM_SOCK_AND byte cmd) {
  WizPut1(B+SK_CR, cmd);
  Line("<C");
  while (WizGet1(B+SK_CR)) {Line("C");}
  Line("C>");
}

void WizWaitStatus(PARAM_SOCK_AND byte want) {
  byte status;
  Line("<W");
  byte stuck = 200;
  do {
    status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("W", status);
    Line("W");
  } while (status != want);
  Line("W>");
}

struct proto {
  byte open_mode;
  byte open_status;
};
const struct proto TcpProto = {
  SK_MR_TCP+SK_MR_ND, // TCP Protocol mode with No Delayed Ack.
  SK_SR_INIT, 
};
const struct proto UdpProto = {
  SK_MR_UDP, // UDP Protocol mode.
  SK_SR_UDP,
};

errnum TcpCheck(PARAM_JUST_SOCK) {
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
      if (ir & SK_IR_TOUT) { // Timeout?
        return 246 /* ERROR: Not Ready */;
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        return 240 /* ERROR: Unit Error*/;
      }
      return 0;
}

errnum TcpRecvDataTry(PARAM_SOCK_AND char* buf, size_t n) {
  errnum e = TcpCheck(JUST_SOCK);
  if (e) return e;

  word bytes_waiting = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
  word rd = WizGet2(B+SK_RX_RD0);
  word wr = WizGet2(B+SK_RX_WR0);

  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.
  PrintH("TTn=%x^*%x^r%x^w%x^b/%x^e/%x", n, bytes_waiting, rd, wr, begin, end);

  if (bytes_waiting < n) return E_NOT_YET;

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
errnum TcpRecvData(PARAM_SOCK_AND char* buf, size_t n) {
  Line("<R");
  errnum e;
  do {
    e = TcpRecvDataTry(SOCK_AND buf, n);
  } while (e == E_NOT_YET);
  Line("R>");
  return e;
}

errnum WizReserveToSend(PARAM_SOCK_AND  size_t n) {
  PrintH("ResTS %x", n);
  // Wait until free space is available.
  word free_size;
  Line("<D");
  do {
  Line("D");
    // TcpCheck(JUST_SOCK);
    free_size = WizGet2(B+SK_TX_FSR0);
    PrintH("Res free %x", free_size);
  } while (free_size < n);
  Line("D>");
  return OKAY;
}

errnum WizDataToSend(PARAM_SOCK_AND char* data, size_t n) {
  word begin = WizGet2(B+SK_TX_WR0) & RING_MASK;
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  return OKAY;
}

errnum WizFinalizeSend(PARAM_SOCK_AND size_t n) {
  print4("FSend %x", n);
  word tx_wr = WizGet2(B+SK_TX_WR0);  
  print4("3tx..%x", tx_wr);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  print4("..%x", tx_wr);
  byte send_command = SK_CR_SEND;
  WizIssueCommand(SOCK_AND  send_command);
  return OKAY;
}

errnum WizSend(PARAM_SOCK_AND  char* data, size_t n) {
  print4("SEND");
  errnum e = TcpCheck(JUST_SOCK);
  if (e) return e;
  e = WizReserveToSend(SOCK_AND  n);
  if (e) return e;
  e = WizDataToSend(SOCK_AND data, n);
  if (e) return e;
  e = WizFinalizeSend(SOCK_AND n);
  return e;
}

#define PD_BUF 8

errnum BlockRead(byte driveNum, byte lsn1, word lsn2, word path_desc) {
  char* buffer = *(char**)(path_desc + PD_BUF);
#if 0
  Quint { BLOCK_READ, drive^lsn1, lsn2 }
  Send
  Recv Quint
  Recv 256 (if not ERROR)
#endif
}

errnum BlockWrite(byte driveNum, byte lsn1, word lsn2, word path_desc) {
  char* buffer = *(char**)(path_desc + PD_BUF);
#if 0
  Quint { BLOCK_WRITE, drive^lsn1, lsn2 }
  Send
  Send 256
#endif
}
