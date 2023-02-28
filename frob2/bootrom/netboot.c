// Boot ROM code for CocoIO 
//
// to compile with CMOC and launch with `preboot.asm`.

#define WANT_PACKIN_PACKOUT 1

#include "frob2/bootrom/bootrom.h"

/////////////////////////////////////////////////
//   Copied & modified from frob2/wiz/wiz5100s.h
// Keep this at the default of 2K for each.
#define TX_SIZE 2048
#define RX_SIZE 2048
#define TX_MASK (TX_SIZE - 1)
#define RX_MASK (RX_SIZE - 1)
#define TX_BUF_0  0x4000
#define RX_BUF_0  0x6000

#define BASE 0x0400 // Wiz Socket 0 control base address

// Socket register offsets:
#define SockMode 0x00
#define SockCommand 0x01
#define SockInterrupt 0x02
#define SockStatus 0x03
#define SockSourcePort 0x04
#define SockDestIp 0x0C
#define SockDestPort 0x10
#define TxFreeSize 0x20
#define TxReadPtr 0x22
#define TxWritePtr 0x24
#define RxSize 0x26
#define RxReadPtr 0x28
#define RxWritePtr 0x2A

struct UdpRecvHeader {
    byte addr[4];
    word port;
    word len;
};

/////////////////////////////////////////////////


void Delay(word n) {
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
}

bool IsBroadcast(const byte* ip) {
  return ip[0]==255;  // Net 255 is enough to check.
}

static byte peek(word reg) {
  volatile struct wiz_port* wiz = WIZ_PORT;
  wiz->addr_hi = (byte)(reg >> 8);
  wiz->addr_lo = (byte)(reg);
  byte z = wiz->data;
  if (reg < 0x1000) printk("%x->%x", reg, z);
  return z;
}
static word peek_word(word reg) {
  volatile struct wiz_port* wiz = WIZ_PORT;
  wiz->addr_hi = (byte)(reg >> 8);
  wiz->addr_lo = (byte)(reg);
  byte z_hi = wiz->data;
  byte z_lo = wiz->data;
  word z = ((word)(z_hi) << 8) + (word)z_lo;
  if (reg < 0x1000) printk("%x=>%x", reg, z);
  return z;
}
static void poke(word reg, byte value) {
  volatile struct wiz_port* wiz = WIZ_PORT;
  wiz->addr_hi = (byte)(reg >> 8);
  wiz->addr_lo = (byte)(reg);
  wiz->data = value;
  if (reg < 0x1000) printk("%x<-%x", reg, value);
}
static void poke_word(word reg, word value) {
  volatile struct wiz_port* wiz = WIZ_PORT;
  wiz->addr_hi = (byte)(reg >> 8);
  wiz->addr_lo = (byte)(reg);
  wiz->data = (byte)(value >> 8);
  wiz->data = (byte)(value);
  if (reg < 0x1000) printk("%x<=%x", reg, value);
}
static void poke_n(word reg, const void* data, word size) {
  volatile struct wiz_port* wiz = WIZ_PORT;
  printk("[%x<<%x]", reg, size);
  const byte* from = (const byte*) data;
  wiz->addr_hi = (byte)(reg >> 8);
  wiz->addr_lo = (byte)(reg);
  for (word i=0; i<size; i++) {
    wiz->data = *from++;
  }
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    word z = peek_word(0x0082/*TCNTR Tick Counter*/);
    printk("t=%x", z);
    // TODO: This is here to catch the case that the wiz chip stopped.
    // It can false positive.
    if (z==0) Fatal("0T", z);
    return z;
}

