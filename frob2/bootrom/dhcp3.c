#include "frob2/bootrom/bootrom3.h"

#define D {PrintF("D%u;", __LINE__); Delay(3000);}

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define BAD_SHORT_PACKET       50
#define BAD_DHCP_OPTION_MAGIC  51
#define BAD_OFFER_LEN          52
#define BAD_BOOTP_REPLY        53

struct dhcp {
    byte opcode; // 1=request 2=response
    byte htype;  // 1=ethernet
    byte hlen;   // == 6 bytes
    byte hops;   // 1 or 0, on a LAN
//4
    byte xid[4];  // transaction id
//8
    word secs;
    word flags;  // $80 broadcast, $00 unicast.
//12
    byte ciaddr[4];      // Client IP addr
    byte yiaddr[4];      // Your IP addr
//20
    byte siaddr[4];      // Server IP addr
    byte giaddr[4];      // Gateway IP addr
//28
    byte chaddr[16];  // client hardware addr
//44
    byte sname[64];   // server name
//108
    byte bname[128];  // boot file name
//236
    // THEN byte options[...]
};

const byte RequestDHCP0[] = {
  1, // opcode: 1=request 2-response
  1, // htype: 1=ethernet
  6, // hlen: == 6 byte MAC.
  0, // hops: 1 or 0, on a LAN.
};
// Then 4 byte xid == name
const byte RequestDHCP8[] = {
  0, 0, // secs
  0x80, 0x00, // broadcast
};
const byte DiscoverOptions[] = {
  99, 130, 83, 99, // magic cookie for DHCP
  53, 1, 1,  // 53=OptionType 1=len 1=Discover
  12, 4, // 12=Hostname, 4=len4chars, FOLLOWED BY NAME
};
const byte RequestOptions[] = {
  99, 130, 83, 99, // magic cookie for DHCP
  53, 1, 3,  // 53=OptionType 1=len 3=Request
  12, 4, // 12=Hostname, 4=len4chars, FOLLOWED BY NAME
};
const byte RequestAddressOption[] = {
  50, 4, // plus 4-byte address.
};
const byte EndOptions[] = {
  255, 0   // 255=end, 0=len
};

const char lmnop[8] = "LMNOPQR";

void SendDhcpRequest(PARAM_SOCK_AND const char* name4, bool second) {
PrintF("Req#%d;", second);
D
  byte mac[6] = {
    2, 32, name4[0], name4[1], name4[2], name4[3]};

  word req_size = sizeof (struct dhcp) + sizeof EndOptions;
  if (second) {
    req_size += sizeof RequestOptions + sizeof RequestAddressOption + 8;
  } else {
    req_size += sizeof RequestOptions + 4;
  }

D
  WizReserveToSend(SOCK_AND req_size);
PrintF("reqs=$%x=%u.;", req_size, req_size);
D

  WizBytesToSend(SOCK_AND RequestDHCP0, sizeof RequestDHCP0);

  WizBytesToSend(SOCK_AND Vars->xid, 4);

  WizBytesToSend(SOCK_AND RequestDHCP8, sizeof RequestDHCP8);

  WizDataToSend(SOCK_AND Eight00s, 8);  // ciaddr & yiaddr

  WizDataToSend(SOCK_AND Eight00s, 8); // siaddr & giaddr

  WizBytesToSend(SOCK_AND mac, 6); // ch_addr

  WizDataToSend(SOCK_AND Eight00s, 2);  // ch_addr cont'd


  byte n = 1/*chaddr cont'd*/ + 8/*sname*/; //  + 16/*bname*/;
  for (byte i = 0; i < n; i++) {

    WizDataToSend(SOCK_AND Eight00s, 8); // ch_addr + sname + bname
  }

  for (byte i = 0; i < 16/*bname*/; i++) {

    WizDataToSend(SOCK_AND lmnop, 8); // ch_addr + sname + bname
  }

D
  if (second) {
D
    WizBytesToSend(SOCK_AND RequestOptions, sizeof RequestOptions);
    WizDataToSend(SOCK_AND name4, 4);
    WizBytesToSend(SOCK_AND RequestAddressOption, sizeof RequestAddressOption);
    WizBytesToSend(SOCK_AND Vars->ip_addr, 4);
  } else {
D
    WizBytesToSend(SOCK_AND DiscoverOptions, sizeof DiscoverOptions);
    WizDataToSend(SOCK_AND name4, 4);
  }
D
  WizBytesToSend(SOCK_AND EndOptions, sizeof EndOptions);

PrintF("reqs=$%x=%u.;", req_size, req_size);
  WizFinalizeSend(SOCK_AND &BroadcastUdpProto, req_size);
D
}

