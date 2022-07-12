#include <cmoc.h>
#include "frobio/wiz5100s.h"
#include "frobio/os9call.h"

// Global storage.
bool wiz_verbose;
byte* wiz_hwport = 0xFF68;  // default hardware port.

// Debugging Verbosity.
#define Say    if (wiz_verbose) printf

static word bogus_word_for_delay;
void wiz_delay(word n) {
  for (word i=0; i<n; i++) bogus_word_for_delay += i;
}

static byte peek(word reg) {
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  byte z = P3;
  Say("[%x->%2x] ", reg, z);
  return z;
}
static word peek_word(word reg) {
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  byte hi = P3;
  byte lo = P3;
  word z = ((word)(hi) << 8) + lo;
  Say("[%x->%4x] ", reg, z);
  return z;
}
static void poke(word reg, byte value) {
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  P3 = value;
  Say("[%x<=%2x] ", reg, value);
}
static void poke_word(word reg, word value) {
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  P3 = (byte)(value >> 8);
  P3 = (byte)(value);
  Say("[%x<=%4x] ", reg, value);
}
static void poke_n(word reg, void* data, word size) {
  byte* from = (byte*) data;
  P1 = (byte)(reg >> 8);
  P2 = (byte)(reg);
  Say("[%x<=== ", reg);
  for (word i=0; i<size; i++) {
    Say("%2x ", *from);
    P3 = *from++;
  }
  Say("] ");
}

// Is current time before the limit?  Using Wiz Ticks.
static byte before(word limit) {
    word t = peek_word(0x0082/*TCNTR Tick Counter*/);
    // After t crosses limit, (limit-t) "goes negative" so high bit is set.
    return 0 == ((limit-t) & 0x8000);
}
static byte wait(word reg, byte value, word millisecs_max) {
    word start = peek_word(0x0082/*TCNTR Tick Counter*/);
    word limit = 10*millisecs_max + start;
    while (before(limit)) {
        if (peek(reg) == value) {
            //// printf(" wait(%d,%d)OK ", reg, value);
            return OKAY;
        }
    }
    //// printf(" wait(%d,%d)TIMEOUT=%d ", reg, value, millisecs_max);
    return 5; // TIMEOUT
}

void wiz_reset() {
  DisableIrqs();
  P0 = 128; // Reset
  wiz_delay(42);
  P0 = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  wiz_delay(42);
  EnableIrqs();
}

void wiz_configure(quad ip_addr, quad ip_mask, quad ip_gateway) {
  DisableIrqs();

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
      wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
      poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
      poke(base+0x001e/*_RXBUF_SIZE*/, 2); // 2KB
      poke(base+0x001f/*_TXBUF_SIZE*/, 2); // 2KB
  }
  EnableIrqs();
}

error udp_close(byte socknum) {
  Say("CLOSE: sock=%x ", socknum);
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  poke(base+0x0001/*_IR*/, 0x1F); // Clear all interrupts.
  poke(base+SockCommand, 0x10/*CLOSE*/);
  wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
  poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  return OKAY;
}

word wiz_ticks() {
  word t;
  DisableIrqs();
  t = peek_word(0x0082/*TCNTR Tick Counter*/);
  EnableIrqs();
  return t; 
}

error wiz_arp(quad dest_ip, byte* mac_out) {
  byte* d = (byte*)&dest_ip;

  // Socketless ARP command.
  DisableIrqs();
  // Socket-less Peer IP Address Register
  poke(0x004c/*=SLCR*/, 0/*=Clear*/); // command
  poke(0x005e/*=SL Irq Mask Reg*/, 0/*=no irqs*/);
  poke(0x005f/*=SL Irq Mask Reg*/, 7/*=clear all*/);
  poke_n(0x0050 /*=SLPIPR*/, &dest_ip, sizeof dest_ip);

  // Socketless PING command.
  Say(" Arp ");
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
    Say("(arp->(%x) %x:%x:%x:%x:%x:%x) ",
      x, m1, m2, m3, m4, m5, m6);
    mac_out[0] = m1;
    mac_out[1] = m2;
    mac_out[2] = m3;
    mac_out[3] = m4;
    mac_out[4] = m5;
    mac_out[5] = m6;
  } while (!x);
  EnableIrqs();
  return (x&2) ? OKAY : 255; // look for ARP ack.
}

