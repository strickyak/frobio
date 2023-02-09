// Boot ROM code for CocoIO 
//
// to compile with CMOC and launch with `preboot.asm`.

#ifdef __GNUC__

#include <stdarg.h>

void memcpy(char* a, const char* b, int c);
void memset(char* a, int b, int c) ;// {}
void strcpy(char* a, const char*b) ;// {}
void strncpy(char* a, const char*b, int n) ;// {}
int strlen(const char* s) ;// {}

#else

#include <cmoc.h>
#include <stdarg.h>

#endif

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
typedef unsigned long quad;
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0

#define WIZ_PORT 0xFF68
#define DHCP_HOSTNAME "coco"

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define VARS_RAM 0x1000

#define VDG_RAM  0x0400
#define VDG_LEN  0x0200
#define VDG_END  0x0600

#define BASE 0x0400 // Wiz Socket 0 control base address

struct dhcp {
    byte opcode; // 1=request 2=response
    byte htype;  // 1=ethernet
    byte hlen;   // == 6 bytes
    byte hops;   // 1 or 0, on a LAN

    byte xid[4];  // transaction id
    word secs;
    word flags;  // $80 broadcast, $00 unicast.

    quad ciaddr;      // Client IP addr
    quad yiaddr;      // Your IP addr
    quad siaddr;      // Server IP addr
    quad giaddr;      // Gateway IP addr
    byte chaddr[16];  // client hardware addr
    byte sname[64];   // server name
    byte bname[128];  // boot file name

    byte options[300];
};

struct vars {
    byte mac_addr[6];
    byte hostname[4];
    quad ip_addr;
    quad ip_mask;
    quad ip_gateway;
    quad ip_dns_server;
    word vdg_ptr;
    word bogus_side_effect;
    byte packet[600];
};

#define Vars ((struct vars*)VARS_RAM)

/////////////////////////////////////////////////
//   Copied & modified from frob2/wiz/wiz5100s.h
// Keep this at the default of 2K for each.
#define TX_SIZE 2048
#define RX_SIZE 2048
#define TX_MASK (TX_SIZE - 1)
#define RX_MASK (RX_SIZE - 1)
#define TX_BUF_0  0x4000
#define RX_BUF_0  0x6000

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
    quad addr;
    word port;
    word len;
};
/////////////////////////////////////////////////

void Delay(word n) {
  volatile word* p = &Vars->bogus_side_effect;
  for (word i=0; i<n; i++) *p += i;
}

void PutChar(char ch) {
    word p = Vars->vdg_ptr;
    if (96 <= ch && ch <= 126) ch -= 32;
    if (ch < 32 || ch >= 127) ch = '?';
    byte codepoint = (byte)ch;
    *(byte*)p = 0x40 | (0x3f & codepoint);
    p++;
    if (p>=VDG_END) {
        for (word i = VDG_RAM; i< VDG_END; i++) {
            if (i < VDG_END-32) {
                // Copy the line below.
                *(volatile byte*)i = *(volatile byte*)(i+32);
            } else {
                // Clear the last line.
                *(volatile byte*)i = 0x40 | 32;
            }
        }
        p = VDG_END-32;
    }
    Vars->vdg_ptr = p;
    Delay(100);  // don't print too fast and then HCF!
}

void PutStr(const char* s) {
    while (*s) PutChar(*s++);
}

const byte HexAlphabet[] = "0123456789abcdef";

void PutHex(quad x) {
  if (x > 15u) {
    PutHex(x >> 4u);
  }
  PutChar( HexAlphabet[ 15u & (word)x ] );
}
void PutDec(word x) {
  if (x > 9u) {
    PutDec(x / 10u);
  }
  PutChar('0' + (byte)(x % 10u));
}

void printk(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const char* s = format;
    while (*s) {
        bool longer = false;
        if (*s == '%') {
NEXT:       ++s;
            switch (*s) {
                case 'l': {
                    longer = true;
                    goto NEXT;
                };
                break;
                case 'x': {
                    quad x;
                    if (longer) {
                        x = va_arg(ap, quad);
                    } else {
                        x = va_arg(ap, word);
                    }
                    PutHex(x);
                }
                break;
                case 'u': {
                    word x = va_arg(ap, word);
                    PutDec(x);
                }
                break;
                case 's': {
                    const char* x = va_arg(ap, const char*);
                    PutStr(x);
                }
                break;
                default:
                    PutChar(*s);
            }
        } else {
            PutChar((*s < ' ' || *s > 126) ? '?' : *s);
        }
        s++;
    }
    va_end(ap);
    PutChar(';');
    Delay(1000);
}

