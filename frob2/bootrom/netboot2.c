#include "frob2/bootrom/bootrom.h"

enum Commands {
  CMD_POKE = 0,
  CMD_LOG = 200,
  CMD_INKEY = 201,
  CMD_PUTCHARS = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_SP_PC = 205,
  CMD_JSR = 255,
};

static byte WizGet1(word reg) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  byte z = WIZ->data;
  if (reg < 0x1000) {
    print7("%x->%x", reg, z);
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
    print7("%x=>%x", reg, z);
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
  print7("%x[%x]>", reg, size);
}
static void WizPut1(word reg, byte value) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  WIZ->data = value;
  if (reg < 0x1000) {
    print7("%x<-%x", reg, value);
  }
}
static void WizPut2(word reg, word value) {
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  WIZ->data = (byte)(value >> 8);
  WIZ->data = (byte)(value);
  if (reg < 0x1000) {
    print7("%x<=%x", reg, value);
  }
}
static void WizPutN(word reg, const void* data, word size) {
  const byte* from = (const byte*) data;
  WIZ->addr_hi = (byte)(reg >> 8);
  WIZ->addr_lo = (byte)(reg);
  for (word i=0; i<size; i++) {
    WIZ->data = *from++;
  }
  print7("%x[%x]<", reg, size);
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    return WizGet2(0x0082/*TCNTR Tick Counter*/);
}

void WizReset() {
  printk("WizReset ");
  WIZ->command = 128; // Reset
  Delay(5000);
  WIZ->command = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  Delay(1000);
 
  // GLOBAL OPTIONS FOR SOCKETLESS AND ALL SOCKETS:

  // Interval until retry: 1 second.
  WizPut2(RTR0, 10000 /* Tenths of milliseconds. */ );
  // Number of retries.
  WizPut1(RCR, 10);
}

