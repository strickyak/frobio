#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"
#include "frob3/wiz/wiz5100s.h"
#include "frob3/wiz/w5100s_defs.h"

// FORWARD of what-was-static:
extern byte WizGet1(word reg);
extern word WizGet2(word reg);
extern void WizPut1(word reg, byte value);
extern void WizPut2(word reg, word value);
extern void WizPutN(word reg, const void* data, word size);

extern byte Wiz__before(word limit);
extern prob Wiz__wait(word reg, byte value, word millisecs_max);

void Wiz__command(word base, byte cmd);
byte Wiz__advance(word base, byte old_status);
bool Wiz__sock_command(word base, byte cmd, byte want);

//chop

// Global storage.
byte* WizHwPort = 0xFF68;  // default hardware port.
//chop

static word bogus_word_for_delay;
void WizDelay(word n) {
  for (word i=0; i<n; i++) bogus_word_for_delay += i;
}

byte WizGet1(word reg) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  byte z = P3;
    EnableIrqsCounting();
  LogDebug("[%4x->%2x]", reg, z);
  return z;
}
word WizGet2(word reg) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  byte hi = P3;
  byte lo = P3;
    EnableIrqsCounting();
  word z = ((word)(hi) << 8) + lo;
  LogDebug("[%4x=>%4x]", reg, z);
  return z;
}
void WizGetN(word reg, char* data, word size) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  for (word i=0; i<size; i++) {
    *data++ = (char) P3;
  }
  EnableIrqsCounting();
}
void WizPut1(word reg, byte value) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  P3 = value;
    EnableIrqsCounting();
  LogDebug("[%4x<-%2x]", reg, value);
}
void WizPut2(word reg, word value) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  P3 = (byte)(value >> 8);
  P3 = (byte)(value);
    EnableIrqsCounting();
  LogDebug("[%4x<=%4x]", reg, value);
}
void WizPutN(word reg, const void* data, word size) {
  const byte* from = (const byte*) data;
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  for (word i=0; i<size; i++) {
    P3 = *from++;
  }
  EnableIrqsCounting();
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    word t = WizGet2(0x0082/*TCNTR Tick Counter*/);
    return t;
}

// Is current time Wiz__before the limit?  Using Wiz Ticks.
byte Wiz__before(word limit) {
    word t = WizGet2(0x0082/*TCNTR Tick Counter*/);
    // After t crosses limit, (limit-t) "goes negative" so high bit is set.
    return 0 == ((limit-t) & 0x8000);
}
prob Wiz__wait(word reg, byte value, word millisecs_max) {
    LogDebug("Wait reg %x value %x ms %x", reg, value, millisecs_max);
    word start = WizGet2(0x0082/*TCNTR Tick Counter*/);
    word limit = 10*millisecs_max + start;
    while (Wiz__before(limit)) {
        if (WizGet1(reg) == value) {
            LogDebug("Wait OK");
            return GOOD;
        }
    }
    LogDebug("Wait Timeout");
    return "Timeout";
}

void WizReset() {
  DisableIrqsCounting();
  P0 = 128; // Reset
  WizDelay(42);
  P0 = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  WizDelay(42);
  EnableIrqsCounting();
}

void wiz_configure_for_DHCP(const char* name4, byte* hw6_out) {
  WizConfigure(0L, 0xFFFFFF00, 0L);

  // Create locally assigned mac_addr from name4.
  byte mac_addr[6] = {2, 32, 0, 0, 0, 0};  // local prefix.
  strncpy((char*)mac_addr+2, name4, 4);
  WizPutN(0x0009/*ether_mac*/, mac_addr, 6);
  memcpy(hw6_out, mac_addr, 6);
}

