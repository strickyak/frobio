// status: not compiling

#include "frob2/frobtype.h"
#include "frob2/froblib.h"
#include "frob2/frobnet.h"

struct dns_header {
    word id;
    byte qr_opcode;
    byte rcode;
    word qd_count;  // questions
    word an_count;  // answers
    word ns_count;  // authorities
    word ar_count;  // additionals
};

#define QR_OPCODE_QUERY 0x01 // qr=query, op=query, recursion desired.
#define QR_OPCODE_REPLY 0x80 // use for bit mask and check value
#define RCODE_OK 0 // 0 is good, all else are errors.

// DnsEncodeName converts name s into DNS name format
// starting at buf.  Returns the last address used plus 1,
// where the next field will follow.
byte* DnsEncodeName(byte* buf, char* s) {
    byte* sizep = buf;
    byte* p = (byte*) buf+1;
    while(true) {
        switch (*s) {
            case '.':
            case '\0': {
                if (p[0]=='.' && p[1]=='\0') s++;  // Allow trailing dot, but skip it.

                word len = p - sizep - 1;
                Assert(len<64);
                *sizep = (byte)len;
                sizep = p++;
                if (*s) s++;  // consume dot but not EOS.
                if (!*s) goto END_WHILE;
            } break;
            default:
                *p++ = (byte)(*s++);
        }
    }
END_WHILE:
    *sizep = '\0';  // Size 0 means End Of Name.
    return sizep+1;  // Where next field will follow.
}

byte* DnsSkipName(byte* base, byte* p) {
  byte first = *p++;
  while (1) {
    if (first == 0) {
      return p;
    } else if (first < 64) {
      p += first;
    } else if (first >= 192) {
      return p+1;
    } else {
      // Corruption.
      return NULL;
    }
  }
}

void DnsSendRequest(byte* buffer256, byte socknum, const byte* dns_server, word dns_port, char* query, word* id_out) {
  struct dns_header *h = (struct dns_header*)buffer256;
  memset(h, 0, sizeof *h);

  *id_out = h->id = WizTicks();
  h->qr_opcode = QR_OPCODE_QUERY;
  h->qd_count = 1;

  byte* p = (byte*)(h+1);
  p = DnsEncodeName(p, query);

  word* wp = (word*)p;
  wp[0] = 1 /* = A, ipv4 address */;
  wp[1] = 1 /* = IN, the internet */;

  p = (byte*)(wp+2);  // End of used part.
  word packet_size = p - buffer256;

  prob err = UdpSend(socknum, buffer256, packet_size, dns_server, dns_port);
  if (err) return err;
}

prob Resolv(byte socknum, byte* buffer256, const byte* dns_server, word dns_port, const char* query, byte* addr4_out) {
  prob err = GOOD;
  word id;
  err = DnsSendRequest(buffer256, socknum, dns_server, server_port, query, &id);
  if (err) return err;

  word size = 256;
  byte from_addr[4];
  word from_port = 0;
  prob err = UdpRecv(socknum, buffer256, &size, from_addr, &from_port);
  if (err) return err;

  struct dns_header *h = (struct dns_header*) buffer256;
  byte* p = (byte*)(h+1);
  if (h->id != id) return "BadDnsID";

  // Skip the questions.
  for (word i=0; i<h->qd_count; i++) {
    p = DnsSkipName(buffer256, p);
    p += 4; // Skip four bytes that follow the name.
  }

  for (word i=0; i<h->an_count; i++) {
    p = DnsSkipName(buffer256, p);

    word type = *(word*)(p);
    word cls = *(word*)(p+2);
    memcpy(addr4_out, p+4, 4);
    return GOOD;
  }

  return "DnsNoA";
}

prob DnsResolver(const char* query, byte* buffer256, const byte* dns_server, word dns_port, byte* addr4_out) {
  byte socknum;
  prob e = UdpOpen(0x8000 | WizTicks(), &socknum);
  if (e) return e;

  e = Resolv(socknum, buffer256, dns_server, dns_port, query, addr_out);

  UdpClose(socknum);

  return e;
}
