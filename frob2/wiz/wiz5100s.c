#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/wiz/wiz5100s.h"

typedef const char* prob;
#define GOOD (const char*)(NULL)
#define NOT_YET "~"

// Global storage.
byte* wiz_hwport = 0xFF68;  // default hardware port.
word ping_sequence;

static word bogus_word_for_delay;
void wiz_delay(word n) {
  for (word i=0; i<n; i++) bogus_word_for_delay += i;
}

static byte peek(word reg) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  byte z = P3;
    EnableIrqsCounting();
  LogDebug("[%4x->%2x]", reg, z);
  return z;
}
static word peek_word(word reg) {
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
static void poke(word reg, byte value) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  P3 = value;
    EnableIrqsCounting();
  LogDebug("[%4x<-%2x]", reg, value);
}
static void poke_word(word reg, word value) {
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  P3 = (byte)(value >> 8);
  P3 = (byte)(value);
    EnableIrqsCounting();
  LogDebug("[%4x<=%4x]", reg, value);
}
static void poke_n(word reg, const void* data, word size) {
  const byte* from = (const byte*) data;
    DisableIrqsCounting();
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  for (word i=0; i<size; i++) {
    P3 = *from++;
  }
  EnableIrqsCounting();
}

// wiz_ticks: 0.1ms but may have readbyte,readbyte error?
word wiz_ticks() {
    word t = peek_word(0x0082/*TCNTR Tick Counter*/);
    return t;
}

// Is current time before the limit?  Using Wiz Ticks.
static byte before(word limit) {
    word t = peek_word(0x0082/*TCNTR Tick Counter*/);
    // After t crosses limit, (limit-t) "goes negative" so high bit is set.
    return 0 == ((limit-t) & 0x8000);
}
static byte wait(word reg, byte value, word millisecs_max) {
    LogDebug("Wait reg %x value %x ms %x", reg, value, millisecs_max);
    word start = peek_word(0x0082/*TCNTR Tick Counter*/);
    word limit = 10*millisecs_max + start;
    while (before(limit)) {
        if (peek(reg) == value) {
            LogDebug("Wait OK");
            return OKAY;
        }
    }
    LogDebug("Wait Timeout");
    return 5; // TIMEOUT
}

void wiz_reset() {
  DisableIrqsCounting();
  P0 = 128; // Reset
  wiz_delay(42);
  P0 = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  wiz_delay(42);
  EnableIrqsCounting();
}

void wiz_configure_for_DHCP(const char* name4, byte* hw6_out) {
  wiz_configure(0L, 0xFFFFFF00, 0L);

  // Create locally assigned mac_addr from name4.
  byte mac_addr[6] = {2, 32, 0, 0, 0, 0};  // local prefix.
  strncpy((char*)mac_addr+2, name4, 4);
  poke_n(0x0009/*ether_mac*/, mac_addr, 6);
  memcpy(hw6_out, mac_addr, 6);
}

