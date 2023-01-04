// f-ntp server-addr:123

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/os9/os9defs.h" // E_EOF

int fd;
char command[99];
char status[99];
char payload[999];
char DaemonPath[] = "/Fuse/Daemon/Twice";

#define READ_WRITE 03
#define SHARABLE_RW 0133

//////////////////////////////////////////

byte DeHexChar(char ch) {
  if ('0' <= ch && ch <= '9') {
    return ch - '0';
  }
  if ('A' <= ch && ch <= 'Z') {
    return ch - 'A' + 10;
  }
  if ('a' <= ch && ch <= 'z') {
    return ch - 'a' + 10;
  }
  LogFatal("DeHex %d", ch);
}

//////////////////////////////////////////

void SendStatus(errnum status, word len) {
  Buf buf;
  BufInit(&buf);
  BufFormat(&buf, "%02x %04x\r", status, len);
  BufFinish(&buf);

  int cc;
  CheckE(Os9WritLn, (fd, buf.s, buf.n, &cc));
  Assert(cc == buf.n);

  BufDel(&buf);
}

void DoOpen() {
  SendStatus(OKAY, 0);
}
int nth = 1;
void DoRead() {
  if (nth > 3) {
    LogInfo("Sending Status EOF");
    SendStatus(E_EOF, 0);
    LogInfo("Sent Status EOF");
  } else {
    LogInfo("Sending Status OKAY");
    SendStatus(OKAY, 5);
    LogInfo("Sent Status OKAY");
  }

  int cc;
  switch (nth) {
    case 1:
      LogInfo("calling Write uno");
      CheckE(Os9Write, (fd, "uno.\r", 5, &cc));
      LogInfo("called Write uno");
      break;
    case 2:
      LogInfo("calling Write dos");
      CheckE(Os9Write, (fd, "dos.\r", 5, &cc));
      LogInfo("called Write dos");
      break;
    case 3:
      LogInfo("calling Write tres");
      CheckE(Os9Write, (fd, "tres\r", 5, &cc));
      LogInfo("called Write tres");
      break;
    default:
      Assert(0);
  }
  Assert(cc == 5);
  ++nth;
}
void DoReadLn() {
  DoRead();
}
void DoClose() {
  SendStatus(OKAY, 0);
}

//////////////////////////////////////////

int main(int argc, char* argv[]) {
  // We can ignore argc, argv.
  LogStep("Hello Hello Twice");

  CheckE(Os9Create, (DaemonPath, /*mode=*/READ_WRITE, /*attrs=*/SHARABLE_RW, &fd));

  while (true) {
    int cc;
    CheckE(Os9ReadLn, (fd, command, sizeof command, &cc));
    LogInfo("ReadLn: %q", command);

    if (!payload[0]) {
      // Remember the first command, twice, as our payload.
      strcpy(payload, command);
      strcat(payload, command);

      switch (command[0]) {
        case 'c':
        case 'o':
          DoOpen(); break;
        case 'r':
          DoRead(); break;
        case 'R':
          DoReadLn(); break;
        case 'C':
          DoClose(); break;
        default:
          LogFatal("Unhandled command: %q", command);
      }
    }
  }

  LogStatus("Finished Finished Twice");
  return 0;
}
