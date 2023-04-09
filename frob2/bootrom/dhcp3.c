#include "frob2/bootrom/bootrom3.h"

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
    byte options[300];  // not really 300?
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
  0x00, 0x80, // broadcast
};
const byte DiscoverOptions[] = {
  99, 130, 83, 99, // magic cookie for DHCP
  51, 1, 1,  // 53=OptionType 1=len 1=Discover
  12, 4, // 12=Hostname, 4=len4chars, FOLLOWED BY NAME
};
const byte DiscoverOptions[] = {
  99, 130, 83, 99, // magic cookie for DHCP
  51, 1, 1,  // 53=OptionType 1=len 1=Discover
  12, 4, // 12=Hostname, 4=len4chars, FOLLOWED BY NAME
};
const byte RequestOptions[] = {
  99, 130, 83, 99, // magic cookie for DHCP
  51, 1, 3,  // 53=OptionType 1=len 3=Request
  12, 4, // 12=Hostname, 4=len4chars, FOLLOWED BY NAME
};
const byte RequestAddressOption[] = {
  50, 4, // plus 4-byte address.
};
const byte EndOptions[] = {
  255, 0   // 255=end, 0=len
};

void DhcpRequest(const char* name4, bool second) {
  byte mac[6] = {
    2, 32, name4[0], name4[1], name4[2], name4[3]};
  word req_size = 236 +
      (second)
        ? (sizeof RequestOptions + 4
            + sizeof RequestAddressOption + 4)
        : (sizeof DiscoverOptions + 4)
      + sizeof EndOptions;

  WizReserveToSend(PARAM_SOCK_AND req_size);

  SendData(RequestDHCP0, sizeof RequestDHCP0);
  SendData(name4, 4);
  SendData(RequestDHCP8, sizeof RequestDHCP8);
  SendData(Eight00s, 8);  // ciaddr & yiaddr
  SendData(Eight00s, 8); // siaddr & giaddr
  SendData(mac, 6); // ch_addr
  SendData(Eight00s, 2);  // ch_addr cont'd

  byte N = 1/*chaddr cont'd*/ + 8/*sname*/ + 16/*bname*/;
  for (byte i = 0; i < N; i++) {
    SendData(Eight00s, 8); // ch_addr + sname + bname
  }

  if (second) {
    SendData(RequestOptions, sizeof RequestOptions);
    SendData(name4, 4);
    SendData(RequestAddressOption, sizeof RequestAddressOption);
    SendData(address, 4);
  } else {
    SendData(DiscoverOptions, sizeof DiscoverOptions);
    SendData(name4, 4);
  }
  SendData(EndOptions, sizeof EndOptions);

  WizFinalizeSend(PARAM_SOCK_AND BroadcastUdpProto, req_size);
}

void WasteRecvBytes(word n) {
  char junk[8];
  while (n >= 8) {
    WizRecvChunk(PARAM_SOCK_AND  buf, 8);
    n -= 8;
  }
  if (n > 0) {
    WizRecvChunk(PARAM_SOCK_AND  buf, n);
  }
}

void DhcpReply(struct UdpRecvHeader *hdr, bool second) {
  char buf[4];

  // we want 16:yiaddr (you) and 20:siaddr (server)
  assert (hdr->len >= 236);

  WasteRecvBytes(16);
  WizRecvChunk(PARAM_SOCK_AND  Vars->ip_addr, 4);
  WizRecvChunk(PARAM_SOCK_AND  Vars->ip_server, 4);
  WasteRecvBytes(236 - 24);

  WizRecvChunk(PARAM_SOCK_AND  buf, 4); // DHCP magic
  assert(buf[0] == 99);
  assert(buf[1] == 130);
  assert(buf[2] == 83);
  assert(buf[3] == 99);

  word opt_len = 0;
  do {
    opt_len += 2;
    WizRecvChunk(PARAM_SOCK_AND  buf, 2); // Option & Len
                                          //
#define OPT buf[0]
#define LEN buf[1]

    opt_len += LEN;
    if (len == 4) {
      WizRecvChunk(PARAM_SOCK_AND  buf, 4); // 4 byte IP addr
      if (OPT == 1) {
        memcpy(Vars->ip_mask, buf, 4);
      } else if (OPT == 3) {
        memcpy(Vars->ip_gateway, buf, 4);
      } else if (OPT == 6) {
        memcpy(Vars->ip_resolver, buf, 4);
      } else {
        // just dont use buf.
      }
    } else {
      WasteRecvBytes(LEN);
    }
  } while (OPT != 255);

  // how much extra junk?
  word junk_len = hdr->len - 236 - opt_len;
  WasteRecvBytes(junk_len);
}

errnum RunOneDhcpRound(const char* name4, bool second) {
  struct UdpRecvHeader hdr;
  DhcpRequest(name4, second);

  bool ok = WizRecvChunk(PARAM_SOCK_AND (char*)&hdr, sizeof hdr);
  assert(ok);
  if (hdr.len < sizeof Offfer) {
     return BAD_OFFER_LEN;
  }
  DhcpReply(SOCK_AND &hdr);
}

errnum RunDhcp(const char* name4) {
  // First round: Discover.
  errnum e = RunOneDhcpRound(name4, false);
  if (e) return e;
  // Second round: Request.
  e = RunOneDhcpRound(name4, true);
  if (e) return e;
}
