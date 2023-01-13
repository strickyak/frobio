// demo fuse daemon fuse.tftp ("/Fuse/TFTP")

#undef MAX_VERBOSE
#ifndef MAX_VERBOSE
#define MAX_VERBOSE LLStep /* Log one banner, then only errors. */
#endif

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/os9/os9defs.h" // E_EOF
#include "frob2/fuse/fuse.h"

#define DAEMON "/Fuse/Daemon/TFTP"
#define PAYSIZ 200 // Short 200 bytes, until FUSEman is fixed.
#define READ_WRITE 03
#define SHARABLE_RW 0133

struct Req {
  struct FuseRequest header;
  char payload[PAYSIZ];
} Request;

struct Rep {
  struct FuseReply header;
  char payload[PAYSIZ];
} Reply;

int DaemonFd;

//////////////////////////////////////////
//
//

#define DEFAULT_SERVER_PORT 69 /* TFTPD */

// Operations defined by TFTP protocol:
#define TFTP_READ 1
#define TFTP_WRITE 2
#define TFTP_DATA 3
#define TFTP_ACK 4
#define TFTP_ERROR 5

byte packet[666];

byte OpenLocalSocket() {
  byte socknum = 0;
  word client_port = suggest_client_port();
  prob ps = UdpOpen(client_port, &socknum);
  if (ps) LogFatal("cannot UdpOpen: %s", ps);
  return socknum;
}

void SendAck(byte socknum, word block, quad addr, word tid) {
  word* wp = (word*)packet;
  wp[0] = TFTP_ACK;
  wp[1] = block;
  prob ps = UdpSend(socknum, packet, 4, addr, tid);
  if (ps) LogFatal("cannot UdpSend ack: %s", ps);
}

void SendTftpRequest(byte socknum, quad host, word port, word opcode, const char* filename, bool ascii) {
  char* p = (char*)packet;
  *(word*)p = opcode;
  p += 2;

  int n = strlen(filename);
  strcpy(p, filename);
  p += n+1;

  const char* mode = ascii ? "netascii" : "octet";
  n = strlen(mode);
  strcpy(p, mode);
  p += n+1;

  prob ps = UdpSend(socknum, packet, p-(char*)packet, host, port);
  if (ps) LogFatal("cannot UdpSend request: %s", ps);
}

struct Fragment {
  struct Fragment* next;
  word len;
  // payload follows
};

// TODO: handle more than one connection.
// For now, a single Fragment list is rooted here:
struct Fragment *FragRoot;

void AppendToFragList(struct Fragment* frag) {
  struct Fragment **pred = &FragRoot;
  for (struct Fragment* p = FragRoot; p; p = p->next) {
    pred = &p->next;
  }
  *pred = frag;
}

void ResetFragList() {
  struct Fragment* p = FragRoot;
  while (p) {
    struct Fragment* next = p->next;
    Free( (char*) p );
    p = next;
  }
  FragRoot = NULL;
}

void SaveBytesToFragList(char* data, word len) {
  if (!len) return;  // Last packet can be empty.

  struct Fragment* frag = (struct Fragment*) Malloc(len + sizeof (struct Fragment));
  Assert(frag);
  char* fragdata = (char*)(frag + 1);
  frag->next = NULL;
  frag->len = len;
  memcpy(fragdata, data, len);

  AppendToFragList(frag);
}

// For now, TGet fatals on errors.
void TGet(const char* server_host_spec, const char* filename) {
  errnum err;
  prob ps;
  word total_len = 0;
  Assert(FragRoot == NULL);

  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&server_host_spec, &server_port); 
  byte socknum = OpenLocalSocket();
  SendTftpRequest(socknum, server_addy, server_port, TFTP_READ, filename, /*ascii=*/0);

  word expected_block = 1;
  while (1) {
    word size = sizeof packet;
    quad from_addr = 0;
    word from_port = 0;
    ps = UdpRecv(socknum, packet, &size, &from_addr, &from_port);
    if (ps) LogFatal("cannot UdpRecv: %s", ps);
    word type = ((word*)packet)[0];
    word arg = ((word*)packet)[1];

    LogDetail("TFTP sz=%x t=%x a=%x", size, type, arg);

    switch (type) {
    case TFTP_DATA:
        if (arg != expected_block) {
            // Don't ack the wrong block.
            break;
        }
        word len = size - 4;
        SendAck(socknum, arg, from_addr, from_port);
        // TODO -- send more Acks for last block, if we don't get the next one.

        SaveBytesToFragList(4+(char*)packet, len);
        total_len += len;

        if (len < 512) goto END_LOOP;

        ++expected_block;
        break;
    case TFTP_ERROR:
        // arg is error number.
      LogFatal("*** Server error %d: %s", arg, packet+4);
      break;
    default:
      LogFatal("TGet() did not expect to recv type %d", type);
    }
  }  // while(1)