bool Wait(word reg, byte value, const char* wut) {
    printk("2Wait(%x,%x,%s)", reg, value, wut);
    for (byte i = 0; i < 50; i++) {
      Delay(200);
      byte got = peek(reg);
      printk("%x", got);
      if (got == value) {
        printk("YES");
        return true;
      }
    }
    Fatal("2WAIT", reg);
} 
// Wait up to 5 seconds for reg to have value.
// Return true if it got the value.
// Return false if it timed out.
bool BadWait(word reg, byte value, const char* wut) {
    printk("Wait(%x,%x,%s)", reg, value, wut);

    word start = WizTicks();

    byte pregot = peek(reg);
    printk("pregot %x", pregot);
    while (true) {
      Delay(10);
      byte got = peek(reg);
      if (got == value) return true;

      word now = WizTicks();
      word duration = now - start;
      printk("%x-%x=dur=%x", now, start, duration);
      if (duration > 50000) {
        printk("got %x", got);
        return false;
      }
    }
}
void WaitOrFatal(word reg, byte value, const char* wut) {
  printk("WOF:%x,%x,%s", reg, value, wut);
  if (!Wait(reg, value, wut)) {
    printk("WOF2:%x,%x,%s", reg, value, wut);
    Fatal(wut, reg);
  }
}

// For any socket
void IssueCommandAtBase(byte cmd, word base) {
      poke(base+SockCommand, cmd);
      for (byte i = 0; i < 250; i++) {
        if (peek(base+SockCommand) == 0) return;
      }

      // We must be stuck.
      Fatal("IC", cmd);
}
// For socket 0
void IssueCommand(byte cmd) {
      poke(BASE+SockCommand, cmd);
      for (byte i = 0; i < 250; i++) {
        if (peek(BASE+SockCommand) == 0) return;
      }

      // We must be stuck.
      Fatal("IC", cmd);
}

void WizReset() {
  PutStr("WizReset ");
  volatile struct wiz_port* wiz = WIZ_PORT;
  wiz->command = 128; // Reset
L Delay(9000);
  wiz->command = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
L Delay(9000);
  //want// poke_word(0x0017, 10000);  // 1 sec retransmission time.

}