void WizReconfigureForDhcp(quad ip_addr, quad ip_mask, quad ip_gateway) {
  P1 = 0; P2 = 1;  // start at addr 0x0001: Gateway IP.
  WizPutN(0x0001/*gateway*/, &ip_gateway, 4);
  WizPutN(0x0005/*mask*/, &ip_mask, 4);
  WizPutN(0x000f/*ip_addr*/, &ip_addr, 4);
  // Keep the same mac_addr.
}
void WizConfigure(quad ip_addr, quad ip_mask, quad ip_gateway) {
  DisableIrqsCounting();

  P1 = 0; P2 = 1;  // start at addr 0x0001: Gateway IP.
  WizPutN(0x0001/*gateway*/, &ip_gateway, 4);
  WizPutN(0x0005/*mask*/, &ip_mask, 4);
  WizPutN(0x000f/*ip_addr*/, &ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
  byte mac_addr[6] = {2, 0, 0, 0, 0, 0};  // local prefix.
  *(quad*)(mac_addr+2) = ip_addr;
  WizPutN(0x0009/*ether_mac*/, mac_addr, 6);

  WizPut1(0x001a/*=Rx Memory Size*/, 0x55); // 2k per sock
  WizPut1(0x001b/*=Tx Memory Size*/, 0x55); // 2k per sock

  // Force all 4 sockets to be closed.
  for (byte socknum=0; socknum<4; socknum++) {
      word base = ((word)socknum + 4) << 8;
      WizPut1(base+SockCommand, 0x10/*CLOSE*/);
      (void) Wiz__wait(base+SockCommand, 0, 500);
      WizPut1(base+SockMode, 0x00/*Protocol: Socket Closed*/);
      WizPut1(base+0x001e/*_RXBUF_SIZE*/, 2); // 2KB
      WizPut1(base+0x001f/*_TXBUF_SIZE*/, 2); // 2KB
  }
  EnableIrqsCounting();
}

prob UdpClose(byte socknum) {
  LogDebug("CLOSE: sock=%x ", socknum);
  if (socknum > 3) return "NoAvailableSocket";

  word base = ((word)socknum + 4) << 8;
  WizPut1(base+SockInterrupt/*_IR*/, 0x1F); // Clear all interrupts.
  WizPut1(base+SockCommand, 0x10/*CLOSE*/);
  (void) Wiz__wait(base+SockCommand, 0, 500);
  WizPut1(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  return GOOD;
}

prob WizArp(quad dest_ip, byte* mac_out) {
  byte* d = (byte*)&dest_ip;

  // Socketless ARP command.
  DisableIrqsCounting();
  // Socket-less Peer IP Address Register
  WizPut1(0x004c/*=SLCR*/, 0/*=Clear*/); // command
  WizPut1(0x005e/*=SL Irq Mask Reg*/, 0/*=no irqs*/);
  WizPut1(0x005f/*=SL Irq Mask Reg*/, 7/*=clear all*/);
  WizPutN(0x0050 /*=SLPIPR*/, &dest_ip, sizeof dest_ip);

  // Socketless PING command.
  WizPut1(0x004c/*=SLCR*/, 2/*=ARP*/); // command

  byte x = 0;
  do {
    WizPut1(0x005f, 0); // clear interrupt reg

    WizDelay(42);
    x = WizGet1(0x005f/*=SLIR socketless interrupt reg*/);
    byte m1 = WizGet1(0x0054);
    byte m2 = WizGet1(0x0055);
    byte m3 = WizGet1(0x0056);
    byte m4 = WizGet1(0x0057);
    byte m5 = WizGet1(0x0058);
    byte m6 = WizGet1(0x0059);
    LogDebug("(arp->(%x) %x:%x:%x:%x:%x:%x) ",
      x, m1, m2, m3, m4, m5, m6);
    mac_out[0] = m1;
    mac_out[1] = m2;
    mac_out[2] = m3;
    mac_out[3] = m4;
    mac_out[4] = m5;
    mac_out[5] = m6;
  } while (!x);
  EnableIrqsCounting();
  return (x&2) ? GOOD : "NoAck"; // look for ARP ack.
}

word ping_sequence;
prob WizPing(quad dest_ip) {
  byte* d = (byte*)&dest_ip;
  LogDebug("PING: dest_ip=%d.%d.%d.%d ", d[0], d[1], d[2], d[3]);

  DisableIrqsCounting();
  // Socket-less Peer IP Address Register
  WizPut1(0x004c/*=SLCR*/, 0/*=Clear*/); // command
  WizPut1(0x005e/*=SL Irq Mask Reg*/, 0/*=no irqs*/);
  WizPut1(0x005f/*=SL Irq Mask Reg*/, 7/*=clear all*/);
  WizPutN(0x0050 /*=SLPIPR*/, &dest_ip, sizeof dest_ip);

  // Socketless PING command.
  LogDebug(" Ping(%x) ", ping_sequence);
  WizPut2(0x005A, ping_sequence++);
  WizPut1(0x004c/*=SLCR*/, 1/*=PING*/); // command

  byte x = 0;
  do {
    x = WizGet1(0x005f/*=SLIR socketless interrupt reg*/);
    byte m1 = WizGet1(0x0054);
    byte m2 = WizGet1(0x0055);
    byte m3 = WizGet1(0x0056);
    byte m4 = WizGet1(0x0057);
    byte m5 = WizGet1(0x0058);
    byte m6 = WizGet1(0x0059);
    LogDebug("(ping->(%x) %x:%x:%x:%x:%x:%x) ",
      x, m1, m2, m3, m4, m5, m6);
  } while (!x);
  EnableIrqsCounting();
  return (x&1) ? GOOD : "NoAck"; // look for PING ack.
}

word suggest_client_port() {
  // Pick a client port in range 0x4000 to 0xbfff.
  // That should avoid degenerate ports and well-known ports.
  word ticks = 0x7fff & WizTicks();
  return ticks + 0x4000;
}

byte find_available_socket(word* base_out) {
  byte socknum;
  word base = 0x0400;
  for (socknum = 0; socknum < 4; socknum++) {
    if (WizGet1(base+SockStatus) == 0/*=SOCK_CLOSED*/) break;
    base += 0x0100;
  }
  *base_out = base;
  LogDebug("FAS=>%x,%x", socknum, base);
  return socknum;
}

prob MacRawOpen(byte* socknum_out) {
  word base = 0;
  prob err = GOOD;
  DisableIrqsCounting();
  byte socknum = find_available_socket(&base);
  // Only socket 0 can be macraw!
  if (socknum > 0) return "NoAvailableSocket";
  *socknum_out = socknum;
    
  WizPut1(base+0x0000/*Sx_MR*/, 0x44/*=MACRAW mode with MAC Filter Enable*/);
  WizPut1(base+0x002F/*Sx_MR2*/, 0x70/*Broadcast Block, Multicast block, IPv6 block*/);
  WizPut1(base+SockCommand/*Sx_CR command reg*/, 1/*=OPEN*/);

  err = Wiz__wait(base+SockCommand, 0, 500);
  if (err) goto Enable;

  err = Wiz__wait(base+SockStatus, 0x42/*SOCK_MACRAW*/, 500);
  if (err) goto Enable;

  word tx_r = WizGet2(base+TxReadPtr);
  WizPut2(base+TxWritePtr, tx_r);
  word rx_w = WizGet2(base+0x002A/*_RX_WR*/);
  WizPut2(base+0x0028/*_RX_RD*/, rx_w);
Enable:
  EnableIrqsCounting();
  return err;
}

prob MacRawClose(byte socknum) {
  LogDebug("CLOSE: sock=%x ", socknum);
  if (socknum > 3) return "BadSockNum";

  word base = ((word)socknum + 4) << 8;
  WizPut1(base+SockCommand, 0x10/*CLOSE*/);
  Wiz__wait(base+SockCommand, 0, 500);
  WizPut1(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  return GOOD;
}

prob MacRawRecv(byte socknum, byte* buffer, word* size_in_out) {
  if (socknum > 3) return "BadSockNum";
  word base = ((word)socknum + 4) << 8;
  word buf = RX_BUF(socknum);

  byte status = WizGet1(base+SockStatus);
  if (status != 0x42/*SOCK_MACRAW*/) return "BadStatus";

  WizPut1(base+SockInterrupt, 0x1f);  // Reset interrupts.
  WizPut1(base+SockCommand, 0x40/*=RECV*/);  // RECV command.
  Wiz__wait(base+SockCommand, 0, 500);
  LogDebug("status->%x ", WizGet1(base+SockStatus));

  LogDebug(" ====== WAIT ====== ");
  while(1) {
    byte irq = WizGet1(base+SockInterrupt);
    if (irq) {
      WizPut1(base+SockInterrupt, 0x1f);  // Reset interrupts.
      if (irq != 0x04 /*=RECEIVED*/) {
        return "BadRecv";
      }
      break;
    }
  }

  word recv_size = WizGet2(base+0x0026/*_RX_RSR*/);
  word rx_rd = WizGet2(base+0x0028/*_RX_RD*/);
  word rx_wr = WizGet2(base+0x002A/*_RX_WR*/);

  word ptr = rx_rd;
  ptr &= RX_MASK;

  word buffer_size = *size_in_out;
  word i;
  for (i = 0; i < recv_size && i < buffer_size; i++) {
      buffer[i] = WizGet1(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  WizPut2(base+0x0028/*_RX_RD*/, rx_rd + recv_size);
  *size_in_out = i; 
  return GOOD;
}


prob UdpOpen(word src_port, byte* socknum_out) {
  DisableIrqsCounting();
  prob err = GOOD;
  word base;
  byte socknum = find_available_socket(&base);
  if (socknum > 3) {
    err = "NoAvailableSocket";
    goto Enable;
  }
  *socknum_out = socknum;

  WizPut1(base+SockMode, 2); // Set UDP Protocol mode.
  WizPut2(base+SockSourcePort, src_port);
  WizPut1(base+SockInterrupt/*_IR*/, 0x1F); // Clear all interrupts.
  WizPut1(base+0x002c/*_IMR*/, 0xFF); // mask all interrupts.
  WizPut2(base+0x002d/*_FRAGR*/, 0); // don't fragment.

  WizPut1(base+0x002f/*_MR2*/, 0x00); // no blocks.
  WizPut1(base+SockCommand, 1/*=OPEN*/);  // OPEN IT!
  err = Wiz__wait(base+SockCommand, 0, 500);
  if (err) {
    LogError("waitC=>%q", err);
    goto Enable;
  }

  err = Wiz__wait(base+SockStatus, 0x22/*SOCK_UDP*/, 500);
  if (err) {
    LogError("waitS=>%q", err);
    goto Enable;
  }

  word tx_r = WizGet2(base+TxReadPtr);
  WizPut2(base+TxWritePtr, tx_r);
  word rx_w = WizGet2(base+0x002A/*_RX_WR*/);
  WizPut2(base+0x0028/*_RX_RD*/, rx_w);
Enable:
  EnableIrqsCounting();
  return err;
}

prob UdpSend(byte socknum, byte* payload, word size, quad dest_ip, word dest_port) {
  LogDebug("SEND: sock=%x payload=%x size=%x ", socknum, payload, size);
  byte* d = (byte*)&dest_ip;
  LogDebug(" dest=%d.%d.%d.%d:%d(dec) ", d[0], d[1], d[2], d[3], dest_port);
  if (socknum > 3) return "NoAvailableSocket";

  word base = ((word)socknum + 4) << 8;
  word buf = TX_BUF(socknum);

  byte status = WizGet1(base+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) {
    LogError("status=$x => NOTRDY", status);
    return "NotReady";
  }

  LogDebug("dest_ip ");
  WizPutN(base+SockDestIp, &dest_ip, sizeof dest_ip);
  LogDebug("dest_p ");
  WizPut2(base+SockDestPort, dest_port);

  bool broadcast = false;
  byte send_command = 0x20;
  if (dest_ip == 0xFFFFFFFFL) {
    // Broadcast to 255.255.255.255
    broadcast = true;
    send_command = 0x21;
    WizPutN(base+6/*Sn_DHAR*/, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
  }

  { // Just verbosity:
    word r = WizGet2(base+TxReadPtr);
    LogDebug("tx_r=%x ", r);
    word w = WizGet2(base+TxWritePtr);
    LogDebug("tx_w=%x ", w);
  }
  word free = WizGet2(base + TxFreeSize);
  LogDebug("SEND: base=%x buf=%x free=%x ", base, buf, free);
  if (free < size) return NotYet;

  word tx_r = WizGet2(base+TxReadPtr);
  LogDebug("tx_r=%x ", tx_r);
  LogDebug("size=%x ", size);
  LogDebug("tx_r+size=%x ", tx_r+size);
  LogDebug("TX_SIZE=%x ", TX_SIZE);
  word offset = TX_MASK & tx_r;
  if (offset + size >= TX_SIZE) {
    // split across edges of circular buffer.
    word size1 = TX_SIZE - offset;
    word size2 = size - size1;
    WizPutN(buf + offset, payload, size1);  // 1st part
    WizPutN(buf, payload + size1, size2);   // 2nd part
  } else {
    // contiguous within the buffer.
    WizPutN(buf + offset, payload, size);  // whole thing
  }

  LogDebug("size ");
  word tx_w = WizGet2(base+TxWritePtr);
  WizPut2(base+TxWritePtr, tx_w + size);

  LogDebug("status->%x ", WizGet1(base+SockStatus));
  //sock_show(socknum);
  LogDebug("cmd:SEND ");

  WizPut1(base+SockInterrupt, 0x1f);  // Reset interrupts.
  WizPut1(base+SockCommand, send_command);  // SEND IT!
  Wiz__wait(base+SockCommand, 0, 500);
  LogDebug("status->%x ", WizGet1(base+SockStatus));

  while(1) {
    byte irq = WizGet1(base+SockInterrupt);
    if (irq&0x10) break;
  }
  WizPut1(base+SockInterrupt, 0x10);  // Reset RECV interrupt.
  return GOOD;
}

prob UdpRecvCommon(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out, bool wait) {
  if (socknum > 3) return "NoAvailableSocket";

  word base = ((word)socknum + 4) << 8;
  word buf = RX_BUF(socknum);

  byte status = WizGet1(base+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) return "NotReady";

  WizPut2(base+0x000c, 0); // clear Dest IP Addr
  WizPut2(base+0x000e, 0); // ...
  WizPut2(base+0x0010, 0); // clear Dest port addr

  WizPut1(base+SockInterrupt, 0x1f);  // Reset interrupts.
  WizPut1(base+SockCommand, 0x40/*=RECV*/);  // RECV command.
  Wiz__wait(base+SockCommand, 0, 500);
  LogDebug("status->%x ", WizGet1(base+SockStatus));

  LogDebug(" ====== WAIT ====== ");
  while(1) {
    byte irq = WizGet1(base+SockInterrupt);
    if (irq) {
      WizPut1(base+SockInterrupt, 0x1f);  // Reset interrupts.
      if (irq != 0x04 /*=RECEIVED*/) {
        return "BadRecv";
      }
      break;
    }
    if (!wait) return NotYet;
  }

  word recv_size = WizGet2(base+0x0026/*_RX_RSR*/);
  word rx_rd = WizGet2(base+0x0028/*_RX_RD*/);
  word rx_wr = WizGet2(base+0x002A/*_RX_WR*/);

  word ptr = rx_rd;
  ptr &= RX_MASK;

  struct UdpRecvHeader hdr;
  for (word i = 0; i < sizeof hdr; i++) {
      ((byte*)&hdr)[i] = WizGet1(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  
  if (hdr.len > *size_in_out) {
    LogError(" *** Packet Too Big [rs=%d. sio=%d.]\n", hdr.len, *size_in_out);
    return "TooBig";
  }

  for (word i = 0; i < hdr.len; i++) {
      payload[i] = WizGet1(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  *size_in_out = hdr.len;
  *from_addr_out = hdr.addr;
  *from_port_out = hdr.port;

  LogDebug("nando RX lib: base=%x rs=%x rd=%x wr=%x hdr.len=%x new_rx_rd=%x", base, recv_size, rx_rd, rx_wr, hdr.len,
                                   RX_MASK & (rx_rd + recv_size));
  WizPut2(base+0x0028/*_RX_RD*/, RX_MASK & (rx_rd + recv_size));

  return GOOD;
}
prob UdpRecv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out) {
  return UdpRecvCommon(socknum, payload, size_in_out, from_addr_out, from_port_out, /*wait=*/true);
}
prob UdpRecvNotYet(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out) {
  return UdpRecvCommon(socknum, payload, size_in_out, from_addr_out, from_port_out, /*wait=*/false);
}

///// ===== TCP =====

void Wiz__command(word base, byte cmd) {
  LogDebug("WIZ COMMAND: base %x cmd %x", base, cmd);
  WizPut1(base+SK_CR, cmd);
  while (WizGet1(base+SK_CR)) {}
}
// Advance to a new status after old_status.
byte Wiz__advance(word base, byte old_status) {
  byte status;
  do {
    status = WizGet1(base+SK_SR);
    LogDebug("WIZ ADVANCE: base %x old %x new %x", base, old_status, status);
  } while (status == old_status);
  return status;
}

bool Wiz__sock_command(word base, byte cmd, byte want) {
  LogDebug("COMMAND: base %x cmd %x want %x", base, cmd, want);
  WizPut1(base+SK_CR, cmd);
  while (WizGet1(base+SK_CR)) {
    LogDebug("do_sock_command %x: wait", cmd);
  }
  if (want) {
    byte status = WizGet1(base+SK_SR);
    if (status == want) return true;
    LogDebug("do_sock_command Bad Status: cmd %x got %x want %x", cmd, status, want);
    return false;
  } else {
    return true;
  }
}

prob TcpOpen(byte* socknum_out) {
  word base = 0;
  byte socknum = find_available_socket(&base);
  if (socknum > 3) {
    return "NoAvailableSocket";
  }
  *socknum_out = socknum;

  WizPut1(base+SK_MR, SK_MR_TCP); // Set TCP Protocol mode.
  WizPut1(base+SK_IR, 0xFF); // Clear all interrupts.
  WizPut1(base+SK_IMR, 0xFF); // Inhibit all interrupts.
  Wiz__command(base, SK_CR_OPEN);
  if (Wiz__advance(base, SK_SR_CLOS) != SK_SR_INIT) {
    return TcpClose(socknum), "TcpCannotOpen";
  }
  WizPut2(base+SK_TX_WR0, 0); // Does this help?
  return GOOD;
}

// Only called for TCP Client.
prob TcpDial(byte socknum, quad host, word port) {
  word base = ((word)socknum + 4) << 8;
  WizPut1(base+SK_KPALVTR, 6);  // 30 sec keepalive // ?manual says RO?
  WizPut2(base+SK_DIPR0, (word)(host>>16));
  WizPut2(base+SK_DIPR2, (word)host);
  WizPut2(base+SK_DPORTR0, port);
  Wiz__command(base, SK_CR_CONN);
  return GOOD;
}

// Only called for TCP Server.
prob TcpListen(byte socknum, word listen_port) {
  word base = ((word)socknum + 4) << 8;
  WizPut1(base+SK_KPALVTR, 6);  // 30 sec keepalive // ?manual says RO?
  WizPut2(base+SK_PORTR0, listen_port);
  Wiz__command(base, SK_CR_LSTN);
  byte a = Wiz__advance(base, SK_SR_INIT);
  if (a != SK_SR_LSTN && a != SK_SR_ESTB) return TcpClose(socknum), "TcpCannotListen";
  return GOOD;
}

// For Server or Client to accept/establish connection.
prob TcpEstablishOrNotYet(byte socknum) {
  word base = ((word)socknum + 4) << 8;

  byte ir = WizGet1(base+SK_IR);
  if (ir & SK_IR_TOUT) {
    WizPut1(base+SK_IR, SK_IR_TOUT); // Clear Timeout Interrupt.
    return "TcpTimeout";
  }
  if (ir & SK_IR_CON) {
    WizPut1(base+SK_IR, SK_IR_CON); // Clear Connection Interrupt.
  }

  byte status = WizGet1(base+SK_SR);
  switch (status) {
    case SK_SR_LSTN:  // Server: Listen mode; Syn not received yet.
    case SK_SR_SYNS:  // Client: Syn Sent.
    case SK_SR_SYNR:  // Client: Syn Recv.
      return NotYet;

    case SK_SR_ESTB:
      break;  // Established.

    case SK_SR_FINW: // FIN Wait
    case SK_SR_TIMW: // Time Wait
    case SK_SR_CLWT: // Close Wait
    case SK_SR_LACK: // Last Ack
    case SK_SR_CLOS: // Closed
      LogError("establish: TcpMoribund=%x", status);
      return TcpClose(socknum), "TcpMoribund";  // Some closing state.

    default:
      LogError("establish: TcpWeird=%x", status);
      return TcpClose(socknum), "TcpWeird";  // Something weird happened.
  }
  return GOOD;
}

prob TcpRecvOrNotYet(byte socknum, char* buf, size_t n, size_t *num_bytes_out) {
  word base = ((word)socknum + 4) << 8;
  word rxbuf = RX_BUF(socknum);

  Assert ( n < 1200 );
  LogInfo("TCPRecvOrNotYet 1 b=%x rx=%x n=%x", base, rxbuf, n);

  word bytes_waiting = WizGet2(base+SK_RX_RSR0);
  LogInfo("TCPRecvOrNotYet 2 bytes_waiting=%x", bytes_waiting);
  if (bytes_waiting == 0) {
    *num_bytes_out = 0;
    return NotYet;
  }
  if (bytes_waiting > n) {
    bytes_waiting = n;
  }
  *num_bytes_out = bytes_waiting;

  // TODO -- assimilate!

#define RING_SIZE 2048
#define RING_MASK (RING_SIZE - 1)

  word rd = WizGet2(base+SK_RX_RD0);
  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + bytes_waiting;    // end: Sum may not be beneath RING_SIZE.
  LogInfo("TCPRecvOrNotYet 3 rd=%x b=%x e=%x", rd, begin, end);

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = bytes_waiting - first_n;
    LogInfo("TCPRecvOrNotYet 4 1st=%x 2nd=%x", first_n, second_n);
    WizGetN(rxbuf+begin, buf, first_n);
    WizGetN(rxbuf, buf+first_n, second_n);
  } else {
    LogInfo("TCPRecvOrNotYet 5");
    WizGetN(rxbuf+begin, buf, bytes_waiting);
  }

  LogInfo("TCPRecvOrNotYet 6 put=%x", rd + bytes_waiting);
  WizPut2(base+SK_RX_RD0, rd + bytes_waiting);

  bool ok = Wiz__sock_command(base, SK_CR_RECV, SK_SR_ESTB);
  if (!ok) return "TcpRecvRequestBad";
  return GOOD;
}

prob TcpSendOrNotYet(byte socknum, const char* buf, size_t num_bytes_to_send) {

  word base = ((word)socknum + 4) << 8;
  word txbuf = TX_BUF(socknum);

  while (num_bytes_to_send) {
    word send_size = MIN(num_bytes_to_send , TX_SIZE);
    num_bytes_to_send -= send_size;

    // WizGet2 -> $9c53
    word get_offset = WizGet2(base+SK_TX_WR0) & TX_MASK;

    // WizGet2 -> $0800
    // TODO why does the document (p60/111) have "<="?
    while (send_size >= WizGet2(base+SK_TX_FSR0)) {
        ; // sleep or spin
    }

    // calculate Write Offset Address.
    // word get_start_address = TX_BASE + get_offset;

    word p = get_offset;
    for (size_t i = 0; i < send_size; i++) {
      WizPut1(txbuf + p, buf[i]);
      p = (p+1) & TX_MASK;
    }

    // increase SX_TX_WR as send_size
    WizPut2(base+SK_TX_WR0, send_size + WizGet2(base+SK_TX_WR0));

    // set SEND command
    bool ok = Wiz__sock_command(base, SK_CR_SEND, SK_SR_ESTB);
    if (!ok) {
      return "TcpSendFailed";
    }

    byte interrupts = WizGet1(base+SK_IR);
    while (true) {
      interrupts = WizGet1(base+SK_IR);
      if (interrupts & SK_IR_DISC) {
        WizPut1(base+SK_IR, SK_IR_DISC);  // clear bit
        return "TcpSendDisconnect";
      }
      if (interrupts & SK_IR_TOUT) {
        WizPut1(base+SK_IR, SK_IR_TOUT);  // clear bit
        return "TcpSendTimeout";
      }
      if (interrupts & SK_IR_TXOK) {
        WizPut1(base+SK_IR, SK_IR_TXOK);  // clear bit
        break; // maybe send some more.
      }
    } // end while
  } // next num_bytes_to_send
  return GOOD;
}

prob TcpClose(byte socknum) {
  word base = ((word)socknum + 4) << 8;
  Wiz__sock_command(base, SK_CR_DISC, 0); // Disconnect.
  WizPut1(base + SK_MR, SK_MR_CLSD); // Closed.
  return GOOD;
}

prob TcpEstablishBlocking(byte socknum) {
  prob e;
  do {
    e = TcpEstablishOrNotYet(socknum);
  } while (e == NotYet);
  return e;
}
prob TcpRecvBlocking(byte socknum, char* buf, size_t buflen, size_t *num_bytes_out) {
  prob e;
  do {
    e = TcpRecvOrNotYet(socknum, buf, buflen, num_bytes_out);
  } while (e == NotYet);
  return e;
}
prob TcpSendBlocking(byte socknum, const char* buf, size_t num_bytes_to_send) {
  prob e;
  do {
    e = TcpSendOrNotYet(socknum, buf, num_bytes_to_send);
  } while (e == NotYet);
  return e;
}
