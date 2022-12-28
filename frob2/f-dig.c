// f-dig server-addr:123 query

// HINT: HOW TO SERVE DNS RESPONSES on an interface (e.g. on Ubuntu 20.04)
// when systemd-resolved is bound to localhost:
//
// $ sudo socat UDP-LISTEN:53,fork,reuseaddr,bind=10.2.2.2 UDP:127.0.0.53:53

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

#define DEFAULT_SERVER_PORT 53 /* named */

byte packet[1550];

struct header {
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

byte OpenLocalSocket() {
  byte socknum = 0;
  word client_port = suggest_client_port();
  errnum err = UdpOpen(client_port, &socknum);
  if (err) LogFatal("cannot UdpOpen: %d\n", err);
  LogDebug("OpenLS=>%x", socknum);
  return socknum;
}

void dump_with_decimal_offset(byte* p, word len) {
    for (int d = 0; d < len; d += 10) {
        printf("%3d: ", d);
        for (int i = d; i < d+10 && i<len; i++) {
            printf("%02x ", p[i]);
        }
        printf("\n");
    }
}

// encode_name converts name s into DNS name format
// starting at buf.  Returns the last address used plus 1,
// where the next field will follow.
byte* encode_name(byte* buf, char* s) {
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

void SendRequest(byte socknum, quad host, word port, char* query, word type) {
  word n = strlen(query);
  Assert(n>0);
  Assert(n<256);

  // packet is initially zero because C.
  struct header *h = (struct header*)packet;
  h->id = WizTicks();
  h->qr_opcode = QR_OPCODE_QUERY;
  h->qd_count = 1;
  byte* p = (byte*)(h+1);
  p = encode_name(p, query);
  word* wp = (word*)p;
  wp[0] = type;
  wp[1] = 1 /* = IN, the internet */;
  p = (byte*)(wp+2);  // End of used part.
  word packet_size = p - packet;

  errnum err = UdpSend(socknum, packet, packet_size, host, port);
  if (err) LogFatal("cannot UdpSend request: %d\n", err);
  memset(packet, 0, packet_size);  // paranoidly prepare to receive.
}

byte* print_name(byte* base, byte* p) {
    byte patience = 64;
    byte* override_result = 0;
    while (--patience) {
        byte first = *p++;
        if (first==0) {
            return override_result ? override_result : p;  // Name is done.
        } else if (first < 64) {
            for (byte i = 0; i < first; i++) {
                byte c = *p++;
                if (c=='-' || '/'<=c && c<='z') putchar(c); else printf("\\x%02x", c);
            }
            putchar('.');
        } else if (first >= 192) {
            word shortcut = ((word)(first-192) << 8) + *p;
            override_result = p+1;
            p = base+shortcut;  // continue there.
        } else {
            LogFatal("decode_name: bad byte at offset %d", (word)(p-1));
            return 0;  // NOTREACHED
        }
    }
    LogFatal("decode_name: caught in a loop? %d", (word)base);
}

void print_dns_cls_name(int cls) {
    switch (cls) {
    case 1:
        printf(" IN ");
        break;
    default:
        printf(" %d ", cls);
    }
}

void print_dns_type_name(int cls) {
    switch (cls) {
    case 1:
        printf(" A ");
        break;
    case 2:
        printf(" NS ");
        break;
    case 5:
        printf(" CNAME ");
        break;
    case 6:
        printf(" SOA ");
        break;
    case 12:
        printf(" PTR ");
        break;
    case 15:
        printf(" MX ");
        break;
    case 16:
        printf(" TXT ");
        break;
    case 28:
        printf(" AAAA ");
        break;
    default:
        printf(" %d ", cls);
    }
}

byte* print_record(byte* base, byte* p, bool is_question) {
    if (64 <= *p && *p < 192) {
        printf("... dns extensions?\n");
        exit(0);
    }
    p = print_name(base, p);
    word type = *(word*)(p);
    word cls = *(word*)(p+2);
    if (is_question) {
      // question record is stunted.
      print_dns_cls_name(cls);
      print_dns_type_name(type);
      printf("\n");
      return p+4;
    } else {
      // non-question records have ttl and payload.
      quad ttl = *(quad*)(p+4);
      word len = *(word*)(p+8);
      p += 10;
      printf(" %8ld ", ttl);
      print_dns_cls_name(cls);
      print_dns_type_name(type);

      if (false) {
          printf("{ len=%d ", len);
          for (word i = 0; i < len && i < 40; i++) {
            printf("%02x ", p[i]);
            if ((i&3)==3) printf(" ");
          }
          printf("}\n");
      }

      if (cls==1 && type==1) { // A
        for (word i = 0; i < len; i++) { if (i) putchar('.'); printf("%d", p[i]); }
      } else if (cls==1 && type==28) { // AAAA
        for (word i = 0; i < len; i++) { if (i) putchar(':'); printf("%02x", p[i]); }
      } else if (cls==1 && type==2) { // NS
        print_name(base, p);
      } else if (cls==1 && type==5) { // CNAME
        print_name(base, p);
      } else if (cls==1 && type==12) { // PTR
        print_name(base, p);
      } else if (cls==1 && type==15) { // MX
        printf(" %u ", *(word*)p);
        print_name(base, p+2);
      } else if (cls==1 && type==6) { // SOA
        byte* s = print_name(base, p);
        printf("  ");
        s = print_name(base, s);
        quad* q = (quad*) s;
        printf("  %ld  %ld  %ld  %ld", q[0], q[1], q[2], q[3]);
      } else if (cls==1 && (type==16 || type == 10)) { // TXT or NULL
        putchar('"');
        for (word i = 1; i < len && i < p[0] && i < 100; i++) {
          byte c = p[i];
          if (32<=c && c<127 && c!='"') putchar(c); else printf("\\x%02x", c);
        }
        putchar('"');
      } else {
        printf(" (len=%d) ", len);
        for (word i = 0; i < len && i<40; i++) { printf("%02x ", p[i]); }
        for (word i = 0; i < len && i<40; i++) { byte c = p[i]; printf("%c", (32<=c && c<127) ? c : '~'); }
      }
      printf("\n");

      return p + len;
    }
}

void Resolv(byte socknum, quad server_host, word server_port, char* query, word type) {
  SendRequest(socknum, server_host, server_port, query, type);

  word size = sizeof packet;
  quad from_addr = 0;
  word from_port = 0;
  errnum err = UdpRecv(socknum, packet, &size, &from_addr, &from_port);
  if (err) LogFatal("cannot UdpRecv data: %d\n", err);

  struct header *h = (struct header*)packet;
  printf(";; id %x qr_opcode %x rcode %x\n", h->id, h->qr_opcode, h->rcode);
  if (!h->qd_count) printf(";; %x questions, %x answers, %x authorities, %x additionals\n",
                h->qd_count, h->an_count, h->ns_count,  h->ar_count);

  byte* p = (byte*)(h+1);

  if (h->qd_count>0) printf(";; QUESTION:\n");
  for (word i=0; i<h->qd_count; i++) { p = print_record(packet, p, /*is_question=*/true); }

  if (h->an_count>0) printf(";; ANSWER:\n");
  for (word i=0; i<h->an_count; i++) { p = print_record(packet, p, /*is_question=*/false); }

  if (h->ns_count>0) printf(";; AUTHORITY:\n");
  for (word i=0; i<h->ns_count; i++) { p = print_record(packet, p, /*is_question=*/false); }

  if (h->ar_count>0) printf(";; ADDITIONAL:\n");
  for (word i=0; i<h->ar_count; i++) { p = print_record(packet, p, /*is_question=*/false); }

  err = UdpClose(socknum);
  if (err) {
    LogFatal("ERROR, Cannot close UDP socket: errnum %d", err);
  }
}

static void FatalUsage() {
    LogFatal("Usage:  f-dig -w0xFF68 -a -tN server_addr:53 www.example.com\n"
    "  (-a for all types)  (-tN for only type N, decimal integer)\n");
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  word type = 1 /*=A*/;
  while (GetFlag(&argc, &argv, "at:v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'a':
        type = 0x00ff; /* = all types */
        break;
      case 't':
        type = atoi(argv[0]+2);
        break;
      case 'v':
        Verbosity = (byte)prefixed_atoi(FlagArg);
        break;
      case 'w':
        WizHwPort = (byte*)prefixed_atoi(FlagArg);
        break;
      default:
        FatalUsage();
    }
  }

  if (argc != 2) {
    FatalUsage();
  }
  const char* parse = argv[0];
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&parse, &server_port); 
  byte socknum = OpenLocalSocket();
  Resolv(socknum, server_addy, server_port, /*query=*/argv[1], type);

  return OKAY;
}

/*

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+


where:

NAME            an owner name, i.e., the name of the node to which this
                resource record pertains.

TYPE            two octets containing one of the RR TYPE codes.

CLASS           two octets containing one of the RR CLASS codes.

TTL             a 32 bit signed integer that specifies the time interval
                that the resource record may be cached before the source
                of the information should again be consulted.  Zero
                values are interpreted to mean that the RR can only be
                used for the transaction in progress, and should not be
                cached.  For example, SOA records are always distributed
                with a zero TTL to prohibit caching.  Zero values can
                also be used for extremely volatile data.

RDLENGTH        an unsigned 16 bit integer that specifies the length in
                octets of the RDATA field.



Mockapetris                                                    [Page 11]

RFC 1035        Domain Implementation and Specification    November 1987


RDATA           a variable length string of octets that describes the
                resource.  The format of this information varies
                according to the TYPE and CLASS of the resource record.

3.2.2. TYPE values

TYPE fields are used in resource records.  Note that these types are a
subset of QTYPEs.

TYPE            value and meaning

A               1 a host address

NS              2 an authoritative name server

MD              3 a mail destination (Obsolete - use MX)

MF              4 a mail forwarder (Obsolete - use MX)

CNAME           5 the canonical name for an alias

SOA             6 marks the start of a zone of authority

MB              7 a mailbox domain name (EXPERIMENTAL)

MG              8 a mail group member (EXPERIMENTAL)

MR              9 a mail rename domain name (EXPERIMENTAL)

NULL            10 a null RR (EXPERIMENTAL)

WKS             11 a well known service description

PTR             12 a domain name pointer

HINFO           13 host information

MINFO           14 mailbox or mail list information

MX              15 mail exchange

TXT             16 text strings

3.2.3. QTYPE values

QTYPE fields appear in the question part of a query.  QTYPES are a
superset of TYPEs, hence all TYPEs are valid QTYPEs.  In addition, the
following QTYPEs are defined:



Mockapetris                                                    [Page 12]

RFC 1035        Domain Implementation and Specification    November 1987


AXFR            252 A request for a transfer of an entire zone

MAILB           253 A request for mailbox-related records (MB, MG or MR)

MAILA           254 A request for mail agent RRs (Obsolete - see MX)

*               255 A request for all records

3.2.4. CLASS values

CLASS fields appear in resource records.  The following CLASS mnemonics
and values are defined:

IN              1 the Internet

CS              2 the CSNET class (Obsolete - used only for examples in
                some obsolete RFCs)

CH              3 the CHAOS class

HS              4 Hesiod [Dyer 87]

===================


4. MESSAGES

4.1. Format

All communications inside of the domain protocol are carried in a single
format called a message.  The top level format of message is divided
into 5 sections (some of which are empty in certain cases) shown below:

    +---------------------+
    |        Header       |
    +---------------------+
    |       Question      | the question for the name server
    +---------------------+
    |        Answer       | RRs answering the question
    +---------------------+
    |      Authority      | RRs pointing toward an authority
    +---------------------+
    |      Additional     | RRs holding additional information
    +---------------------+

RFC 1035        Domain Implementation and Specification    November 1987


4.1.1. Header section format

The header contains the following fields:

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

where:

ID              A 16 bit identifier assigned by the program that
                generates any kind of query.  This identifier is copied
                the corresponding reply and can be used by the requester
                to match up replies to outstanding queries.

QR              A one bit field that specifies whether this message is a
                query (0), or a response (1).

OPCODE          A four bit field that specifies kind of query in this
                message.  This value is set by the originator of a query
                and copied into the response.  The values are:

                0               a standard query (QUERY)

                1               an inverse query (IQUERY)

                2               a server status request (STATUS)

                3-15            reserved for future use

AA              Authoritative Answer - this bit is valid in responses,
                and specifies that the responding name server is an
                authority for the domain name in question section.

                Note that the contents of the answer section may have
                multiple owner names because of aliases.  The AA bit



Mockapetris                                                    [Page 26]

RFC 1035        Domain Implementation and Specification    November 1987


                corresponds to the name which matches the query name, or
                the first owner name in the answer section.

TC              TrunCation - specifies that this message was truncated
                due to length greater than that permitted on the
                transmission channel.

RD              Recursion Desired - this bit may be set in a query and
                is copied into the response.  If RD is set, it directs
                the name server to pursue the query recursively.
                Recursive query support is optional.

RA              Recursion Available - this be is set or cleared in a
                response, and denotes whether recursive query support is
                available in the name server.

Z               Reserved for future use.  Must be zero in all queries
                and responses.

RCODE           Response code - this 4 bit field is set as part of
                responses.  The values have the following
                interpretation:

                0               No error condition

                1               Format error - The name server was
                                unable to interpret the query.

                2               Server failure - The name server was
                                unable to process this query due to a
                                problem with the name server.

                3               Name Error - Meaningful only for
                                responses from an authoritative name
                                server, this code signifies that the
                                domain name referenced in the query does
                                not exist.

                4               Not Implemented - The name server does
                                not support the requested kind of query.

                5               Refused - The name server refuses to
                                perform the specified operation for
                                policy reasons.  For example, a name
                                server may not wish to provide the
                                information to the particular requester,
                                or a name server may not wish to perform
                                a particular operation (e.g., zone



Mockapetris                                                    [Page 27]

RFC 1035        Domain Implementation and Specification    November 1987


                                transfer) for particular data.

                6-15            Reserved for future use.

QDCOUNT         an unsigned 16 bit integer specifying the number of
                entries in the question section.

ANCOUNT         an unsigned 16 bit integer specifying the number of
                resource records in the answer section.

NSCOUNT         an unsigned 16 bit integer specifying the number of name
                server resource records in the authority records
                section.

ARCOUNT         an unsigned 16 bit integer specifying the number of
                resource records in the additional records section.

4.1.2. Question section format

The question section is used to carry the "question" in most queries,
i.e., the parameters that define what is being asked.  The section
contains QDCOUNT (usually 1) entries, each of the following format:

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

where:

QNAME           a domain name represented as a sequence of labels, where
                each label consists of a length octet followed by that
                number of octets.  The domain name terminates with the
                zero length octet for the null label of the root.  Note
                that this field may be an odd number of octets; no
                padding is used.

QTYPE           a two octet code which specifies the type of the query.
                The values for this field include all codes valid for a
                TYPE field, together with some more general codes which
                can match more than one type of RR.



Mockapetris                                                    [Page 28]

RFC 1035        Domain Implementation and Specification    November 1987


QCLASS          a two octet code that specifies the class of the query.
                For example, the QCLASS field is IN for the Internet.

4.1.3. Resource record format

The answer, authority, and additional sections all share the same
format: a variable number of resource records, where the number of
records is specified in the corresponding count field in the header.
Each resource record has the following format:
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

where:

NAME            a domain name to which this resource record pertains.

TYPE            two octets containing one of the RR type codes.  This
                field specifies the meaning of the data in the RDATA
                field.

CLASS           two octets which specify the class of the data in the
                RDATA field.

TTL             a 32 bit unsigned integer that specifies the time
                interval (in seconds) that the resource record may be
                cached before it should be discarded.  Zero values are
                interpreted to mean that the RR can only be used for the
                transaction in progress, and should not be cached.





Mockapetris                                                    [Page 29]

RFC 1035        Domain Implementation and Specification    November 1987


RDLENGTH        an unsigned 16 bit integer that specifies the length in
                octets of the RDATA field.

RDATA           a variable length string of octets that describes the
                resource.  The format of this information varies
                according to the TYPE and CLASS of the resource record.
                For example, the if the TYPE is A and the CLASS is IN,
                the RDATA field is a 4 octet ARPA Internet address.

4.1.4. Message compression

In order to reduce the size of messages, the domain system utilizes a
compression scheme which eliminates the repetition of domain names in a
message.  In this scheme, an entire domain name or a list of labels at
the end of a domain name is replaced with a pointer to a prior occurance
of the same name.

The pointer takes the form of a two octet sequence:

    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | 1  1|                OFFSET                   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

The first two bits are ones.  This allows a pointer to be distinguished
from a label, since the label must begin with two zero bits because
labels are restricted to 63 octets or less.  (The 10 and 01 combinations
are reserved for future use.)  The OFFSET field specifies an offset from
the start of the message (i.e., the first octet of the ID field in the
domain header).  A zero offset specifies the first byte of the ID field,
etc.

The compression scheme allows a domain name in a message to be
represented as either:

   - a sequence of labels ending in a zero octet

   - a pointer

   - a sequence of labels ending with a pointer

Pointers can only be used for occurances of a domain name where the
format is not class specific.  If this were not the case, a name server
or resolver would be required to know the format of all RRs it handled.
As yet, there are no such cases, but they may occur in future RDATA
formats.

If a domain name is contained in a part of the message subject to a
length field (such as the RDATA section of an RR), and compression is



Mockapetris                                                    [Page 30]

RFC 1035        Domain Implementation and Specification    November 1987


used, the length of the compressed name is used in the length
calculation, rather than the length of the expanded name.

==================================================================================
==================================================================================
==================================================================================

https://unix.stackexchange.com/questions/591203/understanding-the-digs-dns-query-does-dig-set-non-zero-value-for-z-field

RFC 1035 on this subject was amended by RFC2535 "DNS Security Extensions".

Its section 6.1 shows the message format:


                                           1  1  1  1  1  1
             0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
            |                      ID                       |
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
            |QR|   Opcode  |AA|TC|RD|RA| Z|AD|CD|   RCODE   |
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
            |                    QDCOUNT                    |
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
            |                    ANCOUNT                    |
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
            |                    NSCOUNT                    |
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
            |                    ARCOUNT                    |
            +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
As you can see, the 3 bits previous Z field is now split between Z, AD and CD.

Hence, your 0x20 = 0b00100000 is to be split as follows:

RA = 0 : recursion NOT available from the server replying this message
Z = 0 : as expected, should be 0
AD = 1 : "all the data included in the answer and authority portion of the response has been authenticated by the server according to the policies of that server."
CD = 0 : checking not disabled (makes sense in query, not in a response, it is defined as "Pending (non-authenticated) data is acceptable to the resolver sending the query.")
RCODE = 0 : No error

==================================================================================
==================================================================================
==================================================================================

$ sudo socat -x -v -v -v UDP-LISTEN:53,fork,reuseaddr,bind=10.2.2.2 UDP:8.8.8.8:53

$ dig @10.2.2.2 nando.yak.net

> 2022/07/17 08:38:57.276284  length=32 from=0 to=31
 1c 0a                                            ..
 01 00 00 01 00 00 00 00 00 00 05 6e 61 6e 64 6f  ...........nando
 03 79 61 6b 03 6e 65 74 00 00 00 01 00 01        .yak.net......
--
< 2022/07/17 08:38:57.283059  length=31 from=0 to=30
 1c 0a                                            ..
 81 82 00 01 00 00 00 00 00 00 05 6e 61 6e 64 6f  ...........nando
 03 79 61 6b 03 6e 65 74 00 00 00 01 00           .yak.net.....
--


> 2022/07/17 08:48:53.681610  length=54 from=0 to=53
 e1 93 01 20 00 01 00 00 00 00 00 01 05 6e 61 6e  ... .........nan
 64 6f 03 79 61 6b 03 6e 65 74 00 00 01 00 01 00  do.yak.net......
 00 29 10 00 00 00 00 00 00 0c 00 0a              .)..........
 00 08 f1 2e e3 b3 4e 7c b6 18                    ......N|..
--
< 2022/07/17 08:48:53.704994  length=58 from=0 to=57
 e1 93 81 80 00 01 00 01 00 00 00 01 05 6e 61 6e  .............nan
 64 6f 03 79 61 6b 03 6e 65 74 00 00 01 00 01 c0  do.yak.net......
 0c 00 01 00 01 00 00 04 57 00 04 9f cb 9c a2 00  ........W.......
 00 29 02 00 00 00 00 00 00 00                    .)........
--

=========================================================================
; <<>> DiG 9.11.3-1ubuntu1.17-Ubuntu <<>> @10.2.2.2 nando.yak.net
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 57747
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 512
;; QUESTION SECTION:
;nando.yak.net.			IN	A

;; ANSWER SECTION:
nando.yak.net.		1111	IN	A	159.203.156.162

;; Query time: 25 msec
;; SERVER: 10.2.2.2#53(10.2.2.2)
;; WHEN: Sun Jul 17 08:48:53 PDT 2022
;; MSG SIZE  rcvd: 58
=========================================================================

https://serverfault.com/questions/1022160/additional-information-in-dns-query-with-dig

https://en.wikipedia.org/wiki/Extension_Mechanisms_for_DNS

=========================================================================

https://datatracker.ietf.org/doc/html/rfc2671

The fixed part of an OPT RR is structured as follows:

     Field Name   Field Type     Description
     ------------------------------------------------------
     NAME         domain name    empty (root domain)
     TYPE         u_int16_t      OPT
     CLASS        u_int16_t      sender's UDP payload size
     TTL          u_int32_t      extended RCODE and flags
     RDLEN        u_int16_t      describes RDATA
     RDATA        octet stream   {attribute,value} pairs

4.4. The variable part of an OPT RR is encoded in its RDATA and is
     structured as zero or more of the following:

                +0 (MSB)                            +1 (LSB)
     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
  0: |                          OPTION-CODE                          |
     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
  2: |                         OPTION-LENGTH                         |
     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
  4: |                                                               |
     /                          OPTION-DATA                          /
     /                                                               /
     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

   OPTION-CODE    (Assigned by IANA.)

   OPTION-LENGTH  Size (in octets) of OPTION-DATA.

   OPTION-DATA    Varies per OPTION-CODE.

*/
