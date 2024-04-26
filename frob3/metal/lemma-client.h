:ifndef _METAL_LEMMA_CLIENT_H_
#define _METAL_LEMMA_CLIENT_H_

// Register values for Socket #1 (used by Lemma)
#define S1B 0x500   // per-socket regs Base
#define S1T 0x4800  // Tx buffer
#define S1R 0x6800  // Rx buffer

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

enum Commands {
  CMD_POKE = 0,
  CMD_HELLO = 1,
  CMD_KEYBOARD = 193,
  CMD_SUM = 194,
  CMD_PEEK2 = 195,
  CMD_LOG = 200,
  CMD_INKEY = 201,
  CMD_PUTCHAR = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_SP_PC = 205, // deprecate
  CMD_REV = 206, // deprecate
  CMD_RTI = 214,  // experimental
  CMD_JSR = 255,
};

struct wiz_port {
  volatile byte command;
  volatile word addr;
  volatile byte data;
} * Wiz;

struct proto {
  byte open_mode;
  byte open_status;
  bool is_broadcast;
  byte send_command;
};

//////////////////////////////////////////////

const struct sock Socks[2] = {
    { 0x400, 0x4000, 0x6000 },
    { 0x500, 0x4800, 0x6800 },
    { 0x600, 0x5000, 0x7000 },
    { 0x700, 0x5800, 0x7800 },
};

const struct proto TcpProto = {
  SK_MR_TCP+SK_MR_ND, // TCP Protocol mode with No Delayed Ack.
  SK_SR_INIT,
  false,
  SK_CR_SEND,
};
const struct proto UdpProto = {
  SK_MR_UDP, // UDP Protocol mode.
  SK_SR_UDP,
  false,
  SK_CR_SEND,
};
const struct proto BroadcastUdpProto = {
  SK_MR_UDP, // UDP Protocol mode.
  SK_SR_UDP,
  true,
  SK_CR_SEND+1,
};

#define SOCK0_AND (Socks+0),
#define JUST_SOCK0 (Socks+0)

#define SOCK1_AND (Socks+1),
#define JUST_SOCK1 (Socks+1)

#define PARAM_JUST_SOCK   const struct sock* sockp
#define PARAM_SOCK_AND    const struct sock* sockp,
#define JUST_SOCK         sockp
#define SOCK_AND          sockp,

//////////////////////////////////////////////

byte WizGet1(word reg) {
  Wiz->addr = reg;
  return Wiz->data;
}
word WizGet2(word reg) {
  Wiz->addr = reg;
  byte z_hi = Wiz->data;
  byte z_lo = Wiz->data;
  return ((word)(z_hi) << 8) + (word)z_lo;
}
void WizPut1(word reg, byte value) {
  Wiz->addr = reg;
  Wiz->data = value;
}
void WizPut2(word reg, word value) {
  Wiz->addr = reg;
  Wiz->data = (byte)(value >> 8);
  Wiz->data = (byte)(value);
}

struct wiz_port* DetectWiz() {
  Wiz = NULL;
  if (PEEK1(0xFF68) == 3) {
    Wiz = (struct wiz_port*)0xFF68;
    // Print(0x0400, "WIZNET DETECTED AT $FF68.");
  } else if (PEEK1(0xFF78) == 3) {
    Wiz = (struct wiz_port*)0xFF78;
    // Print(0x0400, "WIZNET DETECTED AT $FF78.");
  } else {
    Fatal("CANNOT DETECT WIZNET. ");
  }
  return Wiz;
}

void WizIssueCommand(byte cmd) {
  WizPut1(S1B + SK_CR, cmd);
  while (WizGet1(S1B + SK_CR)) continue;
}

word WizGetNumBytesWaiting() {
  return WizGet2(S1B + SK_RX_RSR0);  // Unread received size.
}

// Receive 1 byte from Lemma.
byte GetLemma() {
  word bytes_waiting;
  do {
    bytes_waiting = WizGetNumBytesWaiting();
  } while (bytes_waiting == 0);

  word rd = WizGet2(S1B + SK_RX_RD0);
  word offset = rd & RING_MASK;  // offset: Beneath RING_SIZE.

  byte z = WizGet1(S1R + offset);
  WizPut2(S1B + SK_RX_RD0, rd + 1);
  WizIssueCommand(SK_CR_RECV);  // Inform socket of changed SK_RX_RD.
  return z;
}

// returns tx_ptr
tx_ptr_t WizReserveToSend(const struct sock* sockp,  size_t n) {
  // PrintH("ResTS %x;", n);
  // Wait until free space is available.
  word free_size;
  do {
    free_size = WizGet2(B+SK_TX_FSR0);
    // PrintH("Res free %x;", free_size);
  } while (free_size < n);

  return WizGet2(B+SK_TX_WR0) & RING_MASK;
}

tx_ptr_t WizBytesToSend(const struct sock* sockp, tx_ptr_t tx_ptr, const byte* data, size_t n) {
  return WizDataToSend(SOCK_AND tx_ptr, (char*)data, n);
}
tx_ptr_t WizDataToSend(const struct sock* sockp, tx_ptr_t tx_ptr, const char* data, size_t n) {

  word begin = tx_ptr;  // begin: Beneath RING_SIZE.
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  return (n + tx_ptr) & RING_MASK;
}

void WizFinalizeSend(const struct sock* sockp, const struct proto *proto, size_t n) {
  word tx_wr = WizGet2(B+SK_TX_WR0);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  WizIssueCommand(SOCK_AND  proto->send_command);
}

errnum WizSendChunk(const struct sock* sockp,  const struct proto* proto, char* data, size_t n) {
  // PrintH("Ax WizSendChunk %x@%x : %x %x %x %x %x", n, data, data[0], data[1], data[2], data[3], data[4]);
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;
  tx_ptr_t tx_ptr = WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND tx_ptr, data, n);
  WizFinalizeSend(SOCK_AND proto, n);
  // PrintH("Ax WSC.");
  return OKAY;
}

errnum TcpSend(const struct sock* sockp,  char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizSendChunk(SOCK_AND &TcpProto, p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  return OKAY;
}

#endif  // _METAL_LEMMA_CLIENT_H_
