#include "frob3/axiom/bootrom3.h"

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
  12, 8, // 12=Hostname, 8=len4chars, FOLLOWED BY NAME
};
const byte RequestOptions[] = {
  99, 130, 83, 99, // magic cookie for DHCP
  53, 1, 3,  // 53=OptionType 1=len 3=Request
  12, 8, // 12=Hostname, 8=len4chars, FOLLOWED BY NAME
};
const byte RequestAddressOption[] = {
  50, 4, // plus 4-byte address.
};
const byte EndOptions[] = {
  255, 0   // 255=end, 0=len
};

void SendDhcpRequest(const struct sock* sockp, const char* name4, bool second) {
  PrintF("%s. ", second ? "REQUEST" : "DISCOVER");

  word req_size = sizeof (struct dhcp) + sizeof EndOptions;
  if (second) {
    req_size += sizeof RequestOptions + sizeof RequestAddressOption + 12;
  } else {
    req_size += sizeof RequestOptions + 8;
  }

  WizReserveToSend(SOCK_AND req_size);

  WizBytesToSend(SOCK_AND RequestDHCP0, sizeof RequestDHCP0);

  WizBytesToSend(SOCK_AND Vars->xid, 4);

  WizBytesToSend(SOCK_AND RequestDHCP8, sizeof RequestDHCP8);

  WizDataToSend(SOCK_AND Eight00s, 4); // ciaddr
  WizBytesToSend(SOCK_AND Vars->ip_addr, 4); // yiaddr
  WizDataToSend(SOCK_AND Eight00s, 4); // siaddr
  WizDataToSend(SOCK_AND Eight00s, 4); // giaddr

  WizBytesToSend(SOCK_AND Vars->mac_addr, 6); // ch_addr

  WizDataToSend(SOCK_AND Eight00s, 2);  // ch_addr cont'd


  byte n = 1/*chaddr cont'd*/ + 8/*sname*/;
  for (byte i = 0; i < n; i++) {
    WizDataToSend(SOCK_AND Eight00s, 8); // ch_addr + sname
  }

  for (byte i = 0; i < 16/*bname*/; i++) {
    // If someone is sniffing, this will be obvious:
    WizDataToSend(SOCK_AND "COCOIO_", 8); // bname
  }

  if (second) {
    WizBytesToSend(SOCK_AND RequestOptions, sizeof RequestOptions);
    WizBytesToSend(SOCK_AND Vars->hostname, 8);
    WizBytesToSend(SOCK_AND RequestAddressOption, sizeof RequestAddressOption);
    WizBytesToSend(SOCK_AND Vars->ip_addr, 4);
  } else {
    WizBytesToSend(SOCK_AND DiscoverOptions, sizeof DiscoverOptions);
    WizBytesToSend(SOCK_AND Vars->hostname, 8);
  }
  WizBytesToSend(SOCK_AND EndOptions, sizeof EndOptions);

  WizFinalizeSend(SOCK_AND &BroadcastUdpProto, req_size);
}

errnum WasteRecvBytes(const struct sock* sockp, word n) {
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
  return e;
}

