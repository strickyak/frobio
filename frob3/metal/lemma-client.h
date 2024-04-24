#ifndef _METAL_LEMMA_CLIENT_H_
#define _METAL_LEMMA_CLIENT_H_

// Register values for Socket #1 (used by Lemma)
#define S1B 0x500   // per-socket regs Base
#define S1T 0x4800  // Tx buffer
#define S1R 0x6800  // Rx buffer

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

struct wiz_port {
  volatile byte command;
  volatile word addr;
  volatile byte data;
} * Wiz;

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

#endif  // _METAL_LEMMA_CLIENT_H_
