// demo fuse daemon "Twice"

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/os9/os9defs.h" // E_EOF
#include "../../doing_os9/fuse1/fuse2.h"

#define DAEMON "/Fuse/Daemon/Twice"
#define PAYSIZ 768 // Three 256-byte blocks.
#define READ_WRITE 03
#define SHARABLE_RW 0133

struct Req {
  struct Fuse2Request header;
  char payload[PAYSIZ];
} Request;

struct Rep {
  struct Fuse2Reply header;
  char payload[PAYSIZ];
} Reply;

int Fd;
int count_lines;

//////////////////////////////////////////

void DoOpen() {
  Reply.header.status = 0;
  Reply.header.size = 0;
  LogInfo("FUSE Daemon does Open for client.");
  count_lines = 0;
}
void DoClose() {
  Reply.header.status = 0;
  Reply.header.size = 0;
  LogInfo("FUSE Daemon does Close for client.");
}

void DoReadLn() {
  Reply.header.status = 0;
  Reply.header.size = 6;

  int option = count_lines & 3;
  LogInfo("DoReadLn: count_lines=%x option=%x", count_lines, option);

  if (count_lines > 16) {
    Reply.header.status = E_EOF;
    Reply.header.size = 0;
    LogInfo("FUSE Daemon returns E_EOF to client.");

    /* This version tries returning unix-style 0 bytes
     * instead of error E_EOF to signal EOF.
     * But that is not the OS9 convention.
  } else if (count_lines > 16) {
    Reply.header.status = OKAY;
    Reply.header.size = 0;
    LogInfo("FUSE Daemon returns 0 bytes to client.");
    */

  //////} else switch (count_lines&3) {
  } else switch (option) {
    case 0: strcpy(Reply.payload, "Nando\r"); break;
    case 1: strcpy(Reply.payload, "Bilbo\r"); break;
    case 2: strcpy(Reply.payload, "Frodo\r"); break;
    case 3: strcpy(Reply.payload, ".....\r"); break;
  }
  ++count_lines;
}

void DoWritLn() { LogFatal("WritLn not imp"); }
void DoRead() { DoReadLn(); }
void DoWrite() { LogFatal("Write not imp"); }

//////////////////////////////////////////

int main(int argc, char* argv[]) {
  // We can ignore argc, argv.
  LogStep("Hello Hello Twice");

  CheckE(Os9Create, (DAEMON, /*mode=*/READ_WRITE, /*attrs=*/SHARABLE_RW, &Fd));

  while (true) {
    int cc;
    LogInfo("DAEMON Reading...");
    CheckE(Os9Read, (Fd, (char*)&Request, sizeof Request, &cc));
    Assert(cc >= sizeof Request.header);

    LogInfo("DAEMON Read: op=%x path=%x a=%x b=%x size=%x cc=%x",
        Request.header.operation,
        Request.header.path_num,
        Request.header.a_reg,
        Request.header.b_reg,
        Request.header.size,
        cc);

    switch (Request.header.operation) {
      case OP_CREATE:
      case OP_OPEN: DoOpen(); break;
      case OP_CLOSE: DoClose(); break;
      case OP_READ: DoRead(); break;
      case OP_WRITE: DoWrite(); break;
      case OP_READLN: DoReadLn(); break;
      case OP_WRITLN: DoWritLn(); break;
      default:
        LogFatal("Bad operation: %d", Request.header.operation);
    }

    int n = Reply.header.size + sizeof Reply.header;
    LogInfo("DAEMON Write: status=%x size=%x n=%x",
        Reply.header.status, Reply.header.size, n);
    CheckE(Os9Write, (Fd, (char*)&Reply, n, &cc));
    LogInfo("DAEMON Wrote OKAY");
  }

  LogStatus("Finished Finished Twice");
  return 0;
}
