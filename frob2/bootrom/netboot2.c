#include "frob2/bootrom/bootrom.h"

static byte WizGet1(word reg) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  byte z = WIZ->data;
  if (reg < 0x1000) {
    printk("%x->%x", reg, z);
  }
  return z;
}
static word WizGet2(word reg) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  byte z_hi = WIZ->data;
  byte z_lo = WIZ->data;
  word z = ((word)(z_hi) << 8) + (word)z_lo;
  if (reg < 0x1000) {
    printk("%x=>%x", reg, z);
  }
  return z;
}
static void WizGetN(word reg, void* buffer, word size) {
  byte* to = (byte*) buffer;
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  for (word i=0; i<size; i++) {
    *to++ = WIZ->data;
  }
  printk("%x[%x]>", reg, size);
}
static void WizPut1(word reg, byte value) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  WIZ->data = value;
  if (reg < 0x1000) {
    printk("%x<-%x", reg, value);
  }
}
static void WizPut2(word reg, word value) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  WIZ->data = (byte)(value >> 8);
  WIZ->data = (byte)(value);
  if (reg < 0x1000) {
    printk("%x<=%x", reg, value);
  }
}
static void WizPutN(word reg, const void* data, word size) {
  const byte* from = (const byte*) data;
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  for (word i=0; i<size; i++) {
    WIZ->data = *from++;
  }
  printk("%x[%x]<", reg, size);
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    return WizGet2(0x0082/*TCNTR Tick Counter*/);
}

void WizReset() {
  PutStr("WizReset ");
  WIZ->command = 128; // Reset
L Delay(9000);
  WIZ->command = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
L Delay(9000);

  // Interval until retry.
  WizPut2(RTR0, 10000 /* Tenths of milliseconds. */ );
  // Number of retries.
  WizPut1(RCR, 10);

  // Default configuration of 2k per ring.
  // WizPut1(0x001a/*=Rx Memory Size*/, 0x55); // 2k per sock
  // WizPut1(0x001b/*=Tx Memory Size*/, 0x55); // 2k per sock
}