errnum WasteRecvBytes(PARAM_SOCK_AND word n) {
D
  errnum e = OKAY;
  char junk[8];
  while (n >= 8) {
    e = WizRecvChunk(SOCK_AND  junk, 8);
    if (e) return e;
    n -= 8;
  }
  if (n > 0) {
    e = WizRecvChunk(SOCK_AND  junk, n);
    if (e) return e;
  }
D
  return e;
}

errnum RecvDhcpReply(PARAM_SOCK_AND struct UdpRecvHeader *hdr, bool second) {
D
  errnum e = OKAY;
  char buf[4];

  // we want 16:yiaddr (you) and 20:siaddr (server)
  if (hdr->len < 5 + sizeof (struct dhcp)) return BAD_SHORT_PACKET;

D
  e = WizRecvChunk(SOCK_AND  buf, 4);
  if (e) return e;
  if (buf[0] != 2/*=REPLY*/) return BAD_BOOTP_REPLY;
  
  WasteRecvBytes(SOCK_AND 12);  // done 16

  e = WizRecvChunk(SOCK_AND  Vars->ip_addr, 4);  // done 20
  if (e) return e;
  PrintF("addr=%a;", Vars->ip_addr);

  e = WizRecvChunk(SOCK_AND  Vars->ip_dhcp, 4); // done 24
  if (e) return e;
  PrintF("dhcpd=%a;", Vars->ip_dhcp);

  WasteRecvBytes(SOCK_AND sizeof (struct dhcp) - 24);  // done struct dhcp


D
  e = WizRecvChunk(SOCK_AND  buf, 4); // DHCP magic // done 4 more
  if (e) return e;
  if (buf[0] != 99) return BAD_DHCP_OPTION_MAGIC;
  if (buf[1] != 130) return BAD_DHCP_OPTION_MAGIC;
  if (buf[2] != 83) return BAD_DHCP_OPTION_MAGIC;
  if (buf[3] != 99) return BAD_DHCP_OPTION_MAGIC;

  word togo = hdr->len - 4 - sizeof (struct dhcp);
  byte dhcp_op = 0, opt, len;
  while (togo >= 1) {
D
    e = WizRecvChunk(SOCK_AND  &opt, 1); // Get Option
    if (e) return e;
    --togo;

    if (opt == 255) break;  // From Google Wifi, 255 is last, no len.

    e = WizRecvChunk(SOCK_AND  &len, 1); // Get Len
    if (e) return e;
    --togo;

    PrintF("opt=%x len=%x;", opt, len);
    togo -= len;
    if (len == 4) {
      e = WizRecvChunk(SOCK_AND  buf, 4); // 4 byte IP addr
      if (e) return e;

      if (opt == 1) {
        memcpy(Vars->ip_mask, buf, 4);
        PrintF("mask=%a;", Vars->ip_mask);
      } else if (opt == 3) {
        memcpy(Vars->ip_gateway, buf, 4);
        PrintF("gw=%a;", Vars->ip_gateway);
      } else if (opt == 6) {
        memcpy(Vars->ip_resolver, buf, 4);
        PrintF("resolv=%a;", Vars->ip_resolver);
      } else {
        // just dont use buf.
      }
    } else if (opt==53 && len==1) {

// Value 	Message Type 	Reference 
// 1	DHCPDISCOVER	[RFC2132]
// 2	DHCPOFFER	[RFC2132]
// 3	DHCPREQUEST	[RFC2132]
// 4	DHCPDECLINE	[RFC2132]
// 5	DHCPACK	[RFC2132]
// 6	DHCPNAK	[RFC2132]
// 7	DHCPRELEASE	[RFC2132]
// 8	DHCPINFORM	[RFC2132]
// 9	DHCPFORCERENEW	[RFC3203]
// 10	DHCPLEASEQUERY	[RFC4388]
// 11	DHCPLEASEUNASSIGNED	[RFC4388]
// 12	DHCPLEASEUNKNOWN	[RFC4388]
// 13	DHCPLEASEACTIVE	[RFC4388]
// 14	DHCPBULKLEASEQUERY	[RFC6926]
// 15	DHCPLEASEQUERYDONE	[RFC6926]
// 16	DHCPACTIVELEASEQUERY	[RFC7724]
// 17	DHCPLEASEQUERYSTATUS	[RFC7724]
// 18	DHCPTLS	[RFC7724]

      e = WizRecvChunk(SOCK_AND &dhcp_op, 1); // 1 byte opcode
      if (e) return e;
      PrintF("dhcp_op=%x;", dhcp_op);
      if (dhcp_op != (second ? 5 : 2)) {
        break;
      }
    } else {
      WasteRecvBytes(SOCK_AND  len);
    }
  }

    PrintF("final waste=%x;", togo);
  if (togo) {
    e = WasteRecvBytes(SOCK_AND togo);
    if (e) return e;
  }
D
  if (dhcp_op != (second ? 5 : 2)) {
D
    return NOTYET;
  }
D
  return OKAY;
}

