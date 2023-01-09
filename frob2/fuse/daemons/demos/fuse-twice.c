// demo fuse daemon "Twice"

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/os9/os9defs.h" // E_EOF
#include "frob2/fuse/fuse.h"

#define DAEMON "/Fuse/Daemon/Twice"
#define PAYSIZ 768 // Three 256-byte blocks.
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

int Fd;
int CountLines;
char* Message;

//////////////////////////////////////////

void DoOpen() {
  Reply.header.status = 0;
  Reply.header.size = 0;
  LogInfo("TWICE does Open for client.");
  CountLines = 0;
}
void DoClose() {
  Reply.header.status = 0;
  Reply.header.size = 0;
  LogInfo("TWICE does Close for client, after %x lines were read", CountLines);
}

void DoReadLn() {
  LogInfo("TWICE DoReadLn: CountLines=%x", CountLines);

  if (CountLines >= 2) {
    Reply.header.status = E_EOF;
    Reply.header.size = 0;
    LogInfo("TWICE DoReadLn: returns E_EOF to client.");
  } else {
    strcpy(Reply.payload, Message);
    Reply.header.status = 0;
    Reply.header.size = strlen(Reply.payload);
    LogInfo("TWICE DoReadLn: returns %x bytes to client: %q", Reply.header.size, Reply.payload);
  }
  ++CountLines;
}

void DoWritLn() {
  Buf buf;
  BufInit(&buf);
  for (word i=0; Request.payload[i]; i++) {
    char ch = Request.payload[i];
    BufAppC(&buf, ch & 0x7F);
    if (ch & 0x80) break;
    if (ch == 10 || ch == 13) break;
  }
  BufFinish(&buf);
  Free(Message);
  Message = BufTake(&buf);
  LogInfo("TWICE DoWritLn: got new message len=%x %q", strlen(Message), Message);

  Reply.header.status = 0;
  Reply.header.size = strlen(Message);
}
void DoRead() { DoReadLn(); }
void DoWrite() { LogFatal("Write not imp"); }

void HexDump(char* payload, word size) {
    if (size > 1030) {
      LogFatal("HexDump: too big: %x", size);
    }
    for (word i = 0; i < size; i += 16) {
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
}
//////////////////////////////////////////

int main(int argc, char* argv[]) {
  // We can ignore argc, argv.
  LogStep("Hello Hello Twice");
  Message = strdup("we might be mocked but we'll never stop");

  CheckE(Os9Create, (DAEMON, /*mode=*/READ_WRITE, /*attrs=*/SHARABLE_RW, &Fd));

  while (true) {
    int cc;
    LogInfo("TWICE Reading...");
    CheckE(Os9Read, (Fd, (char*)&Request, sizeof Request, &cc));
    Assert(cc >= sizeof Request.header);

    LogInfo("TWICE Read: op=%x path=%x a=%x b=%x size=%x cc=%x",
        Request.header.operation,
        Request.header.path_num,
        Request.header.a_reg,
        Request.header.b_reg,
        Request.header.size,
        cc);

    /*
    int m = cc - sizeof Request;
    for (word i = 0; i < m; i += 16) {
      Printf("\n%04x: ", i);
      for (word j = 0; j < 16 && (i+j) < cc; j++) {
        Printf("%02x ", Request.payload[i+j]);
        if ((j&3)==3) Printf(" ");
      }
    }
    Printf("\n");
    */
    HexDump(Request.payload, cc - sizeof Request.header);

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
        LogFatal("Bad operation: %d", Request.header.operation);
    }

    LogInfo("TWICE Write: status=%x size=%x n=%x",
        Reply.header.status, Reply.header.size, n);
    HexDump(Reply.payload, n - sizeof Reply.header);
    CheckE(Os9Write, (Fd, (char*)&Reply, n, &cc));
    LogInfo("TWICE Wrote OKAY");
  }

  LogStatus("Finished Finished Twice");
  return 0;
}