void WizConfigure(const byte* ip_addr, const byte* ip_mask, const byte* ip_gateway) {
L WizPutN(0x0001/*gateway*/, ip_gateway, 4);
L WizPutN(0x0005/*mask*/, ip_mask, 4);
L WizPutN(0x000f/*ip_addr*/, ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
  Vars->mac_addr [0] = 2;
  Vars->mac_addr [1] = 2;
  memcpy(Vars->mac_addr+2, ip_addr, 4);
L WizPutN(0x0009/*ether_mac*/, Vars->mac_addr, 6);
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

// Advance to a new status after old_status.
byte WizNextStatus(PARAM_SOCK_AND byte old_status) {
  byte status;
  Line("<S");
  byte stuck = 200;
  do {
    if (!--stuck) Fatal("S", old_status);
    status = WizGet1(B+SK_SR);
    Line("S");
  } while (status == old_status);
  Line("S>");
  return status;
}


void TcpOpen(PARAM_SOCK_AND word local_port ) {
  WizPut1(B+SK_MR, SK_MR_TCP); // Set TCP Protocol mode.
  WizPut2(B+SK_PORTR0, local_port); // Set TCP local port.
  WizPut1(B+SK_IR, 0xFF); // Clear all interrupts.
  WizIssueCommand(SOCK_AND SK_CR_OPEN);

  WizWaitStatus(SOCK_AND SK_SR_INIT);
  WizPut2(B+SK_TX_WR0, 0); // Does this help?
}

// Only called for TCP Client.
void TcpDial(PARAM_SOCK_AND const byte* host, word port) {
  WizPutN(B+SK_DIPR0, host, 4);
  WizPut2(B+SK_DPORTR0, port);
  WizIssueCommand(SOCK_AND SK_CR_CONN);
}

// For Server or Client to accept/establish connection.
void TcpEstablishOrNotYet(PARAM_JUST_SOCK) {
  WizPut1(B+SK_IR, 0xFF); // Clear Interrupt bits.

  byte stuck = 250;
  byte status;
  Line("<E");
  while(1) {
    Line("E");
    // Or we could wait for the connected interrupt bit,
    // and not the disconnected nor the timeout bit.
    byte status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("TEZ", status);

    if (status == SK_SR_ESTB) break;
    if (status == SK_SR_INIT) continue;
    if (status == SK_SR_SYNS) continue;

    Fatal("TE", status);

  };
  Line("E>");
  // TODO -- why does this bit continue to show up?
  WizPut1(B+SK_IR, SK_IR_CON); // Clear the Connection bit.
}

void TcpPoll(PARAM_JUST_SOCK) {
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
      byte sr = WizGet1(B+SK_SR); // Socket Status Register

      if (sr != SK_SR_ESTB) { // No longer established?
        return;
        // Fatal("TNE", sr);
      }
      if (ir & SK_IR_TOUT) { // Timeout?
        Fatal("TTO", ir);
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        Fatal("TDIS", ir);
      }

      if (ir & SK_IR_TXOK) { // OK to send?
        if (WizGet2(B+SK_TX_RD0) != WizGet2(B+SK_TX_WR0)) {  // some bytes to send?
          WizPut1(B+SK_IR, SK_IR_TXOK);  // clear TXOK bit.
          WizIssueCommand(SOCK_AND  SK_CR_SEND);
          Line("SEND!");
        }
      }
}

void TcpRecvData(PARAM_SOCK_AND char* buf, size_t n) {
  word bytes_waiting;
  Line("<R");
  do {
    Line("R");
    bytes_waiting = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
    TcpPoll(JUST_SOCK);
  } while (bytes_waiting < n);
  Line("R>");

  word begin = WizGet2(B+SK_RX_RD0) & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizGetN(R+begin, buf, first_n);
    WizGetN(R, buf+first_n, second_n);
  } else {
    WizGetN(R+begin, buf, n);
  }

  WizPut2(B+SK_RX_RD0, R + begin + n);
  WizIssueCommand(SOCK_AND  SK_CR_RECV); // Inform socket of changed SK_RX_RD.
}

void TcpReserveToSend(PARAM_SOCK_AND  size_t n) {
  word free_size;
  Line("<D");
  do {
  Line("D");
    TcpPoll(JUST_SOCK);
    free_size = WizGet2(B+SK_TX_FSR0);
  } while (free_size < n);
  Line("D>");
  SS->tx_ptr = WizGet2(B+SK_TX_WR0) & RING_MASK;
  SS->tx_to_go = n;
}

void TcpDataToSend(PARAM_SOCK_AND char* data, size_t n) {
  AssertLE(n, SS->tx_to_go);  // Must have already reserved.

  word begin = SS->tx_ptr;  // begin: Beneath RING_SIZE.
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  SS->tx_to_go -= n;
  SS->tx_ptr = (n + SS->tx_ptr) & RING_MASK;
}

void TcpFinalizeSend(PARAM_JUST_SOCK) {
  Line("DF");
  WizPut2(B+SK_TX_WR0, T + SS->tx_ptr);
  AssertEQ(SS->tx_to_go, 0);
  TcpPoll(JUST_SOCK); // Issues the SEND command, when it can.
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

void Waitee(PARAM_JUST_SOCK) {
  char buf[5];
  while (true) {
    printk("<W");
    TcpRecvData(SOCK_AND buf, 5);
    printk("W>");
    switch (buf[0]) {
      case 0: // case POKE
        {
          word n = *(word*)(buf+1);
          word p = *(word*)(buf+3);
          printk("POKE(%x@%x)", n, p);
          TcpRecvData(SOCK_AND (char*)p, n);
        }
        break;
      default:
        Fatal("WUT?", buf[0]);
        break;
    }
  }
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#if WHOLE_PROGRAM
#define RomMain main  // Whole Program Optimization requires 'main' instead of 'RomMain'.
#endif
int RomMain() {
    // Clear our "global" variables to zero.
    memset(Vars, 0, sizeof (struct vars));

    // Set VDG 32x16 text screen to dots, excepting the top line.
    memset(VDG_RAM+32, '.', VDG_LEN-32);

    // Next things printed go on that second line.
    Vars->vdg_ptr = 0x0420;
    // Don't be a boor.
    PutStr("HELO");
    PutStr(__TIME__);

#if CHECKSUMS
    Checksum();
#endif

L   WizReset();
#if BR_STATIC
L   WizConfigure(BR_ADDR, BR_MASK, BR_GATEWAY);
#endif
    word local_port = 0x8000 | 0xFFFE & WizTicks();
    TcpOpen(SOCK1_AND local_port);
    TcpDial(SOCK1_AND BR_GATEWAY, 14511);

#if 1
    char *message = "NANDO";
    TcpReserveToSend(SOCK1_AND  5);
    TcpDataToSend(SOCK1_AND  message, 5);
    TcpFinalizeSend(JUST_SOCK1);
#endif
L
    Waitee(JUST_SOCK1);

    // NOTREACHED
    return 0;
}