void wiz_reconfigure_for_DHCP(quad ip_addr, quad ip_mask, quad ip_gateway) {
  P1 = 0; P2 = 1;  // start at addr 0x0001: Gateway IP.
  poke_n(0x0001/*gateway*/, &ip_gateway, 4);
  poke_n(0x0005/*mask*/, &ip_mask, 4);
  poke_n(0x000f/*ip_addr*/, &ip_addr, 4);
  // Keep the same mac_addr.
}
void wiz_configure(quad ip_addr, quad ip_mask, quad ip_gateway) {
  DisableIrqsCounting();

  P1 = 0; P2 = 1;  // start at addr 0x0001: Gateway IP.
  poke_n(0x0001/*gateway*/, &ip_gateway, 4);
  poke_n(0x0005/*mask*/, &ip_mask, 4);
  poke_n(0x000f/*ip_addr*/, &ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
  byte mac_addr[6] = {2, 0, 0, 0, 0, 0};  // local prefix.
  *(quad*)(mac_addr+2) = ip_addr;
  poke_n(0x0009/*ether_mac*/, mac_addr, 6);

  poke(0x001a/*=Rx Memory Size*/, 0x55); // 2k per sock
  poke(0x001b/*=Tx Memory Size*/, 0x55); // 2k per sock

  // Force all 4 sockets to be closed.
  for (byte socknum=0; socknum<4; socknum++) {
      word base = ((word)socknum + 4) << 8;
      poke(base+SockCommand, 0x10/*CLOSE*/);
      wait(base+SockCommand, 0, 500);
      poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
      poke(base+0x001e/*_RXBUF_SIZE*/, 2); // 2KB
      poke(base+0x001f/*_TXBUF_SIZE*/, 2); // 2KB
  }
  EnableIrqsCounting();
}

errnum udp_close(byte socknum) {
  LogDebug("CLOSE: sock=%x ", socknum);
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  poke(base+0x0001/*_IR*/, 0x1F); // Clear all interrupts.
  poke(base+SockCommand, 0x10/*CLOSE*/);
  wait(base+SockCommand, 0, 500);
  poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  return OKAY;
}

errnum wiz_arp(quad dest_ip, byte* mac_out) {
  byte* d = (byte*)&dest_ip;

  // Socketless ARP command.
  DisableIrqsCounting();
  // Socket-less Peer IP Address Register
  poke(0x004c/*=SLCR*/, 0/*=Clear*/); // command
  poke(0x005e/*=SL Irq Mask Reg*/, 0/*=no irqs*/);
  poke(0x005f/*=SL Irq Mask Reg*/, 7/*=clear all*/);
  poke_n(0x0050 /*=SLPIPR*/, &dest_ip, sizeof dest_ip);

  // Socketless PING command.
  poke(0x004c/*=SLCR*/, 2/*=ARP*/); // command

  byte x = 0;
  do {
    poke(0x005f, 0); // clear interrupt reg

    wiz_delay(42);
    x = peek(0x005f/*=SLIR socketless interrupt reg*/);
    byte m1 = peek(0x0054);
    byte m2 = peek(0x0055);
    byte m3 = peek(0x0056);
    byte m4 = peek(0x0057);
    byte m5 = peek(0x0058);
    byte m6 = peek(0x0059);
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
  return (x&2) ? OKAY : 255; // look for ARP ack.
}

errnum wiz_ping(quad dest_ip) {
  byte* d = (byte*)&dest_ip;
  LogDebug("PING: dest_ip=%d.%d.%d.%d ", d[0], d[1], d[2], d[3]);

  DisableIrqsCounting();
  // Socket-less Peer IP Address Register
  poke(0x004c/*=SLCR*/, 0/*=Clear*/); // command
  poke(0x005e/*=SL Irq Mask Reg*/, 0/*=no irqs*/);
  poke(0x005f/*=SL Irq Mask Reg*/, 7/*=clear all*/);
  poke_n(0x0050 /*=SLPIPR*/, &dest_ip, sizeof dest_ip);

  // Socketless PING command.
  LogDebug(" Ping(%x) ", ping_sequence);
  poke_word(0x005A, ping_sequence++);
  poke(0x004c/*=SLCR*/, 1/*=PING*/); // command

  byte x = 0;
  do {
    x = peek(0x005f/*=SLIR socketless interrupt reg*/);
    byte m1 = peek(0x0054);
    byte m2 = peek(0x0055);
    byte m3 = peek(0x0056);
    byte m4 = peek(0x0057);
    byte m5 = peek(0x0058);
    byte m6 = peek(0x0059);
    LogDebug("(ping->(%x) %x:%x:%x:%x:%x:%x) ",
      x, m1, m2, m3, m4, m5, m6);
  } while (!x);
  EnableIrqsCounting();
  return (x&1) ? OKAY : 255; // look for PING ack.
}

word suggest_client_port() {
  // Pick a client port in range 0x4000 to 0xbfff.
  // That should avoid degenerate ports and well-known ports.
  word ticks = 0x7fff & wiz_ticks();
  return ticks + 0x4000;
}

byte find_available_socket(word* base_out) {
  byte socknum;
  word base = 0x0400;
  for (socknum = 0; socknum < 4; socknum++) {
    if (peek(base+SockStatus) == 0/*=SOCK_CLOSED*/) break;
    base += 0x0100;
  }
  *base_out = base;
  LogDebug("FAS=>%x,%x", socknum, base);
  return socknum;
}

errnum macraw_open(byte* socknum_out) {
  word base = 0;
  errnum err = OKAY;
  DisableIrqsCounting();
  byte socknum = find_available_socket(&base);
  // Only socket 0 can be macraw!
  if (socknum > 0) return 0xf0/*E_UNIT*/;
  *socknum_out = socknum;
    
  poke(base+0x0000/*Sx_MR*/, 0x44/*=MACRAW mode with MAC Filter Enable*/);
  poke(base+0x002F/*Sx_MR2*/, 0x70/*Broadcast Block, Multicast block, IPv6 block*/);
  poke(base+SockCommand/*Sx_CR command reg*/, 1/*=OPEN*/);

  err = wait(base+SockCommand, 0, 500);
  if (err) goto Enable;

  err = wait(base+SockStatus, 0x42/*SOCK_MACRAW*/, 500);
  if (err) goto Enable;

  word tx_r = peek_word(base+TxReadPtr);
  poke_word(base+TxWritePtr, tx_r);
  word rx_w = peek_word(base+0x002A/*_RX_WR*/);
  poke_word(base+0x0028/*_RX_RD*/, rx_w);
Enable:
  EnableIrqsCounting();
  return err;
}

errnum macraw_close(byte socknum) {
  LogDebug("CLOSE: sock=%x ", socknum);
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  poke(base+SockCommand, 0x10/*CLOSE*/);
  wait(base+SockCommand, 0, 500);
  poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  return OKAY;
}

errnum macraw_recv(byte socknum, byte* buffer, word* size_in_out) {
  if (socknum > 3) return 0xf0/*E_UNIT*/;
  word base = ((word)socknum + 4) << 8;
  word buf = RX_BUF(socknum);

  byte status = peek(base+SockStatus);
  if (status != 0x42/*SOCK_MACRAW*/) return 0xf6 /*E_NOTRDY*/;

  poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(base+SockCommand, 0x40/*=RECV*/);  // RECV command.
  wait(base+SockCommand, 0, 500);
  LogDebug("status->%x ", peek(base+SockStatus));

  LogDebug(" ====== WAIT ====== ");
  while(1) {
    byte irq = peek(base+SockInterrupt);
    if (irq) {
      poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
      if (irq != 0x04 /*=RECEIVED*/) {
        return 0xf4/*=E_READ*/;
      }
      break;
    }
  }

  word recv_size = peek_word(base+0x0026/*_RX_RSR*/);
  word rx_rd = peek_word(base+0x0028/*_RX_RD*/);
  word rx_wr = peek_word(base+0x002A/*_RX_WR*/);

  word ptr = rx_rd;
  ptr &= RX_MASK;

  word buffer_size = *size_in_out;
  word i;
  for (i = 0; i < recv_size && i < buffer_size; i++) {
      buffer[i] = peek(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  poke_word(base+0x0028/*_RX_RD*/, rx_rd + recv_size);
  *size_in_out = i; 
  return OKAY;
}


errnum udp_open(word src_port, byte* socknum_out) {
  DisableIrqsCounting();
  errnum err = OKAY;
  word base;
  byte socknum = find_available_socket(&base);
  if (socknum > 3) {
    err = 0xf0/*E_UNIT*/;
    goto Enable;
  }
  *socknum_out = socknum;

  poke(base+SockMode, 2); // Set UDP Protocol mode.
  poke_word(base+SockSourcePort, src_port);
  poke(base+0x0001/*_IR*/, 0x1F); // Clear all interrupts.
  poke(base+0x002c/*_IMR*/, 0xFF); // mask all interrupts.
  poke_word(base+0x002d/*_FRAGR*/, 0); // don't fragment.

  poke(base+0x002f/*_MR2*/, 0x00); // no blocks.
  poke(base+SockCommand, 1/*=OPEN*/);  // OPEN IT!
  err = wait(base+SockCommand, 0, 500);
  if (err) {
    LogError("waitC=>%x", err);
    goto Enable;
  }

  err = wait(base+SockStatus, 0x22/*SOCK_UDP*/, 500);
  if (err) {
    LogError("waitS=>%x", err);
    goto Enable;
  }

  word tx_r = peek_word(base+TxReadPtr);
  poke_word(base+TxWritePtr, tx_r);
  word rx_w = peek_word(base+0x002A/*_RX_WR*/);
  poke_word(base+0x0028/*_RX_RD*/, rx_w);
Enable:
  EnableIrqsCounting();
  return err;
}

errnum udp_send(byte socknum, byte* payload, word size, quad dest_ip, word dest_port) {
  LogDebug("SEND: sock=%x payload=%x size=%x ", socknum, payload, size);
  byte* d = (byte*)&dest_ip;
  LogDebug(" dest=%d.%d.%d.%d:%d(dec) ", d[0], d[1], d[2], d[3], dest_port);
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  word buf = TX_BUF(socknum);

  byte status = peek(base+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) {
    LogError("status=$x => NOTRDY", status);
    return 0xf6 /*E_NOTRDY*/;
  }

  LogDebug("dest_ip ");
  poke_n(base+SockDestIp, &dest_ip, sizeof dest_ip);
  LogDebug("dest_p ");
  poke_word(base+SockDestPort, dest_port);

  bool broadcast = false;
  byte send_command = 0x20;
  if (dest_ip == 0xFFFFFFFFL) {
    // Broadcast to 255.255.255.255
    broadcast = true;
    send_command = 0x21;
    poke_n(base+6/*Sn_DHAR*/, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
  }

  { // Just verbosity:
    word r = peek_word(base+TxReadPtr);
    LogDebug("tx_r=%x ", r);
    word w = peek_word(base+TxWritePtr);
    LogDebug("tx_w=%x ", w);
  }
  word free = peek_word(base + TxFreeSize);
  LogDebug("SEND: base=%x buf=%x free=%x ", base, buf, free);
  if (free < size) return 255; // no buffer room.

  word tx_r = peek_word(base+TxReadPtr);
  LogDebug("tx_r=%x ", tx_r);
  LogDebug("size=%x ", size);
  LogDebug("tx_r+size=%x ", tx_r+size);
  LogDebug("TX_SIZE=%x ", TX_SIZE);
  word offset = TX_MASK & tx_r;
  if (offset + size >= TX_SIZE) {
    // split across edges of circular buffer.
    word size1 = TX_SIZE - offset;
    word size2 = size - size1;
    poke_n(buf + offset, payload, size1);  // 1st part
    poke_n(buf, payload + size1, size2);   // 2nd part
  } else {
    // contiguous within the buffer.
    poke_n(buf + offset, payload, size);  // whole thing
  }

  LogDebug("size ");
  word tx_w = peek_word(base+TxWritePtr);
  poke_word(base+TxWritePtr, tx_w + size);

  LogDebug("status->%x ", peek(base+SockStatus));
  //sock_show(socknum);
  LogDebug("cmd:SEND ");

  poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(base+SockCommand, send_command);  // SEND IT!
  wait(base+SockCommand, 0, 500);
  LogDebug("status->%x ", peek(base+SockStatus));

  while(1) {
    byte irq = peek(base+SockInterrupt);
    if (irq&0x10) break;
  }
  poke(base+SockInterrupt, 0x10);  // Reset RECV interrupt.
  return OKAY;
}

errnum udp_recv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out) {
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  word buf = RX_BUF(socknum);

  byte status = peek(base+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) return 0xf6 /*E_NOTRDY*/;

  poke_word(base+0x000c, 0); // clear Dest IP Addr
  poke_word(base+0x000e, 0); // ...
  poke_word(base+0x0010, 0); // clear Dest port addr

  poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(base+SockCommand, 0x40/*=RECV*/);  // RECV command.
  wait(base+SockCommand, 0, 500);
  LogDebug("status->%x ", peek(base+SockStatus));

  LogDebug(" ====== WAIT ====== ");
  while(1) {
    byte irq = peek(base+SockInterrupt);
    if (irq) {
      poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
      if (irq != 0x04 /*=RECEIVED*/) {
        return 0xf4/*=E_READ*/;
      }
      break;
    }
  }

  word recv_size = peek_word(base+0x0026/*_RX_RSR*/);
  word rx_rd = peek_word(base+0x0028/*_RX_RD*/);
  word rx_wr = peek_word(base+0x002A/*_RX_WR*/);

  word ptr = rx_rd;
  ptr &= RX_MASK;

  struct UdpRecvHeader hdr;
  for (word i = 0; i < sizeof hdr; i++) {
      ((byte*)&hdr)[i] = peek(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  
  if (hdr.len > *size_in_out) {
    LogError(" *** Packet Too Big [rs=%d. sio=%d.]\n", hdr.len, *size_in_out);
    return 0xed/*E_NORAM*/;
  }

  for (word i = 0; i < hdr.len; i++) {
      payload[i] = peek(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  *size_in_out = hdr.len;
  *from_addr_out = hdr.addr;
  *from_port_out = hdr.port;

  LogDebug("nando RX lib: base=%x rs=%x rd=%x wr=%x hdr.len=%x new_rx_rd=%x", base, recv_size, rx_rd, rx_wr, hdr.len,
                                   RX_MASK & (rx_rd + recv_size));
  poke_word(base+0x0028/*_RX_RD*/, RX_MASK & (rx_rd + recv_size));

  return OKAY;
}

#if 0
void sock_show(byte socknum) {
  bool v = Verbose;

  if (v) {
      wiz_verbose = 0;
      word base = ((word)socknum + 4) << 8;
      for (byte i = 0; i < 64; i+=16) {
        printf("\n%04x: ", base+i);
        for (byte j = 0; j < 16; j++) {
          byte k = i+j;
          printf("%02x ", peek(base+k));
          if ((j&3)==3) printf(" ");
        }
      }
      wiz_verbose = v;
  }
}
#endif

///// ===== TCP =====

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/wiz/wiz5100s.h"
#include "frob2/wiz/w5100s_defs.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

static bool sock_command(word base, byte cmd, byte want) {
  poke(base+SK_CR, SK_CR_OPEN);  // Open Command.
  while (peek(base+SK_CR)) {
    LogDebug("do_sock_command %x: wait", cmd);
  }
  if (want) {
    byte status = peek(base+SK_SR);
    if (status == want) return true;
    LogDebug("check_sock_status: cmd $x got %x want %x", cmd, status, want);
    return false;
  } else {
    return true;
  }
}

prob tcp_open_server(word listen_port, byte* socknum_out) {
  errnum err = OKAY;
  word base;

  byte socknum = find_available_socket(&base);
  if (socknum > 3) {
    err = 0xf0/*E_UNIT*/;
    return "NoAvailableSocket";
  }
  *socknum_out = socknum;

  while (true) {
    poke(base+SK_MR, SK_MR_TCP); // Set TCP Protocol mode.
    poke_word(base+SK_PORTR0, listen_port);
    poke(base+SK_IR, 0xFF); // Clear all interrupts.
    poke(base+SK_IMR, 0xFF); // mask all interrupts.

    if (!sock_command(base, SK_CR_OPEN, SK_SR_INIT)) continue;

    if (!sock_command(base, SK_CR_LSTN, SK_SR_LSTN)) continue;

  }
  return OKAY;
}

#if 0
--errnum tcp_accept(byte socknum, bool* accepted_out) {
  *accepted_out = false;
  word base = ((word)socknum + 4) << 8;
  byte ir = peek(base+SK_IR);
  if (ir & SK_IR_TOUT) {
    poke(base+SK_IR, SK_IR_TOUT);
    return E_TIMEOUT;
  }
  if (ir & SK_IR_CON) {
    poke(base+SK_IR, SK_IR_CON);
    *accepted_out = true;
  }
  return OKAY;
--}
#endif

// Error Compromise:
//   Use literal "const char*" for errors.
//   Use literal "~" for Not Yet.
//   Use literal GOOD for good status.
// To communicate better error messages, application can LogError.

prob tcp_accept(byte socknum) {
  word base = ((word)socknum + 4) << 8;
  byte ir = peek(base+SK_IR);
  if (ir & SK_IR_TOUT) {
    poke(base+SK_IR, SK_IR_TOUT);
    return "TcpAcceptTimeout";
  }
  if (ir & SK_IR_CON) { // test interrupt bit.
    poke(base+SK_IR, SK_IR_CON); // clear interrrupt bit.
    return GOOD;  // Good, Accepted.
  }
  return NOT_YET;
}

prob tcp_recv(byte socknum, char* buf, size_t buflen, size_t *num_bytes_out) {
  word base = ((word)socknum + 4) << 8;
  word rxbuf = RX_BUF(socknum);

  word get_size = peek_word(base+SK_RX_RSR0);
  if (get_size == 0) {
    return NOT_YET;
  }

  word get_offset = peek_word(base+SK_RX_RD0) & RX_MASK;
  word get_start_address = rxbuf + get_offset;

  word n = MIN(get_size, buflen);

  word p = get_offset;
  for (size_t i = 0; i < n; i++) {
    byte x = peek(rxbuf + p);
    p = (p+1) & RX_MASK;
  }

  poke_word(base+SK_RX_RD0, n + peek_word(base+SK_RX_RD0));

  bool ok = sock_command(base, SK_CR_RECV, SK_SR_ESTB);
  if (!ok) return "BROKEN";
  return GOOD;
}

prob tcp_send(byte socknum, char* buf, size_t num_bytes_to_send) {

  word base = ((word)socknum + 4) << 8;
  word txbuf = TX_BUF(socknum);

  while (num_bytes_to_send) {
    word send_size = MIN(num_bytes_to_send , TX_SIZE);
    num_bytes_to_send -= send_size;

    word get_offset = peek_word(base+SK_TX_WR0) & TX_MASK;

    while (send_size <= peek_word(base+SK_TX_FSR0)) {
        ; // sleep or spin
    }

    // calculate Write Offset Address.
    // word get_start_address = TX_BASE + get_offset;

    word p = get_offset;
    for (size_t i = 0; i < send_size; i++) {
      poke(txbuf + p, buf[i]);
      p = (p+1) & TX_MASK;
    }

    // increase SX_TX_WR as send_size
    poke_word(base+SK_TX_WR0, send_size + peek_word(base+SK_TX_WR0));

    // set SEND command
    bool ok = sock_command(base, SK_CR_SEND, SK_SR_ESTB);
    if (!ok) {
      return "TcpSendFailed";
    }

    byte interrupts = peek(base+SK_IR);
    while (true) {
      interrupts = peek(base+SK_IR);
      if (interrupts & SK_IR_DISC) {
        poke(base+SK_IR, SK_IR_DISC);  // clear bit
        return "TcpSendDisconnect";
      }
      if (interrupts & SK_IR_TOUT) {
        poke(base+SK_IR, SK_IR_TOUT);  // clear bit
        return "TcpSendTimeout";
      }
      if (interrupts & SK_IR_TXOK) {
        poke(base+SK_IR, SK_IR_TXOK);  // clear bit
        break; // maybe send some more.
      }
    } // end while
  } // next num_bytes_to_send
  return GOOD;
}

prob tcp_close(byte socknum) {
  word base = ((word)socknum + 4) << 8;
  sock_command(base, SK_CR_DISC, 0); // Disconnect.
  poke(base + SK_MR, SK_MR_CLSD); // Closed.
  return GOOD;
}