// Debug Trace by Line Number
void ShowLine(word line) {
    printk("%u", line);
}
#define L ShowLine(__LINE__);

void Fatal() {
    PutStr(" *FATAL* ");
    while (1) continue;
}

static byte peek(word reg) {
  byte* wiz = (byte*)WIZ_PORT;
  wiz[1] = (byte)(reg >> 8);
  wiz[2] = (byte)(reg);
  byte z = wiz[3];
  //printk("[%x->%x]", reg, z);
  return z;
}
static word peek_word(word reg) {
  byte* wiz = (byte*)WIZ_PORT;
  wiz[1] = (byte)(reg >> 8);
  wiz[2] = (byte)(reg);
  byte hi = wiz[3];
  byte lo = wiz[3];
  word z = ((word)(hi) << 8) + (word)lo;
  //printk("[%x=>%x]", reg, z);
  return z;
}
static void poke(word reg, byte value) {
  byte* wiz = (byte*)WIZ_PORT;
  wiz[1] = (byte)(reg >> 8);
  wiz[2] = (byte)(reg);
  wiz[3] = value;
  //printk("[%x<-%x]", reg, value);
}
static void poke_word(word reg, word value) {
  byte* wiz = (byte*)WIZ_PORT;
  wiz[1] = (byte)(reg >> 8);
  wiz[2] = (byte)(reg);
  wiz[3] = (byte)(value >> 8);
  wiz[3] = (byte)(value);
  //printk("[%x<=%x]", reg, value);
}
static void poke_n(word reg, const void* data, word size) {
  byte* wiz = (byte*)WIZ_PORT;
  //printk("[%x<<%x]", reg, size);
  const byte* from = (const byte*) data;
  wiz[1] = (byte)(reg >> 8);
  wiz[2] = (byte)(reg);
  for (word i=0; i<size; i++) {
    wiz[3] = *from++;
  }
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    word t = peek_word(0x0082/*TCNTR Tick Counter*/);
    return t;
}

// Is current time before the limit?  Using Wiz Ticks.
byte before(word limit) {
    word t = peek_word(0x0082/*TCNTR Tick Counter*/);
    // After t crosses limit, (limit-t) "goes negative" so high bit is set.
    return 0 == ((limit-t) & 0x8000);
}
byte wait(word reg, byte value, word millisecs_max) {
L   word start = peek_word(0x0082/*TCNTR Tick Counter*/);
L   word limit = 10*millisecs_max + start;
    while (true || before(limit)) {  // TODO ddt
L       if (peek(reg) == value) {
L           return OKAY;
        }
    }
L   return 5; // TIMEOUT
}

void WizReset() {
  byte* wiz = (byte*)WIZ_PORT;
  wiz[0] = 128; // Reset
L Delay(99);
  wiz[0] = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
L Delay(99);
}