END_LOOP:
  ps = UdpClose(socknum);
  if (ps) {
    LogFatal("ERROR, Cannot close UDP socket: %s", ps);
  }
  LogStatus("OKAY: got $%x bytes for %q", total_len, filename);
}

//
//
//////////////////////////////////////////


void HexDump(char* payload, word size) {  // Just verbosity.
#if MAX_VERBOSE >= LLDebug
    if (size > 1030) {
      LogFatal("HexDump: too big: %x", size);
    }
    for (word i = 0; i < size; i += 16) {
      if (i>=32) break; ///////////////////////// STOP SHORT
      Buf buf;
      BufInit(&buf);
      BufFormat(&buf, "%04x: ", i);
      for (word j = 0; j < 16; j++) {
        if (i+j < size) {
          BufFormat(&buf, "%02x ", payload[i+j]);
        } else {
          BufFormat(&buf, "   ");
        }
        if ((j&3)==3) BufAppC(&buf, ' ');
      }
      BufAppC(&buf, ' ');
      for (word j = 0; j < 16 && (i+j) < size; j++) {
        char ch = payload[i+j];
        if (32 <= ch && ch <= 126) {
          BufAppC(&buf, ch);
        } else {
          BufAppC(&buf, '.');
        }
      }
      BufAppC(&buf, '\n');
      FPuts(BufFinish(&buf), StdOut);
      BufDel(&buf);
    }
    Printf("\n");
#endif
}

//////////////////////////////////////////

#define MAX_PATHS 32 // Limit on internal OS9 path number.

// Requests identify which internal OS9 path they are coming from.
// Since OS9 cannot support more than a couple dozen paths,
// we map them one-to-one into this array, even if most slots
// are never used.
struct PathInfo {
  struct Fragment* root;
  struct Fragment* current;
  word off; // consumed offset in current fragment.
} Paths[MAX_PATHS];

void ShowFragments(struct Fragment* f) {
  while (f) {
    LogDebug("Fragment @%x len %x", (word)&f, f->len);
    f = f->next;
  }
  LogDebug("End Fragment List");
}

void ShowPathInfo(struct PathInfo* p) {  // Just verbosity.
  if (p) {
      LogDebug("PathInfo[%x] { root@%x current@%x", p-Paths, p->root, p->current);
      ShowFragments(p->root);
  } else {
    LogDebug("p=NULL");
  }
}

word CopyBytesToBuffer(struct PathInfo* p, char* buf, word buf_size, bool linely) {
  char* b = buf;
  word togo = buf_size;

  while (togo) {
    struct Fragment* f = p->current;
    if (!f) break;
    if (p->off >= f->len) {
      p->current = f->next;
      p->off = 0;
      continue;
    }
    char* pay = (char*)(f+1);

    if (linely) {
      char ch = pay[p->off];
      /*
      if (ch == '\0') {
        p->off++;
        LogDebug("CopyBToB: NUL off=%x", p->off);
        break;
      }
      */
      if (ch==0) ch='~';  // Make data NULs visible as `~`
      *b++ = ch;
      p->off++;
      --togo;
      LogDebug("CopyBToB: ch=%x off=%x togo=%x", p->off);
      if (ch == '\n' || ch == '\r') break;
    } else {
      word span = MIN(togo, f->len - p->off);
      Assert(span);
      memcpy(b, pay+p->off, span);
      b += span;
      p->off += span;
      togo -= span;
    }
  }

  LogInfo("CopyBToB buf_size=%x - togo=%x = %x", buf_size, togo, buf_size - togo);
  return buf_size - togo;  // how many bytes consumed & copied.
}

//////////////////////////////////////////

const char* StrDupRequestedFileName() {
  // Where we get the filename.
  char* pay = Request.payload;
  word n = Request.header.size;

  // Malloc a string for the filename. 
  char* s = (char*) Malloc(n + 1);
  memcpy(s, pay, n);
  s[n] = '\0';

  // PrsNam includes trailing spaces at the end
  // of the path.  This gets rid of trailing
  // whitespace.
  for (word i = 0; i < n; i++) {
    if (s[i] <= 32) s[i] = '\0';
  }

  return (const char*)s;
}

bool SplitThirdAndRest(const char* s, const char** third_out, const char** rest_out) {
  // Skip first name, e.g. "FUSE"
  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  while (*s > 32 && *s != '/') s++;  // Skip component
  if (*s <= 32) return false;

  // Skip second name, e.g. "TFTP"
  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  while (*s > 32 && *s != '/') s++;  // Skip component
  if (*s <= 32) return false;

  // Remember third name, e.g. "10.5.5.5"
  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  const char* third_begin = s;
  while (*s > 32 && *s != '/') s++;  // Skip component
  if (*s <= 32) return false;

  *(char*)s = '\0'; // Temporarily alter input string.
  *third_out = StrDup((byte*)third_begin);
  *(char*)s = '/'; // Restore input string.
  s++;

  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  *rest_out = StrDup((byte*)s);
  return true;
}