word ping_sequence = 100;
error wiz_ping(quad dest_ip) {
  byte* d = (byte*)&dest_ip;
  Say("\nPING: dest_ip=%d.%d.%d.%d ", d[0], d[1], d[2], d[3]);
  Say("SLPIPR ");

  DisableIrqs();
  // Socket-less Peer IP Address Register
  poke(0x004c/*=SLCR*/, 0/*=Clear*/); // command
  poke(0x005e/*=SL Irq Mask Reg*/, 0/*=no irqs*/);
  poke(0x005f/*=SL Irq Mask Reg*/, 7/*=clear all*/);
  poke_n(0x0050 /*=SLPIPR*/, &dest_ip, sizeof dest_ip);

  // Socketless PING command.
  Say(" Ping(%x) ", ping_sequence);
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
    Say("(ping->(%x) %x:%x:%x:%x:%x:%x) ",
      x, m1, m2, m3, m4, m5, m6);
  } while (!x);
  EnableIrqs();
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
    word base = ((word)socknum + 4) << 8;
    if (peek(base+SockStatus) == 0/*=SOCK_CLOSED*/) break;
    base += 0x0100;
  }
  *base_out = base;
  return socknum;
}

error macraw_open(byte* socknum_out) {
  word base = 0;
  byte socknum = find_available_socket(&base);
  if (socknum > 3) return 0xf0/*E_UNIT*/;
  *socknum_out = socknum;
    
  poke(base+0x0000/*Sx_MR*/, 0x44/*=MACRAW mode with MAC Filter Enable*/);
  poke(base+0x002F/*Sx_MR2*/, 0x70/*Broadcast Block, Multicast block, IPv6 block*/);
  poke(base+SockCommand/*Sx_CR command reg*/, 1/*=OPEN*/);
  wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
}

error macraw_close(byte socknum) {
  Say("CLOSE: sock=%x ", socknum);
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  poke(base+SockCommand, 0x10/*CLOSE*/);
  wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
  poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  return OKAY;
}