errnum RunOneDhcpRound(PARAM_SOCK_AND const char* name4, bool second) {
PrintF("Round#%x;", second);
D
  errnum e = OKAY;
  struct UdpRecvHeader hdr;

  // do {
  D
    SendDhcpRequest(SOCK_AND name4, second);

  D
    e = WizRecvChunk(SOCK_AND (char*)&hdr, sizeof hdr);
  PrintF("rc->%x", e);
  D
    if (e) return e;
  D

    if (hdr.len < sizeof (struct dhcp)) {
  D
       return BAD_OFFER_LEN;
    }
  D
    e = RecvDhcpReply(SOCK_AND &hdr, second);
  PrintF("reply->%x", e);
  // } while (e==NOTYET);
  D
  return e;
}

void WizConfigureDhcpFirst(const byte* name4) {
D
  WizPutN(0x0001/*gateway*/, Eight00s, 4);
  WizPutN(0x0005/*mask*/, ClassAMask, 4);
  WizPutN(0x000f/*ip_addr*/, Eight00s, 4);

  // Create locally assigned mac_addr from ip_addr.
  Vars->mac_addr [0] = 2;
  Vars->mac_addr [1] = 32;
  memcpy(Vars->mac_addr+2, name4, 4);
  WizPutN(0x0009/*ether_mac*/, Vars->mac_addr, 6);
  PrintF("Conf a=%a m=%a g=%a MAC=2.32.%a;", Vars->ip_addr, Vars->ip_mask, Vars->ip_gateway, Vars->mac_addr+2);
}

errnum RunDhcp(PARAM_SOCK_AND const char* name4, word ticks) {
D
    WizOpen(SOCK_AND &BroadcastUdpProto, DHCP_CLIENT_PORT);
D
  WizConfigureDhcpFirst(name4);

  UdpDial(SOCK_AND  &BroadcastUdpProto,
          /*dest_ip=*/ SixFFs, DHCP_SERVER_PORT);

  // First round: Discover.
D
  Vars->xid[0] = 42;
  Vars->xid[1] = 42;
  Vars->xid[2] = (byte)(ticks>>8);
  Vars->xid[3] = (byte)(ticks);
  ++ticks;
  errnum e = RunOneDhcpRound(SOCK_AND name4, false);
  if (e) { Fatal("R1DR/F", e); Delay(50000); return e; }
  // Second round: Request.
D
  Vars->xid[2] = (byte)(ticks>>8);
  Vars->xid[3] = (byte)(ticks);
  ++ticks;
  e = RunOneDhcpRound(SOCK_AND name4, true);
  if (e) { Fatal("R1DR/F", e); Delay(50000); return e; }
D
  WizConfigure(Vars->ip_addr, Vars->ip_mask, Vars->ip_gateway);
D
  WizClose(JUST_SOCK);
D
  return OKAY;
}