void WizConfigure(const byte* ip_addr, const byte* ip_mask, const byte* ip_gateway) {
  WizPutN(0x0001/*gateway*/, ip_gateway, 4);
  WizPutN(0x0005/*mask*/, ip_mask, 4);
  WizPutN(0x000f/*ip_addr*/, ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
  Vars->mac_addr [0] = 2;
  Vars->mac_addr [1] = 2;
  memcpy(Vars->mac_addr+2, ip_addr, 4);
  WizPutN(0x0009/*ether_mac*/, Vars->mac_addr, 6);
  printk("Conf a=%a m=%a g=%a MAC=2.2.%a", ip_addr, ip_mask, ip_gateway, Vars->mac_addr+2);
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

bool IsBroadcast(PARAM_JUST_SOCK) {
  byte first = WizGet1(B+SK_DIPR0);
  return first==255;  // Net 255 is enough to check.
}


void WizOpen(PARAM_SOCK_AND struct proto* proto, word local_port ) {
  WizPut1(B+SK_MR, proto->open_mode);
  WizPut2(B+SK_PORTR0, local_port); // Set local port.
  WizPut1(B+SK_IR, 0xFF); // Clear all interrupts.
  WizIssueCommand(SOCK_AND SK_CR_OPEN);

  WizWaitStatus(SOCK_AND proto->open_status);
}

// Only called for TCP Client.
void TcpDial(PARAM_SOCK_AND const byte* host, word port) {
  WizPut2(B+SK_TX_WR0, T); // does this help

  printk("tcp dial %a $x", host, port);
  WizPutN(B+SK_DIPR0, host, 4);
  WizPut2(B+SK_DPORTR0, port);
  WizPut1(B+SK_IR, 0xFF); // Clear Interrupt bits.
  WizIssueCommand(SOCK_AND SK_CR_CONN);
}

// For Server or Client to accept/establish connection.
void TcpEstablish(PARAM_JUST_SOCK) {
  byte stuck = 250;
  while(1) {
    Delay(1000);
    printk("+");
    // Or we could wait for the connected interrupt bit,
    // and not the disconnected nor the timeout bit.
    byte status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("TEZ", status);

    if (status == SK_SR_ESTB) break;
    if (status == SK_SR_INIT) continue;
    if (status == SK_SR_SYNS) continue;

    Fatal("TE", status);

  };

  print4("est:q=%x", WizGet1(B+SK_IR));
  WizPut1(B+SK_IR, SK_IR_CON); // Clear the Connection bit.
  print4("..:q=%x", WizGet1(B+SK_IR));
}

void TcpCheck(PARAM_JUST_SOCK) {
#if 0
      byte sr = WizGet1(B+SK_SR); // Socket Status Register

      // TODO -- this is clearly wrong.
      if (sr != SK_SR_ESTB) { // No longer established?
        return;
      }
#endif
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
      if (ir & SK_IR_TOUT) { // Timeout?
        Fatal("TMO", ir);
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        Fatal("DIS", ir);
      }
}

bool TcpRecvDataTry(PARAM_SOCK_AND char* buf, size_t n) {
  TcpCheck(JUST_SOCK);

  word bytes_waiting = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
  word rd = WizGet2(B+SK_RX_RD0);
  word wr = WizGet2(B+SK_RX_WR0);

  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.
  print5("n=%x^*%x^r%x^w%x^b/%x^e/%x", n, bytes_waiting, rd, wr, begin, end);

  if (bytes_waiting < n) return false;

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
  return true;
}
void TcpRecvData(PARAM_SOCK_AND char* buf, size_t n) {
  Line("<R");
  bool ok;
  do {
    ok = TcpRecvDataTry(SOCK_AND buf, n);
  } while (!ok);
  Line("R>");
}

void UdpDial(PARAM_SOCK_AND  const byte* dest_ip, word dest_port) {
  WizPutN(B+SK_DIPR0, dest_ip, 4);
  WizPut2(B+SK_DPORTR0, dest_port);

  // Only UDP will send to broadcast.
  byte send_command = SK_CR_SEND;
  if (IsBroadcast(JUST_SOCK)) {
    // Broadcast to 255.255.255.255
    send_command = SK_CR_SEND+1;
    // TODO: when do we undo this?
    WizPutN(B+6/*Sn_DHAR*/, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
  }

}
void WizReserveToSend(PARAM_SOCK_AND  size_t n) {
  // Wait until free space is available.
  word free_size;
  Line("<D");
  do {
  Line("D");
    // TcpCheck(JUST_SOCK);
    free_size = WizGet2(B+SK_TX_FSR0);
  } while (free_size < n);
  Line("D>");

  SS->tx_ptr = WizGet2(B+SK_TX_WR0) & RING_MASK;
  SS->tx_to_go = n;
  print4("1tx.ptr=%x togo=%x", SS->tx_ptr, SS->tx_to_go);
}

void WizDataToSend(PARAM_SOCK_AND char* data, size_t n) {
  // TcpCheck(JUST_SOCK);
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
  print4("2tx.ptr=%x togo=%x", SS->tx_ptr, SS->tx_to_go);
}

void WizFinalizeSend(PARAM_SOCK_AND size_t n) {
  // TcpCheck(JUST_SOCK);
  print4("FSend %x", n);
  word tx_wr = WizGet2(B+SK_TX_WR0);  
  print4("3tx..%x", tx_wr);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  print4("..%x", tx_wr);
  AssertEQ(SS->tx_to_go, 0);
  byte send_command = SK_CR_SEND + IsBroadcast(JUST_SOCK);
  WizIssueCommand(SOCK_AND  send_command);
  // TcpCheck(JUST_SOCK);
}

void WizSend(PARAM_SOCK_AND  char* data, size_t n) {
  print4("SEND");
  TcpCheck(JUST_SOCK);
  WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND data, n);
  WizFinalizeSend(SOCK_AND n);
}

void Close(PARAM_JUST_SOCK) {
  print4("Z");
  WizIssueCommand(SOCK1_AND 0x10/*CLOSE*/);
  WizPut1(B+SK_MR, 0x00/*Protocol: Socket Closed*/);
  WizPut1(B+0x0002/*_IR*/, 0x1F); // Clear all interrupts.
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

void LemmaClientS1() {
  char quint[5];
  char inkey;
  bool ok;

  while (true) {
    inkey = PolCat();
    if (inkey) {
      memset(quint, 0, sizeof quint);
      quint[0] = CMD_INKEY;
      quint[4] = inkey;
      WizSend(SOCK1_AND  quint, sizeof quint);
    }

    TcpCheck(JUST_SOCK1);

    ok = TcpRecvDataTry(SOCK1_AND quint, sizeof quint);
    if (ok) {
      word n = *(word*)(quint+1);
      word p = *(word*)(quint+3);
      switch ((byte)quint[0]) {
        case CMD_POKE:
          {
            print3("POKE(%x@%x)", n, p);
            while (n) {
              word chunk = (n < 256u) ? n : 256u;
              printk("#$%x", chunk);
              TcpRecvData(SOCK1_AND (char*)p, chunk);
              n -= chunk;
              p += chunk;
            }
            print3(".");
          }
          break;
        case CMD_PUTCHARS:
          {
            for (word i = 0; i < n; i++) {
              PutChar(*(char*)(p+i));
            }
          }
          break;
        case CMD_PEEK:
          {
            print3("PEEK(%x@%x)", n, p);
            quint[0] = CMD_DATA;
            WizSend(SOCK1_AND quint, 5);
            WizSend(SOCK1_AND (char*)p, n);
          }
          break;
        case CMD_JSR:
          {
            func fn = (func)p;
            print3("CALLING %x", fn); Delay(9000);
            fn();
            print3("RETURNING FROM %x", fn); Delay(9000);
          }
          break;
        default:
          Fatal("WUT?", quint[0]);
          break;
      } // switch
    } // ok
  } // true
}

void Send5(byte cmd, word n, word p) {
    char quint[5] = { cmd, (byte)(n>>8), (byte)(n&255), (byte)(p>>8), (byte)(p&255) };
    WizSend(SOCK1_AND quint, 5);
} 


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#if WHOLE_PROGRAM
#define RomMain main  // Whole Program Optimization requires 'main' instead of 'RomMain'.
#endif
int RomMain() {
    word sp_reg = StackPointer();

    // Clear our "global" variables to zero.
    memset(Vars, 0, sizeof (struct vars));

    // Set VDG 32x16 text screen to dots, excepting the top line.
    memset(VDG_RAM+32, '.', VDG_LEN-32);

    // Next things printed go on that second line.
    Vars->vdg_ptr = 0x0420;

    printk("SP=%x REV=%s@%s", sp_reg, __DATE__, __TIME__);

#if CHECKSUMS
    Checksum();
#endif
    word ticks = WizTicks();
    word local_port = 0x8000 | ticks;
    printk("  TICKS *%x* PORT *%x*  ", ticks, local_port);

L   WizReset();
#if BR_STATIC
L   WizConfigure(BR_ADDR, BR_MASK, BR_GATEWAY);
#endif
    // word local_port = 0x8000 | 0xFFFE & WizTicks();
    WizOpen(SOCK1_AND &TcpProto, local_port);
    TcpDial(SOCK1_AND BR_WAITER, 14511);
    TcpEstablish(JUST_SOCK1);
    Send5(CMD_SP_PC, sp_reg, (word)&RomMain);

word p = 0x0000;
while (1) {
    Send5(CMD_LOG, 5, 0); WizSend(SOCK1_AND  "NANDO", 5);
    Send5(CMD_LOG, 5, 0); WizSend(SOCK1_AND  "NANDO", 5);
    //Delay(1000);
    Send5(CMD_LOG, 5, 0); WizSend(SOCK1_AND  "BILBO", 5);
    //Delay(1000);
    Send5(CMD_LOG, 5, 0); WizSend(SOCK1_AND  "FRODO", 5);
    //Delay(1000);
    Send5(CMD_LOG, 5, 0); WizSend(SOCK1_AND  "SAMWI", 5);
    // Delay(5000);
    
    char quint[5] = { 204, 1, 0, p>>8, p&255 };
    WizSend(SOCK1_AND quint, 5);
    WizSend(SOCK1_AND p, 0x100);
    // Delay(5000);
    p += 0x100;
    if ( p == 0xFF00 ) p = 0x0000;
}

#if 0
    while (1) {}
    Fatal("NOT", 13);

#if 1
    LemmaClientS1();
#endif
    Fatal("OKAY", 250);

    // NOTREACHED
#endif
    return 0;
}
