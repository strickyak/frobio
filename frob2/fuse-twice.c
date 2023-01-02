// f-ntp server-addr:123

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

int fd;
char command[99];
char status[99];
char payload[999];
char DaemonPath[] = "/Fuse/Daemon/Twice";

#define READ_WRITE 03
#define SHARABLE_RW 0133

void DoOpen() {
  LogFatal("not yet DoOpen");
}
void DoRead() {
  LogFatal("not yet DoRead");
}
void DoReadLn() {
  LogFatal("not yet DoReadLn");
}
void DoClose() {
  LogFatal("not yet DoClose");
}

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
