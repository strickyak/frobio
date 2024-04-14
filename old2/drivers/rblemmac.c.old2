#include "frob2/drivers/rblemmac.h"

////////////////////////////////////////////////////////////
#if EMULATED
#undef PrintH
void PrintH(const char* format, ...) {
  const char ** fmt_ptr = &format;
#ifdef __GNUC__
  asm volatile ("ldx %0\n  swi\n  fcb 111" : : "m" (fmt_ptr));
#else
  asm {
      ldx fmt_ptr
      swi
      fcb 111  // PrintH2 in emu/hyper.go
  }
#endif
}
#endif
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
  CMD_BLOCK_ERROR = 209,  // nack
  CMD_BLOCK_OKAY = 210,  // ack
  CMD_BOOT_BEGIN = 211,  // boot_lemma
  CMD_BOOT_CHUNK = 212,  // boot_lemma
  CMD_BOOT_END = 213,  // boot_lemma
  CMD_JSR = 255,
};

byte WizGet1(word reg) {
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
  byte* to = (byte*) buffer;
  WIZ->addr = reg;
  for (word i=size; i; i--) {
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
  const byte* from = (const byte*) data;
  WIZ->addr = reg;

  for (word i=size; i; i--) {
    WIZ->data = *from++;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void WizIssueCommand(byte cmd) {
  WizPut1(BASE+SK_CR, cmd);
  while (WizGet1(BASE+SK_CR)) {}
}

void WizWaitStatus(byte want) {
  byte status;
  do {
    status = WizGet1(BASE+SK_SR);
  } while (status != want);
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

errnum TcpCheck() {
      byte ir = WizGet1(BASE+SK_IR); // Socket Interrupt Register.
      if (ir & SK_IR_TOUT) { // Timeout?
        return 246 /* ERROR: Not Ready */;
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        return 240 /* ERROR: Unit Error*/;
      }
      return 0;
}

errnum TcpRecvDataTry(char* buf, size_t n) {
  errnum e = TcpCheck();
  if (e) return e;

  word bytes_waiting = WizGet2(BASE+SK_RX_RSR0);  // Unread Received Size.
  word rd = WizGet2(BASE+SK_RX_RD0);
  word wr = WizGet2(BASE+SK_RX_WR0);

  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.
  PrintH("TTn=%x^*%x^r%x^w%x^b/%x^e/%x", n, bytes_waiting, rd, wr, begin, end);

  if (bytes_waiting < n) return E_NOT_YET;

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizGetN(RX_RING+begin, buf, first_n);
    WizGetN(RX_RING, buf+first_n, second_n);
  } else {
    WizGetN(RX_RING+begin, buf, n);
  }

  WizPut2(BASE+SK_RX_RD0, rd + n);
  WizIssueCommand(SK_CR_RECV); // Inform socket of changed SK_RX_RD.
  return OKAY;
}
errnum TcpRecvData(char* buf, size_t n) {
  errnum e;
  do {
    e = TcpRecvDataTry(buf, n);
  } while (e == E_NOT_YET);
  return e;
}

errnum WizReserveToSend(size_t n) {
  PrintH("ResTS %x", n);
  // Wait until free space is available.
  word free_size;
  do {
    // TcpCheck();
    free_size = WizGet2(BASE+SK_TX_FSR0);
    PrintH("Res free %x", free_size);
  } while (free_size < n);
  return OKAY;
}

errnum WizDataToSend(char* data, size_t n) {
  word begin = WizGet2(BASE+SK_TX_WR0) & RING_MASK;
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(TX_RING+begin, data, first_n);
    WizPutN(TX_RING, data+first_n, second_n);
  } else {
    WizPutN(TX_RING+begin, data, n);
  }
  return OKAY;
}

errnum WizFinalizeSend(size_t n) {
  word tx_wr = WizGet2(BASE+SK_TX_WR0);  
  tx_wr += n;
  WizPut2(BASE+SK_TX_WR0, tx_wr);
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

#define PD_BUF 8

struct quint {
  byte command;
  word n;
  word p;
};

errnum BlockRead(byte driveNum, byte lsn1, word lsn2, word path_desc) {
  struct quint q = {CMD_BLOCK_READ, (driveNum<<8)|lsn1, lsn2};
  errnum e = WizSend((char*)&q, 5);
  if (e) return e;

  // Receive confirmation quint.
  e = TcpRecvData((char*)&q, 5);
  if (e) return e;
  if (q.command == CMD_BLOCK_ERROR) return q.p ? (errnum)q.p : 187; // illegal arg
  if (q.command != CMD_BLOCK_OKAY) return 192; // illegal command

  char* buffer = *(char**)(path_desc + PD_BUF);
  return TcpRecvData(buffer, 256);
}

errnum BlockWrite(byte driveNum, byte lsn1, word lsn2, word path_desc) {
  struct quint q = {CMD_BLOCK_WRITE, (driveNum<<8)|lsn1, lsn2};
  errnum e = WizSend((char*)&q, 5);
  if (e) return e;

  char* buffer = *(char**)(path_desc + PD_BUF);
  e = WizSend(buffer, 256);
  if (e) return e;

  // Receive confirmation quint.
  e = TcpRecvData((char*)&q, 5);
  if (e) return e;
  if (q.command == CMD_BLOCK_ERROR) return q.p ? (errnum)q.p : 187; // illegal arg
  if (q.command != CMD_BLOCK_OKAY) return 192; // illegal command
  return OKAY;
}