error macraw_recv(byte socknum, byte* buffer, word* size_in_out) {
  if (socknum > 3) return 0xf0/*E_UNIT*/;
  word base = ((word)socknum + 4) << 8;
  word buf = RX_BUF(socknum);

  byte status = peek(base+SockStatus);
  if (status != 0x42/*SOCK_MACRAW*/) return 0xf6 /*E_NOTRDY*/;

  poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(base+SockCommand, 0x40/*=RECV*/);  // RECV command.
  wait(base+SockCommand, 0, 500); // while (peek(base+0x0001)) continue;  // wait for command bit to clear.
  Say("status->%x ", peek(base+SockStatus));

  Say(" ====== WAIT ====== ");
  while(1) {
    bool v = wiz_verbose;
    wiz_verbose = 0;
    byte irq = peek(base+SockInterrupt);
    wiz_verbose = v;
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


error udp_open(word src_port, byte* socknum_out) {
  DisableIrqs();
  error err = OKAY;
  word base;
  byte socknum = find_available_socket(&base);
  if (socknum > 3) {
    err = 0xf0/*E_UNIT*/;
    goto Enable;
  }

  poke(base+SockMode, 2); // Set UDP Protocol mode.
  poke_word(base+SockSourcePort, src_port);
  poke(base+0x0001/*_IR*/, 0x1F); // Clear all interrupts.
  poke(base+0x002c/*_IMR*/, 0xFF); // mask all interrupts.
  poke_word(base+0x002d/*_FRAGR*/, 0); // don't fragment.

  // TODO: this is not enough to fix the socket for more use.
#if 0
  poke_word(base+0x0028/*_RX_RD*/, peek_word(base+0x002a)); // Start RX read pointer at RX write pointer (?)
#endif

  poke(base+0x002f/*_MR2*/, 0x00); // no blocks.
  poke(base+SockCommand, 1/*=OPEN*/);  // OPEN IT!
  err = wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
  if (err) goto Enable;

  err = wait(base+SockStatus, 0x22/*SOCK_UDP*/, 500);
  if (err) goto Enable;

  word tx_r = peek_word(base+TxReadPtr);
  poke_word(base+TxWritePtr, tx_r);
  word rx_w = peek_word(base+0x002A/*_RX_WR*/);
  poke_word(base+0x0028/*_RX_RD*/, rx_w);
Enable:
  EnableIrqs();
  return err;
}

error udp_send(byte socknum, byte* payload, word size, quad dest_ip, word dest_port) {
  Say("SEND: sock=%x payload=%x size=%x ", socknum, payload, size);
  byte* d = (byte*)&dest_ip;
  Say(" dest=%d.%d.%d.%d:%d(dec) ", d[0], d[1], d[2], d[3], dest_port);
  if (socknum > 3) return 0xf0/*E_UNIT*/;

  word base = ((word)socknum + 4) << 8;
  word buf = TX_BUF(socknum);

  byte status = peek(base+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) return 0xf6 /*E_NOTRDY*/;

  Say("dest_ip ");
  poke_n(base+SockDestIp, &dest_ip, sizeof dest_ip);
  Say("dest_p ");
  poke_word(base+SockDestPort, dest_port);

  word free = peek_word(base + TxFreeSize);
  Say("SEND: base=%x buf=%x free=%x ", base, buf, free);
  if (free < size) return 255; // no buffer room.

  word tx_r = peek_word(base+TxReadPtr);
  Say("tx_r=%x ", tx_r);
  Say("size=%x ", size);
  Say("tx_r+size=%x ", tx_r+size);
  Say("TX_SIZE=%x ", TX_SIZE);
  word offset = TX_MASK & tx_r;
  if (offset + size >= TX_SIZE) {
    // split across edges of circular buffer.
    word size1 = TX_SIZE - offset;
    word size2 = size - size1;
    poke_n(buf + offset, payload, size1);  // 1st part
    poke_n(buf, payload + size1, size2);   // 2nd part
  } else {
    // contiguous within the buffer.
    poke_n(buf + tx_r, payload, size);  // whole thing
  }

  Say("size ");
  // ?
  word tx_w = peek_word(base+TxWritePtr);
  poke_word(base+TxWritePtr, tx_w + size);
  //? poke_word(base+TxWritePtr, TX_MASK&(tx_r+size));

  Say("status->%x ", peek(base+SockStatus));
  //sock_show(socknum);
  Say("cmd:SEND ");

  poke(base+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(base+SockCommand, 0x20/*=SEND*/);  // SEND IT!
  wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
  Say("status->%x ", peek(base+SockStatus));

  while(1) {
    byte irq = peek(base+SockInterrupt);
    if (irq&0x10) break;
  }
  poke(base+SockInterrupt, 0x10);  // Reset RECV interrupt.
  return OKAY;
}

error udp_recv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out) {
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
  wait(base+SockCommand, 0, 500); // while (peek(base+SockCommand)) continue;  // wait for command bit to clear.
  Say("status->%x ", peek(base+SockStatus));

  Say(" ====== WAIT ====== ");
  while(1) {
    bool v = wiz_verbose;
    wiz_verbose = 0;
    byte irq = peek(base+SockInterrupt);
    wiz_verbose = v;
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
    printf(" *** Packet Too Big [rs=%d. sio=%d.]\n", hdr.len, *size_in_out);
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

  poke_word(base+0x0028/*_RX_RD*/, rx_rd + recv_size);

  return OKAY;
}

#if 0
Hints of how to do this correctly?

/home/strick/go/src/github.com/Wiznet/W5100S-EVB/Loopback/Eclipse/W5100S_loopback/W5100S_Loopback/src/ioLibrary_Driver/Ethernet/W5100/w5100.c

/*
@brief  This function is being called by recv() also. This function is being used for copy the data form Receive buffer of the chip to application buffer.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
It calculate the actual physical address where one has to read
the data from Receive buffer. Here also take care of the condition while it exceed
the Rx memory uper-bound of socket.
*/
void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
{
  uint16_t ptr;
  uint16_t size;
  uint16_t src_mask;
  uint16_t src_ptr;

  ptr = getSn_RX_RD(sn);

  src_mask = (uint32_t)ptr & getSn_RxMASK(sn);
  src_ptr = (getSn_RxBASE(sn) + src_mask);


  if( (src_mask + len) > getSn_RxMAX(sn) )
  {
    size = getSn_RxMAX(sn) - src_mask;
    WIZCHIP_READ_BUF((uint32_t)src_ptr, (uint8_t*)wizdata, size);
    wizdata += size;
    size = len - size;
        src_ptr = getSn_RxBASE(sn);
    WIZCHIP_READ_BUF(src_ptr, (uint8_t*)wizdata, size);
  }
  else
  {
    WIZCHIP_READ_BUF(src_ptr, (uint8_t*)wizdata, len);
  }

  ptr += len;

  setSn_RX_RD(sn, ptr);
}

void wiz_recv_ignore(uint8_t sn, uint16_t len)
{
  uint16_t ptr;

  ptr = getSn_RX_RD(sn);

  ptr += len;
  setSn_RX_RD(sn,ptr);
}
#endif

void sock_show(byte socknum) {
  bool v = wiz_verbose;

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