errnum RecvDhcpReply(const struct sock* sockp, struct UdpRecvHeader *hdr, bool second) {
  errnum e = OKAY;
  char buf[4];

  // we want 16:yiaddr (you) and 20:siaddr (server)
  if (hdr->len < 5 + sizeof (struct dhcp)) return BAD_SHORT_PACKET;

  e = WizRecvChunk(SOCK_AND  buf, 4);
  if (e) return e;
  if (buf[0] != 2/*=REPLY*/) return BAD_BOOTP_REPLY;
  PrintF("(R) ");
  
  WasteRecvBytes(SOCK_AND 12);  // done 16

  e = WizRecvChunkBytes(SOCK_AND  Vars->ip_addr, 4);  // done 20
  if (e) return e;
  PrintF("addr=%a;", Vars->ip_addr);

  e = WizRecvChunkBytes(SOCK_AND  Vars->ip_dhcp, 4); // done 24
  if (e) return e;
  PrintF("dhcpd=%a;", Vars->ip_dhcp);

  WasteRecvBytes(SOCK_AND sizeof (struct dhcp) - 24);  // done struct dhcp


  e = WizRecvChunk(SOCK_AND  buf, 4); // DHCP magic // done 4 more
  if (e) return e;
  if (buf[0] != 99) return BAD_DHCP_OPTION_MAGIC;
  if (buf[1] != 130) return BAD_DHCP_OPTION_MAGIC;
  if (buf[2] != 83) return BAD_DHCP_OPTION_MAGIC;
  if (buf[3] != 99) return BAD_DHCP_OPTION_MAGIC;

  word togo = hdr->len - 4 - sizeof (struct dhcp);
  byte dhcp_op = 0, opt, len;
  while (togo >= 1) {
    e = WizRecvChunkBytes(SOCK_AND  &opt, 1); // Get Option
    if (e) return e;
    --togo;

    if (opt == 255) break;  // From Google Wifi, 255 is last, no len.

    e = WizRecvChunkBytes(SOCK_AND  &len, 1); // Get Len
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

      e = WizRecvChunkBytes(SOCK_AND &dhcp_op, 1); // 1 byte opcode
      if (e) return e;
      PrintF(" %s=$%x; ",
          (dhcp_op==2 ? "OFFER" : dhcp_op==5 ? "ACK" : "?"),
          dhcp_op);
      if (dhcp_op != (second ? 5 : 2)) {
        break;
      }
    } else {
      WasteRecvBytes(SOCK_AND  len);
    }
  }

    PrintF("extra=%x;", togo);
  if (togo) {
    e = WasteRecvBytes(SOCK_AND togo);
    if (e) return e;
  }
  if (dhcp_op != (second ? 5 : 2)) {
    return NOTYET;
  }
  return OKAY;
}

errnum RunOneDhcpRound(const struct sock* sockp, const char* name4, bool second) {
  PrintF(" Round#%x; ", second);
  errnum e = OKAY;
  struct UdpRecvHeader hdr;

  SendDhcpRequest(SOCK_AND name4, second);

  e = WizRecvChunk(SOCK_AND (char*)&hdr, sizeof hdr);
  if (e) return e;

  if (hdr.len < sizeof (struct dhcp)) {
    return BAD_OFFER_LEN;
  }
  e = RecvDhcpReply(SOCK_AND &hdr, second);
  return e;
}

errnum RunDhcp(const struct sock* sockp, const char* name4, word ticks) {
  errnum e = OKAY;
  byte random = 31 & ticks; // use low 5 bits.
  byte hostchar = (random > 9) ? 'A' + random - 10 : '0' + random;

  // 16 possible MACs.
  memset(Vars->ip_addr, 0, 4);
  memset(Vars->ip_mask, 0, 4);
  memset(Vars->ip_gateway, 0, 4);
  memcpy(Vars->mac_addr, "\x02\x02\x02\x02\x02", 5);
  Vars->mac_addr[5] = hostchar;
  memcpy(Vars->hostname, "CocoIO_", 7);
  Vars->hostname[7] = hostchar;
  WizConfigure();

  WizOpen(SOCK_AND &BroadcastUdpProto, DHCP_CLIENT_PORT);
  UdpDial(SOCK_AND  &BroadcastUdpProto,
          /*dest_ip=*/ (const byte*)SixFFs, DHCP_SERVER_PORT);

  // First round: Discover.
  Vars->xid[0] = 42;
  Vars->xid[1] = 42;
  Vars->xid[2] = (byte)(ticks>>8);
  Vars->xid[3] = (byte)(ticks);
  ++ticks;
  e = RunOneDhcpRound(SOCK_AND name4, false);
  if (e) { Fatal("R1DR/1", e); }

  // Second round: Request.
  Vars->xid[2] = (byte)(ticks>>8);
  Vars->xid[3] = (byte)(ticks);
  ++ticks;
  e = RunOneDhcpRound(SOCK_AND name4, true);
  if (e) { Fatal("R1DR/2", e); }

  WizClose(JUST_SOCK);
  return e;
}