void WizConfigure(const byte* ip_addr, const byte* ip_mask, const byte* ip_gateway) {
L poke_n(0x0001/*gateway*/, ip_gateway, 4);
L poke_n(0x0005/*mask*/, ip_mask, 4);
L poke_n(0x000f/*ip_addr*/, ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
L poke_n(0x0009/*ether_mac*/, Vars->mac_addr, 6);

L poke(0x001a/*=Rx Memory Size*/, 0x55); // 2k per sock
L poke(0x001b/*=Tx Memory Size*/, 0x55); // 2k per sock
                                         //
#if 1
  // Force all 4 sockets to be closed.
  for (word base=0x0400; base<0x0800; base+=0x0100) {
L     IssueCommandAtBase( 0x10/*CLOSE*/, base);
L     poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
  }
#endif
}

#if BR_DHCP
const byte BroadcastAddr[4] = {255, 255, 255, 255};
const byte ZeroAddr[4] = {0, 0, 0, 0};
const byte ClassA[4] = {255, 0, 0, 0};
void wiz_configure_for_DHCP(const char* name4, byte* hw6_out) {
  WizConfigure(ZeroAddr, ClassA, ZeroAddr);

  // Create locally assigned mac_addr from name4.
  byte mac_addr[6] = {2, 32, 0, 0, 0, 0};  // local prefix.
  strncpy((char*)mac_addr+2, name4, 4);
  poke_n(0x0009/*ether_mac*/, mac_addr, 6);
  memcpy(hw6_out, mac_addr, 6);
}

void WizReconfigureForDhcp(const byte* ip_addr, const byte* ip_mask, const byte* ip_gateway) {
  printk("reconfigure");
  poke_n(0x0001/*gateway*/, ip_gateway, 4);
  poke_n(0x0005/*mask*/, ip_mask, 4);
  poke_n(0x000f/*ip_addr*/, ip_addr, 4);
  // Keep the same mac_addr.
}
#endif

void UdpClose() {
  printk("CLOSE");

  poke(BASE+0x0002/*_IR*/, 0x1F); // Clear all interrupts.
  IssueCommand( 0x10/*CLOSE*/);
  poke(BASE+SockMode, 0x00/*Protocol: Socket Closed*/);
}

errnum UdpOpen(word src_port) {
  printk("UdpOpen %u....", src_port);
  poke(BASE+SockMode, 2); // Set UDP Protocol mode.
  poke_word(BASE+SockSourcePort, src_port);
  poke(BASE+0x0002/*_IR*/, 0x1F); // Clear all interrupts.
  poke(BASE+0x002c/*_IMR*/, 0xFF); // mask all interrupts.
  IssueCommand( 1/*=OPEN*/);  // OPEN IT!
L  
  Delay(100);
  WaitOrFatal(BASE+SockStatus, 0x22/*SOCK_UDP*/, "UO");
L
  // Must we do this?
  word tx_r = peek_word(BASE+TxReadPtr);
  poke_word(BASE+TxWritePtr, tx_r);
  word rx_w = peek_word(BASE+0x002A/*_RX_WR*/);
  poke_word(BASE+0x0028/*_RX_RD*/, rx_w);
  // Must we do that?
  printk("UdpOpen %x OKAY", src_port);
  return OKAY;
}

errnum UdpSend(byte* payload, word size, const byte* dest_ip, word dest_port) {
  printk("SEND: pay=%x size=%x ", payload, size);
  printk("dest=%a:%d", dest_ip, dest_port);

  word buf = TX_BUF_0;

  byte status = peek(BASE+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) {
    printk("bad st%x", status);
    return 0xf6 /*E_NOTRDY*/;
  }

  poke_n(BASE+SockDestIp, dest_ip, 4);
  poke_word(BASE+SockDestPort, dest_port);

  bool broadcast = false;
  byte send_command = 0x20;
  if (IsBroadcast(dest_ip)) {
    // Broadcast to 255.255.255.255
    broadcast = true;
    send_command = 0x21;
    // TODO: when do we undo this?
    poke_n(BASE+6/*Sn_DHAR*/, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
  }

  word free = peek_word(BASE + TxFreeSize);
  printk("SEND: buf=%x free=%x ", buf, free);
  if (free < size) return 255; // no buffer room.

  word tx_r = peek_word(BASE+TxReadPtr);
  printk("tx_r=%x ", tx_r);
  printk("size=%x ", size);
  printk("tx_r+size=%x ", tx_r+size);
  printk("TX_SIZE=%x ", TX_SIZE);
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

  printk("size ");
  word tx_w = peek_word(BASE+TxWritePtr);
  poke_word(BASE+TxWritePtr, tx_w + size);

  printk("st%x ", peek(BASE+SockStatus));
  //sock_show(socknum);
  printk("cmd:SEND ");

  poke(BASE+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(BASE+SockCommand, send_command);  // SEND IT!
  WaitOrFatal(BASE+SockCommand, 0, "US");
  printk("status->%x ", peek(BASE+SockStatus));

  while(1) {
    byte irq = peek(BASE+SockInterrupt);
    if (irq&0x10) break;
  }
  poke(BASE+SockInterrupt, 0x10);  // Reset RECV interrupt.
  return OKAY;
}

errnum UdpRecv(byte* payload, word* size_in_out, byte* from_addr_out, word* from_port_out) {
  word buf = RX_BUF_0;

  printk("st%x", peek(BASE+SockStatus));
  byte status = peek(BASE+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) return 0xf6 /*E_NOTRDY*/;

  poke_word(BASE+0x000c, 0); // clear Dest IP Addr
  poke_word(BASE+0x000e, 0); // ...
  poke_word(BASE+0x0010, 0); // clear Dest port addr

  poke(BASE+SockInterrupt, 0x1f);  // Reset interrupts.
  IssueCommand( 0x40/*=RECV*/);  // RECV command.

  printk("status->%x ", peek(BASE+SockStatus));

  printk(" < RECV > ");
  while(1) {
    byte irq = peek(BASE+SockInterrupt);
    if (irq) {
      poke(BASE+SockInterrupt, 0x1f);  // Reset interrupts.
      if (irq != 0x04 /*=RECEIVED*/) {
        Fatal("URQ", irq);
      }
      break;
    }
  }

  word recv_size = peek_word(BASE+0x0026/*_RX_RSR*/);
  word rx_rd = peek_word(BASE+0x0028/*_RX_RD*/);
  word rx_wr = peek_word(BASE+0x002A/*_RX_WR*/);

  word ptr = rx_rd;
  ptr &= RX_MASK;

  struct UdpRecvHeader hdr;
  for (word i = 0; i < sizeof hdr; i++) {
      ((byte*)&hdr)[i] = peek(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  
  if (hdr.len > *size_in_out) {
    Fatal("2BIG", *size_in_out);
  }

  for (word i = 0; i < hdr.len; i++) {
      payload[i] = peek(buf+ptr);
      ptr++;
      ptr &= RX_MASK;
  }
  *size_in_out = hdr.len;
  memcpy(from_addr_out, hdr.addr, 4);
  *from_port_out = hdr.port;

  poke_word(BASE+0x0028/*_RX_RD*/, rx_rd + recv_size);

  return OKAY;
}

#if BR_DHCP
void UseOptions(byte* o) {
    if (o[0]!=99 || o[1]!=130 || o[2]!=83 || o[3]!=99) {
        printk("bad magic: %a", o);
        Fatal("DM", o[0]);
    }
    byte* p = o+4;
    while (*p != 255) {
        byte opt = *p++;
        byte len = *p++;
        switch (opt) {
            case 1: // subnet mask
                memcpy(Vars->ip_mask, p, 4);
                printk("ip_mask %a ", p);
                break;
            case 3: // Gateway
                memcpy(Vars->ip_gateway, p, 4);
                printk("ip_gateway %a ", Vars->ip_gateway);
                break;
            case 6: // DNS Server
                memcpy(Vars->ip_resolver, p, 4);
                printk("ip_resolver %a ", p);
                break;
            default:
                printk("(opt %x len %x) ", opt, len);
        }
        p += len;
    }
}

void RunDhcp() {
    struct dhcp* out = (struct dhcp*) Vars->packout;
    out->opcode = 1; // request
    out->htype = 1; // ethernet
    out->hlen = 6; // 6-byte hw addrs
    out->hops = 0;
    memcpy(out->xid, Vars->hostname, 4);
    out->flags = 0x80; // broadcast
    memcpy(out->chaddr, Vars->mac_addr, 6);
    strcpy((char*)out->bname, BR_BOOTFILE);

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively
    byte *w = out->options;
    *w++ = 99;  // magic cookie for DHCP
    *w++ = 130;
    *w++ = 83;
    *w++ = 99;

    *w++ = 53; // 53 = DHCP Message Type Option
    *w++ = 1;  // length 1 byte
    *w++ = 1;  // 1 = Discover
    
    *w++ = 12;  // 12 = Hostname
    *w++ = 4;  // length 4 chars
    *w++ = out->xid[0];
    *w++ = out->xid[1];
    *w++ = out->xid[2];
    *w++ = out->xid[3];

    *w++ = 255;  // 255 = End
    *w++ = 0;  // length 0 bytes
    printk("Discover");

    errnum e = UdpOpen(DHCP_CLIENT_PORT);
    if (e) {
        Fatal("DO", e);
    }
    e = UdpSend((byte*)out, sizeof *out, BroadcastAddr, DHCP_SERVER_PORT);
    if (e) {
        Fatal("DS1", e);
    }

    struct dhcp* in = (struct dhcp*) Vars->packin;
    memset(Vars->packin, 0, 600);

    word size = 600;
    byte recv_from[4];
    memset(recv_from, 0, 4);
    word recv_port = 0;
    e = UdpRecv(Vars->packin, &size, recv_from, &recv_port);
    if (e) {
        Fatal("DR1", e);
    }
    printk("Offer");

    byte* yi = (byte*)&in->yiaddr;
    byte* si = (byte*)&in->siaddr;
    printk("you=%a  server=%a", yi, si);

    UseOptions(in->options);

    WizReconfigureForDhcp(in->yiaddr, Vars->ip_mask, Vars->ip_gateway);

    memset(Vars->packout, 0, 600);
    out->opcode = 1; // request
    out->htype = 1; // ethernet
    out->hlen = 6; // 6-byte hw addrs
    out->hops = 0;
    memcpy(out->xid, Vars->hostname, 4);
    out->flags = 0x80; // broadcast
    memcpy( out->ciaddr , in->yiaddr, 4);
    memcpy( out->yiaddr , in->yiaddr, 4);
    memcpy( out->siaddr , in->siaddr, 4);
    memcpy(out->chaddr, Vars->mac_addr, 6);
    strcpy((char*)out->bname, BR_BOOTFILE);

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively
    w = out->options;
    *w++ = 99;  // magic cookie for DHCP
    *w++ = 130;
    *w++ = 83;
    *w++ = 99;

    *w++ = 53; // 53 = DHCP Message Type Option
    *w++ = 1;  // length 1 byte
    *w++ = 3;  // 3 = Request
    
    *w++ = 50;  // 50 = Requested Address
    *w++ = 4;  // length 4
    *w++ = yi[0];
    *w++ = yi[1];
    *w++ = yi[2];
    *w++ = yi[3];

    *w++ = 12;  // 12 = Hostname
    *w++ = 4;  // length 4 chars
    *w++ = out->xid[0];
    *w++ = out->xid[1];
    *w++ = out->xid[2];
    *w++ = out->xid[3];

    *w++ = 255;  // 255 = End
    *w++ = 0;  // length 0 bytes
    printk("Request");

    e = UdpSend((byte*)out, sizeof *out, recv_from, DHCP_SERVER_PORT);
    if (e) {
        Fatal("DS2", e);
    }

    memset(Vars->packin, 0, 600);
    size = 600;
    memset(recv_from, 0, 4);
    recv_port = 0;
    e = UdpRecv(Vars->packin, &size, recv_from, &recv_port);
    if (e) {
        Fatal("DRs", e);
    }
    printk("Ack");

    // TODO: check for Ack option.

    yi = in->yiaddr;
    si = in->siaddr;
    byte* ma = Vars->ip_mask;
    byte* ga = Vars->ip_gateway;
    byte* dn = Vars->ip_resolver;

    printk("you=%a server=%a ", yi, si);
    printk("mask=%a gateway=%a dns=%a", ma, ga, dn);
}
#endif


/*
 * Implement the DECB LOADM format for loading bytes,
 * getting one byte pushed at a time.
 *
 * State 0 -> 1 -> 2 -> 3 -> 4 -> 5 (stay while countdown)
 * State 6 is the illegal state.
 *
 * op 0: load 1 or more bytes at given address.
 * op 255: Call given address like a function.
 *
 */

void LoadByte(byte b) {
  struct loader *p = &Vars->loader;

  switch (p->state) {
    case 0:
        p->op = b;
        break;
    case 1:
        p->counter = (word)b << 8;
        break;
    case 2:
        p->counter |= (word)b;
        break;
    case 3:
        p->addr = (word)b << 8;
        break;
    case 4:
        p->addr |= (word)b;
        break;
    case 5:
        switch (p->op) {
          case 0:
            *((byte*)p->addr++) = b;
            -- p->counter;

            if (p->counter == 0) {
              p->state = 0;
              return; // Don't let state increment this time.
            }
            break;
          default:
            p->state = 6;  // Becomes illegal state.
            break;
        }
        break;
    default:
        break;  // Illegal state -- get stuck here.
  }

  if (p->state < 5) {
    ++ p->state;
  }

  if (p->state == 5 && p->op == 255) {
    func f = (func)p->addr;
    f();
    p->state = 0;
  }
}
// Operations defined by TFTP protocol:
#define OP_READ 1
#define OP_WRITE 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

void TftpRequest(const byte* host, word port, word opcode, const char* filename) {
  char* p = (char*)Vars->packout;
  *(word*)p = opcode;
  p += 2;

  int n = strlen(filename);
  strcpy(p, filename);
  p += n+1;

  const char* mode = "octet";
  n = strlen(mode);
  strcpy(p, mode);
  p += n+1;

  errnum err = UdpSend(Vars->packout, p-(char*)Vars->packout, host, port);
  if (err) {
    Fatal("TUS", err);
  }
}

// TODO: currently this goes into a loop refetching one page.
void RunTftpGet(const byte* waiter) {
    UdpClose();
    Delay(500);
    word client_port = (0x8000 | WizTicks()) - 256;  // Avoid $FFFF.
    printk("client_port = $%x", client_port);
    errnum e = UdpOpen(client_port);
    if (e) {
           Fatal("T-UO", e);
    }
L   word opcode;

    // TODO: currently this goes into a loop refetching one page.
    while (true) {
        opcode = 0;
        printk("TFTP");
        TftpRequest(waiter, TFTPD_SERVER_PORT, OP_READ, BR_BOOTFILE);

        word size = 600;
        byte from_addr[4];
        word from_port = 0;
L       printk("recv");
        // TODO: Timeouts in UdpRecv
        errnum eee = UdpRecv(Vars->packin, &size, from_addr, &from_port);
L       printk("got %x,%d", size, eee);
        if (eee) {
            Fatal("TRE", eee);
        }
L
        word* w = (word*)Vars->packin;
        opcode = w[0];
        printk("opcode %u.", opcode);

        if (opcode == OP_ERROR) {
            printk("GOT TFTP ERROR");
            const byte* p = (byte*) Vars->packin;
            for (word i=0; i<size; i++) {
                printk("'%x", p[i]);
            }
            Fatal("TOE", opcode);
        }

        if (opcode == OP_DATA) {
            printk("GOT TFTP DATA");
        }
        Delay(60000u);
    }
}

#if WHOLE_PROGRAM
#define RomMain main  // Whole Program Optimization requires 'main' instead of 'RomMain'.
#endif
int RomMain() {
    // Clear our "global" variables to zero.
    memset(VARS_RAM, 0, sizeof (struct vars));

    // Set VDG 32x16 text screen to dots, excepting the top line.
    memset(VDG_RAM+32, '.', VDG_LEN-32);

    // Next things printed go on that second line.
    Vars->vdg_ptr = 0x0420;
    // Don't be a boor.
    PutStr("HELO");
    PutStr(__TIME__);

#if 0
   printk("(MMU");
     for (word p = 0xFFA0; p < 0xFFB0; p++) {
       printk("%x", 0x3F & *(byte*)p);
     }
   printk(")");

   // volatile byte* ramrom = (byte*) 0xFFDE;
   // *ramrom = 255;
#endif

#if CHECKSUMS
L   Checksum();  // Cannot call external if WHOLE_PROGRAM
#endif

L   WizReset();
L   memcpy(Vars->hostname, BR_NAME, 4);

#if BR_DHCP
L   wiz_configure_for_DHCP(BR_NAME, Vars->mac_addr);
L   PutStr("wc4DHCP ");
L   RunDhcp();
#endif

#if BR_STATIC
L   WizConfigure(BR_ADDR, BR_MASK, BR_GATEWAY);
#endif

    // TODO: currently this goes into a loop refetching one page.
L   RunTftpGet(BR_WAITER);

    // TODO: currently NOT REACHED
L   Fatal("OK", 250);

#if 0
    UdpClose();
    char* boot_me = Vars->packet + 4; // skip 4: opcode and blocknum.
#ifdef __GNUC__
    asm volatile ("jmp [%0]" : : "m" (boot_me) );
#else
    asm {
        jmp [boot_me]
    }
#endif
#endif
    return OKAY;
}