void WizConfigure(quad ip_addr, quad ip_mask, quad ip_gateway) {
L poke_n(0x0001/*gateway*/, &ip_gateway, 4);
L poke_n(0x0005/*mask*/, &ip_mask, 4);
L poke_n(0x000f/*ip_addr*/, &ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
L poke_n(0x0009/*ether_mac*/, Vars->mac_addr, 6);

L poke(0x001a/*=Rx Memory Size*/, 0x55); // 2k per sock
L poke(0x001b/*=Tx Memory Size*/, 0x55); // 2k per sock

  // Force all 4 sockets to be closed.
  for (word base=0x0400; base<0x0800; base+=0x0100) {
L     poke(base+SockCommand, 0x10/*CLOSE*/);
L     wait(base+SockCommand, 0, 500);
L     poke(base+SockMode, 0x00/*Protocol: Socket Closed*/);
L     poke(base+0x001e/*_RXBUF_SIZE*/, 2); // 2KB
L     poke(base+0x001f/*_TXBUF_SIZE*/, 2); // 2KB
  }
}

void wiz_configure_for_DHCP(const char* name4, byte* hw6_out) {
  WizConfigure(0L, 0xFFFFFF00, 0L);

  // Create locally assigned mac_addr from name4.
  byte mac_addr[6] = {2, 32, 0, 0, 0, 0};  // local prefix.
  strncpy((char*)mac_addr+2, name4, 4);
  poke_n(0x0009/*ether_mac*/, mac_addr, 6);
  memcpy(hw6_out, mac_addr, 6);
}

void WizReconfigureForDhcp(quad ip_addr, quad ip_mask, quad ip_gateway) {
  printk("reconfigure");
  poke_n(0x0001/*gateway*/, &ip_gateway, 4);
  poke_n(0x0005/*mask*/, &ip_mask, 4);
  poke_n(0x000f/*ip_addr*/, &ip_addr, 4);
  // Keep the same mac_addr.
}

void UdpClose() {
  printk("CLOSE");

  poke(BASE+0x0001/*_IR*/, 0x1F); // Clear all interrupts.
  poke(BASE+SockCommand, 0x10/*CLOSE*/);
  wait(BASE+SockCommand, 0, 500);
  poke(BASE+SockMode, 0x00/*Protocol: Socket Closed*/);
}

errnum UdpOpen(word src_port) {
  printk("UdpOpen %u....", src_port);
  poke(BASE+SockMode, 2); // Set UDP Protocol mode.
  poke_word(BASE+SockSourcePort, src_port);
  poke(BASE+0x0001/*_IR*/, 0x1F); // Clear all interrupts.
  poke(BASE+0x002c/*_IMR*/, 0xFF); // mask all interrupts.
  poke_word(BASE+0x002d/*_FRAGR*/, 0); // don't fragment.

  poke(BASE+0x002f/*_MR2*/, 0x00); // no blocks.
  poke(BASE+SockCommand, 1/*=OPEN*/);  // OPEN IT!
  errnum err = wait(BASE+SockCommand, 0, 500);
  if (err) {L; Fatal();}

  err = wait(BASE+SockStatus, 0x22/*SOCK_UDP*/, 500);
  if (err) {L; Fatal();}

  word tx_r = peek_word(BASE+TxReadPtr);
  poke_word(BASE+TxWritePtr, tx_r);
  word rx_w = peek_word(BASE+0x002A/*_RX_WR*/);
  poke_word(BASE+0x0028/*_RX_RD*/, rx_w);
  printk("UdpOpen OKAY", src_port);
  return OKAY;
}

errnum UdpSend(byte* payload, word size, quad dest_ip, word dest_port) {
  printk("SEND: payload=%x size=%x ", payload, size);
  byte* d = (byte*)&dest_ip;
  printk(" dest=%u.%u.%u.%u:%u. ", d[0], d[1], d[2], d[3], dest_port);

  word buf = TX_BUF_0;

  byte status = peek(BASE+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) return 0xf6 /*E_NOTRDY*/;

  printk("dest_ip ");
  poke_n(BASE+SockDestIp, &dest_ip, sizeof dest_ip);
  printk("dest_p ");
  poke_word(BASE+SockDestPort, dest_port);

  bool broadcast = false;
  byte send_command = 0x20;
  if (dest_ip == 0xFFFFFFFFL) {
    // Broadcast to 255.255.255.255
    broadcast = true;
    send_command = 0x21;
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

  printk("status->%x ", peek(BASE+SockStatus));
  //sock_show(socknum);
  printk("cmd:SEND ");

  poke(BASE+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(BASE+SockCommand, send_command);  // SEND IT!
  wait(BASE+SockCommand, 0, 500);
  printk("status->%x ", peek(BASE+SockStatus));

  while(1) {
    byte irq = peek(BASE+SockInterrupt);
    if (irq&0x10) break;
  }
  poke(BASE+SockInterrupt, 0x10);  // Reset RECV interrupt.
  return OKAY;
}

errnum UdpRecv(byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out) {
  word buf = RX_BUF_0;

  byte status = peek(BASE+SockStatus);
  if (status != 0x22/*SOCK_UDP*/) return 0xf6 /*E_NOTRDY*/;

  poke_word(BASE+0x000c, 0); // clear Dest IP Addr
  poke_word(BASE+0x000e, 0); // ...
  poke_word(BASE+0x0010, 0); // clear Dest port addr

  poke(BASE+SockInterrupt, 0x1f);  // Reset interrupts.
  poke(BASE+SockCommand, 0x40/*=RECV*/);  // RECV command.
  wait(BASE+SockCommand, 0, 500);
  printk("status->%x ", peek(BASE+SockStatus));

  printk(" ====== WAIT ====== ");
  while(1) {
    byte irq = peek(BASE+SockInterrupt);
    if (irq) {
      poke(BASE+SockInterrupt, 0x1f);  // Reset interrupts.
      if (irq != 0x04 /*=RECEIVED*/) {
        return 0xf4/*=E_READ*/;
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
    printk(" *** Packet Too Big [rs=%x. sio=%x.]\n", hdr.len, *size_in_out);
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

  poke_word(BASE+0x0028/*_RX_RD*/, rx_rd + recv_size);

  return OKAY;
}

void UseOptions(byte* o) {
    if (o[0]!=99 || o[1]!=130 || o[2]!=83 || o[3]!=99) {
        printk("UseOptions: bad magic: %lx", *(quad*)o);
        Fatal();
    }
    byte* p = o+4;
    while (*p != 255) {
        byte opt = *p++;
        byte len = *p++;
        switch (opt) {
            case 1: // subnet mask
                Vars->ip_mask = *(quad*)p;
                printk("ip_mask %lx ", Vars->ip_mask);
                break;
            case 3: // Gateway
                Vars->ip_gateway = *(quad*)p;
                printk("ip_gateway %lx ", Vars->ip_gateway);
                break;
            case 6: // DNS Server
                Vars->ip_dns_server = *(quad*)p;
                printk("ip_dns_server %lx ", Vars->ip_dns_server);
                break;
            default:
                printk("(opt %x len %x) ", opt, len);
        }
        p += len;
    }
}

void RunDhcp() {
    struct dhcp* p = (struct dhcp*) Vars->packet;
    p->opcode = 1; // request
    p->htype = 1; // ethernet
    p->hlen = 6; // 6-byte hw addrs
    p->hops = 0;
    memcpy(p->xid, Vars->hostname, 4);
    p->flags = 0x80; // broadcast
    memcpy(p->chaddr, Vars->mac_addr, 6);
    strcpy((char*)p->bname, "cocoio.boot");

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively
    byte *w = p->options;
    *w++ = 99;  // magic cookie for DHCP
    *w++ = 130;
    *w++ = 83;
    *w++ = 99;

    *w++ = 53; // 53 = DHCP Message Type Option
    *w++ = 1;  // length 1 byte
    *w++ = 1;  // 1 = Discover
    
    *w++ = 12;  // 12 = Hostname
    *w++ = 4;  // length 4 chars
    *w++ = p->xid[0];
    *w++ = p->xid[1];
    *w++ = p->xid[2];
    *w++ = p->xid[3];

    *w++ = 255;  // 255 = End
    *w++ = 0;  // length 0 bytes
    printk("Discover");

    errnum e = UdpOpen(DHCP_CLIENT_PORT);
    if (e) {
        printk("cannot UdpOpen: e=%x.", e);
        Fatal();
    }
    e = UdpSend((byte*)p, sizeof *p, 0xFFFFFFFFL, 67);
    if (e) {
        printk("cannot UdpSend: e=%x.", e);
        Fatal();
    }
    memset(Vars->packet, 0, 600);
    word size = 600;
    quad recv_from = 0;
    word recv_port = 0;
    e = UdpRecv(Vars->packet, &size, &recv_from, &recv_port);
    if (e) {
        printk("cannot UdpRecv: e=%x.", e);
        Fatal();
    }
    printk("Offer");
    quad yiaddr = p->yiaddr;
    quad siaddr = p->siaddr;

    byte* yi = (byte*)&p->yiaddr;
    byte* si = (byte*)&p->siaddr;
    printk("you=%u.%u.%u.%u  server=%u.%u.%u.%u",
        yi[0], yi[1], yi[2], yi[3],
        si[0], si[1], si[2], si[3]
        );

    UseOptions(p->options);

    WizReconfigureForDhcp(yiaddr, Vars->ip_mask, Vars->ip_gateway);

    memset(Vars->packet, 0, 600);
    p->opcode = 1; // request
    p->htype = 1; // ethernet
    p->hlen = 6; // 6-byte hw addrs
    p->hops = 0;
    memcpy(p->xid, Vars->hostname, 4);
    p->flags = 0x80; // broadcast
    p->ciaddr = yiaddr;
    p->yiaddr = yiaddr;
    p->siaddr = siaddr;
    memcpy(p->chaddr, Vars->mac_addr, 6);
    strcpy((char*)p->bname, "cocoio.boot");

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively
    w = p->options;
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
    *w++ = p->xid[0];
    *w++ = p->xid[1];
    *w++ = p->xid[2];
    *w++ = p->xid[3];

    *w++ = 255;  // 255 = End
    *w++ = 0;  // length 0 bytes
    printk("Request");

    e = UdpSend((byte*)p, sizeof *p, 0xFFFFFFFFL, 67);
    if (e) {
        printk("cannot UdpSend: e=%x.\n", e);
        Fatal();
    }

    memset(Vars->packet, 0, 600);
    size = 600;
    recv_from = 0;
    recv_port = 0;
    e = UdpRecv(Vars->packet, &size, &recv_from, &recv_port);
    if (e) {
        printk("cannot UdpRecv: e=%x.\n", e);
        Fatal();
    }
    printk("Ack");

    // TODO: check for Ack option.

    yiaddr = p->yiaddr;
    siaddr = p->siaddr;

    yi = (byte*)&p->yiaddr;
    si = (byte*)&p->siaddr;
    byte* ma = (byte*)&Vars->ip_mask;
    byte* ga = (byte*)&Vars->ip_gateway;
    byte* dn = (byte*)&Vars->ip_dns_server;

    printk("you=%u.%u.%u.%u server=%u.%u.%u.%u ",
        yi[0], yi[1], yi[2], yi[3],
        si[0], si[1], si[2], si[3]);

    printk("mask=%u.%u.%u.%u gateway=%u.%u.%u.%u dns=%u.%u.%u.%u",
        ma[0], ma[1], ma[2], ma[3],
        ga[0], ga[1], ga[2], ga[3],
        dn[0], dn[1], dn[2], dn[3]);
}

#define DEFAULT_TFTPD_PORT 69 /* TFTPD */

// Operations defined by TFTP protocol:
#define OP_READ 1
#define OP_WRITE 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

void TftpRequest(quad host, word port, word opcode, const char* filename) {
  char* p = (char*)Vars->packet;
  *(word*)p = opcode;
  p += 2;

  int n = strlen(filename);
  strcpy(p, filename);
  p += n+1;

  const char* mode = "octet";
  n = strlen(mode);
  strcpy(p, mode);
  p += n+1;

  errnum err = UdpSend(Vars->packet, p-(char*)Vars->packet, host, port);
  if (err) {
    printk("cannot UdpSend request: errnum %x", err);
    Fatal();
  }
}

void RunTftpGet() {
L   word opcode;
    while (true) {
        opcode = 0;
        printk("TFTP");
        TftpRequest(0xFFFFFFFFUL, DEFAULT_TFTPD_PORT, OP_READ, "cocoio.boot");

        word size = 600;
        quad from_addr = 0;
        word from_port = 0;
L       printk("recv");
        // TODO: Timeouts in UdpRecv
        errnum e = UdpRecv(Vars->packet, &size, &from_addr, &from_port);
L       printk("got size $%x", size);
        if (e) {
L           printk("error %u.", e);
            opcode = 255;
        } else {
L           word* w = (word*)Vars->packet;
            opcode = w[0];
            printk("opcode %u.", opcode);
        }

        if (opcode == OP_ERROR) {
            printk("GOT TFTP ERROR");
            const byte* p = (byte*) Vars->packet;
            for (word i=0; i<size; i++) {
                printk("*%x", p[i]);
            }
        }

        if (opcode == OP_DATA) {
            printk("GOT TFTP DATA");
            break;
        }
        Delay(60000u);
        Delay(60000u);
    }
}

// TODO: Load more than one block, at somewhere like $2600.
int main() {
    memset(VARS_RAM, 0, sizeof (struct vars));
    memset(VDG_RAM+32, '.', VDG_LEN-32);
    Vars->vdg_ptr = 0x0420;
    PutStr("Hello BootRom ");

L   WizReset();
L   PutStr("WizReset ");
L   memcpy(Vars->hostname, DHCP_HOSTNAME, 4);
L   wiz_configure_for_DHCP(DHCP_HOSTNAME, Vars->mac_addr);
L   PutStr("wiz_configure_for_DHCP ");

L   RunDhcp();

L   RunTftpGet();
    UdpClose();
    char* boot_me = Vars->packet + 4; // skip 4: opcode and blocknum.
#ifdef __GNUC__
    asm volatile ("jmp [%0]" : : "m" (boot_me) );
#else
    asm {
        jmp [boot_me]
    }
#endif
    return OKAY;
}
