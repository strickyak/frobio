// demo fuse daemon: fuse.n (/FUSE/N or /N)

#ifndef MAX_VERBOSE
// #define MAX_VERBOSE LLStep /* Log one banner, then only errors. */
#define MAX_VERBOSE LLDebug
#endif

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"
#include "frob3/os9/os9defs.h" // E_EOF
#include "frob3/fuse.h"

#define DAEMON "/Fuse/Daemon/N"
#define PAYSIZ 2048
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

void HexDump(char* payload, word size) {  // Just verbosity.
#if 1
#if 1 || MAX_VERBOSE >= LLDebug
    for (word i = 0; i < size; i += 16) {
      Buf buf;
      BufInit(&buf);
      BufFormat(&buf, "%04x: ", i);
      for (word j = 0; j < 16; j++) {
        if (i+j < size) {
          BufFormat(&buf, "%02x ", 0xFF & payload[i+j]);
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
#endif
}

const char* RequestedFileName() {
  // Where we get the filename.
  char* pay = Request.payload;
  word n = Request.header.size;

  // Malloc a string for the filename. 
  char* s = (char*) Malloc(n + 1);
  memcpy(s, pay, n);
  s[n] = '\0';
  LogInfo("RequestedFileName    (%x) %q", strlen(s), s);

  // There can be trailing spaces.  Zap the first one.
  for (word i = 0; i < n; i++) {
    if (s[i] <= 32) { s[i] = '\0'; break; }
  }

  LogInfo("RequestedFileName -> (%x) %q", strlen(s), s);
  return (const char*)s;
}

#if 0
errnum DoOpenWrite() {
  const char* name = RequestedFileName();
  LogInfo("DoOpenWrite: path=%x file=%q", Request.header.path_num, name);
  Free((void*)name);
  return 0;
}

errnum DoOpenRead() {
  const char* name = RequestedFileName();
  LogInfo("DoOpenRead: path=%x file=%q", Request.header.path_num, name);
  Free((void*)name);
  return 0;
}
#endif

void DoOpen(bool unused_create) {
  const char* name = RequestedFileName();
  LogInfo("DoCreat/Open: mode=%x path=%x file=%q", Request.header.a_reg, Request.header.path_num, name);
  Free((void*)name);
  errnum e = 0;
#if 0
  switch (/*mode*/Request.header.a_reg) {
    case 1: // Read
        e = DoOpenRead();
        break;
    case 2: // Write
        e = DoOpenWrite();
        break;
    default:
        e = E_BMODE; // "Bad Mode"
  }
#endif
  Reply.header.status = e;
  Reply.header.size = 0;
}
void DoClose() {
  LogInfo("DoClose: path=%x", Request.header.path_num);

  Reply.header.status = 0;
  Reply.header.size = 0;
}

void DoRead(bool linely) {
  errnum e = 0;
  LogInfo("DoRead%s path=%x", (linely? "ln" : ""), Request.header.path_num);

  memcpy(Reply.payload, "OK\r", 4);
  Reply.header.status = 0;
  Reply.header.size = 3;
  return;
}

void DoWrite(bool linely) {
  errnum e = 0;
  LogInfo("DoWrit%s path=%x", (linely? "ln" : "e"), Request.header.path_num);

  HexDump(Request.payload, Request.header.size);

  Reply.header.status = 0;
  Reply.header.size = 0;
  return;
}

void DoGetStat() {
  LogInfo("DoGetStat p=%x op=%x\n");
  Reply.header.status = 0;
  Reply.header.size = 0;
}

void DoSetStat() {
  LogInfo("DoSetStat p=%x op=%x\n");
  Reply.header.status = 0;
  Reply.header.size = 0;
}

//////////////////////////////////////////

// Fixing the size for where the first Carriage Return appears
// is the job if the daemon, becuase the FuseMan copies bytes
// here without looking at them.
void FixSizeForWritLn() {
  word n = MIN(Request.header.size, sizeof Request.payload);
  char* p = Request.payload;
  word i;
  for (i = 0; i < n; i++) {
    if (*p == '\r') {
      Request.header.size = i+1;
      return;
    }
    p++;
  }
  Request.header.size = n;
}

int main(int argc, char* argv[]) {
  // We can ignore argc, argv.
  LogStep("Starting.");

  // Open the /FUSE device in *Daemon Mode*.
  // That is, open the path name "/Fuse/Daemon/N"
  // where the second component "Daemon" means we are the daemon,
  // and the third component "N" is which fuse we will serve.
  // Clients opening "/Fuse/N/..." or "/N/..." will send their
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
    if (cc > sizeof Request.header) { // Just verbosity.
      HexDump(Request.payload, cc - sizeof Request.header);
    }

    // What we do depends on the operation, so we have a big switch.
    // Different operations need different handling.
    int n = sizeof Reply.header;
    switch (Request.header.operation) {
      case OP_CREATE: DoOpen(true); break;
      case OP_OPEN: DoOpen(false); break;
      case OP_CLOSE: DoClose(); break;
      case OP_READ: DoRead(false); n += Reply.header.size; break;
      case OP_READLN: DoRead(true); n += Reply.header.size; break;
      case OP_WRITE: DoWrite(false); break;
      case OP_WRITLN: FixSizeForWritLn(); DoWrite(true); break;
      case OP_GETSTAT: DoGetStat(); break;
      case OP_SETSTAT: DoSetStat(); break;
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
  return 1;
}
