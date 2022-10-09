// Boot ROM code for CocoIO 

typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned long quad;

#define WIZ_PORT 0xFF68
#define DHCP_HOSTNAME "coco"

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define VARS_RAM 0x1000
#define VDG_RAM 0x0400

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
    quad my_ipaddr;
    quad my_ipmask;
    quad my_gateway;
    byte* vdg_ptr;
    struct dhcp dhcp;
};

#define Vars ((struct vars*)VARS_RAM)

void PutChar(char ch) {
    *Vars->vdg_ptr = (byte)ch;
    Vars->vdg_ptr += 2;
}
void PutStr(const char* s) {
        while (*s) PutChar(*s);
}

word bogus_word_for_delay;
void wiz_delay(word n) {
  for (word i=0; i<n; i++) bogus_word_for_delay += i;
}

#define WIZ ((byte*)WIZ_PORT)

static byte peek(word reg) {
  WIZ[1] = (byte)(reg >> 8);
  WIZ[2] = (byte)(reg);
  byte z = WIZ[3];
  LogDebug("[%x->%2x]", reg, z);
  return z;
}
static word peek_word(word reg) {
  WIZ[1] = (byte)(reg >> 8);
  WIZ[2] = (byte)(reg);
  byte hi = WIZ[3];
  byte lo = WIZ[3];
  word z = ((word)(hi) << 8) + lo;
  LogDebug("[%x->%4x]", reg, z);
  return z;
}
static void poke(word reg, byte value) {
  WIZ[1] = (byte)(reg >> 8);
  WIZ[2] = (byte)(reg);
  WIZ[3] = value;
  LogDebug("[%x<=%2x]", reg, value);
}
static void poke_word(word reg, word value) {
  WIZ[1] = (byte)(reg >> 8);
  WIZ[2] = (byte)(reg);
  WIZ[3] = (byte)(value >> 8);
  WIZ[3] = (byte)(value);
  LogDebug("[%x<=%4x]", reg, value);
}
static void poke_n(word reg, const void* data, word size) {
  const byte* from = (const byte*) data;
  WIZ[1] = (byte)(reg >> 8);
  WIZ[2] = (byte)(reg);
  for (word i=0; i<size; i++) {
    WIZ[3] = *from++;
  }
}

void wiz_reset() {
  WIZ[0] = 128; // Reset
  wiz_delay(42);
  WIZ[0] = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  wiz_delay(42);
}

void wiz_configure(quad ip_addr, quad ip_mask, quad ip_gateway) {
  WIZ[1] = 0; WIZ[2] = 1;  // start at addr 0x0001: Gateway IP.
  poke_n(0x0001/*gateway*/, &ip_gateway, 4);
  poke_n(0x0005/*mask*/, &ip_mask, 4);
  poke_n(0x000f/*ip_addr*/, &ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
  poke_n(0x0009/*ether_mac*/, Vars->mac_addr, 6);

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
  WIZ[1] = 0; WIZ[2] = 1;  // start at addr 0x0001: Gateway IP.
  poke_n(0x0001/*gateway*/, &ip_gateway, 4);
  poke_n(0x0005/*mask*/, &ip_mask, 4);
  poke_n(0x000f/*ip_addr*/, &ip_addr, 4);
  // Keep the same mac_addr.
}

void Boot() {
    MemSet(VARS_RAM, 0, sizeof (struct vars));

    Vars->vdg_ptr = VDG_RAM;
    PutStr("Hello BootRom ");
    
    wiz_reset();
    PutStr("wiz_reset ");
    MemCpy(Vars->hostname, DHCP_HOSTNAME, 4);
    wiz_configure_for_DHCP(DHCP_HOSTNAME, Vars->mac_addr);
    PutStr("wiz_configure_for_DHCP ");

}