errnum DoOpenRead() {
  const char* name = StrDupRequestedFileName();
  const char* third = NULL;
  const char* rest = NULL;
  bool ok = SplitThirdAndRest(name, &third, &rest);

  TGet(third, rest); // for now, TGet fatals on errors.
  // Result is returned in FragRoot list.

  struct PathInfo *p = &Paths[Request.header.path_num];
  p->root = FragRoot;
  p->current = p->root;
  p->off = 0;

  LogInfo("DoOpenRead: path=%x file=%q", Request.header.path_num, name);
  Free((void*)name);
  return 0;
}

void DoOpen() {
  errnum e;
  switch (Request.header.a_reg) {
    case 1: // Read
        e = DoOpenRead();
        break;
    case 2: // Write
        e = E_UNKSVC;  // Write not supported.
        break;
    default:
        e = E_BMODE; // "Bad Mode"
  }

  Reply.header.status = e;
  Reply.header.size = 0;
}
void DoClose() {
  struct PathInfo *p = &Paths[Request.header.path_num];
  LogInfo("DoClose: path=%x", Request.header.path_num);
  ResetFragList();
  p->root = NULL;
  p->current = p->root;
  p->off = 0;

  Reply.header.status = 0;
  Reply.header.size = 0;
}

void DoReadLn() {
  errnum e = 0;
  struct PathInfo *p = &Paths[Request.header.path_num];
  LogInfo("DoReadLn path=%x", Request.header.path_num);
  ShowPathInfo(p);

  word cc = CopyBytesToBuffer(p, Reply.payload, PAYSIZ, /*linely=*/true);
  LogInfo("DoReadLn cc=%x", cc);

  if (cc) {
    Reply.header.status = 0;
    //? Reply.header.size = cc + sizeof Reply;
    Reply.header.size = cc;
  } else {
    Reply.header.status = E_EOF;
    //? Reply.header.size = sizeof Reply;
    Reply.header.size = 0;
  }
  return;
}

void DoWritLn() {
  Reply.header.status = E_UNKSVC;
  Reply.header.size = 0;
}

void DoRead() {
  Reply.header.status = E_UNKSVC;
  Reply.header.size = 0;
}

void DoWrite() {
  Reply.header.status = E_UNKSVC;
  Reply.header.size = 0;
}

//////////////////////////////////////////

int main(int argc, char* argv[]) {
  // We can ignore argc, argv.
  LogStep("Starting.");

  // Open the /FUSE device in *Daemon Mode*.
  // That is, open the path name "/Fuse/Daemon/TFTP"
  // where the second component "Daemon" means we are the daemon,
  // and the third component "TFTP" is which fuse we will serve.
  // Clients opening "/Fuse/TFTP/..." will send their
  // filesystem operations to this daemon.
  CheckE(Os9Create, (DAEMON, /*mode=*/READ_WRITE, /*attrs=*/SHARABLE_RW, &DaemonFd));

  // Now go into an infinite loop calling Os9Read to get the request
  // and then Os9Write to send the reply.
  while (true) {
    int cc;
    LogInfo("Reading...");
    CheckE(Os9Read, (DaemonFd, (char*)&Request, sizeof Request, &cc));
    Assert(cc >= sizeof Request.header);

    LogInfo("Read: op=%x path=%x a=%x b=%x size=%x cc=%x",
        Request.header.operation,
        Request.header.path_num,
        Request.header.a_reg,
        Request.header.b_reg,
        Request.header.size,
        cc); // Just verbosity.

    // What we do depends on the operation, so we have a big switch.
    // Different operations need different handling.
    int n = sizeof Reply.header;
    switch (Request.header.operation) {
      case OP_CREATE:
      case OP_OPEN: DoOpen(); break;
      case OP_CLOSE: DoClose(); break;
      case OP_READ: DoRead(); n += Reply.header.size; break;
      case OP_READLN: DoReadLn(); n += Reply.header.size; break;
      case OP_WRITE: DoWrite(); break;
      case OP_WRITLN: DoWritLn(); break;
      default:
        Reply.header.status = E_UNKSVC;
        Reply.header.size = 0;
        LogError("Bad operation: %d", Request.header.operation);
    }

    LogInfo("Reply: status=%x size=%x n=%x",
        Reply.header.status,
        Reply.header.size,
        n); // Just verbosity.
    if (n > sizeof Reply.header) { // Just verbosity.
      HexDump(Reply.payload, n - sizeof Reply.header);
    }

    CheckE(Os9Write, (DaemonFd, (char*)&Reply, n, &cc));
    LogInfo("Wrote Reply.");
  }
  // NOTREACHED
  return 0;
}
