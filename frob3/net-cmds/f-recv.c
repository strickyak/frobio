// ... | f.send peeraddr:port

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

bool show_from;
byte packet[600];
char buffer[600];

byte OpenLocalSocket(word localport) {
  byte socknum = 0;
  prob ps = UdpOpen(localport, &socknum);
  if (ps) LogFatal("cannot UdpOpen: %s", ps);
  return socknum;
}

void MainLoop(byte socknum) {
  while (true) {
      quad from_addr = 0;
      word from_port = 0;
      word size = sizeof packet;
      prob ps = UdpRecv(socknum, packet, &size, &from_addr, &from_port);
      if (ps) LogFatal("cannot UdpRecv: %s", ps);

      int n = 0;
      if (show_from) {
        byte* p = (byte*)&from_addr;
        char* s = StrFormat("%d.%d.%d.%d:%d ", p[0], p[1], p[2], p[3], from_port);
        strcpy(buffer, s);
        Free(s);
        n = strlen(buffer);
      }

      memcpy(buffer+n, (char*)packet, size);
      int bytes_written = 0;
      errnum err = Os9WritLn(1/*=stdout*/, buffer, n+size, &bytes_written);
      if (err) LogFatal("cannot writln to stdout: err %d", err);
      if (bytes_written != n+size) {
        LogFatal("short writln to stdout: short by %d",
           n+size - bytes_written);
      }
  }  // while(true)
}

static void FatalUsage() {
    LogFatal("Usage:  f-recv -wWiznetPortHex -pLocalPort | ...");
}

int main(int argc, char* argv[]) {
  word localport = 0;
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "fp:v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'f':
        show_from = true;
        break;
      case 'p':
        localport = NyParseDecimalWord(&FlagArg);
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
  if (argc != 0) {
    FatalUsage();
  }

  if (!localport) {
    LogError("ERROR: local port is 0");
    FatalUsage();
  }
  byte socknum = OpenLocalSocket(localport);
  MainLoop(socknum);

  return OKAY;
}
