// ... | f.send peeraddr:port

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

bool show_from;
byte packet[2000];
char buffer[2000];

byte OpenLocalSocket(word localport) {
  byte socknum = 0;
  error err = udp_open(localport, &socknum);
  if (err) NyFatalD("cannot udp_open: %d\n", err);
  return socknum;
}

void MainLoop(byte socknum) {
  while (true) {
      quad from_addr = 0;
      word from_port = 0;
      word size = sizeof packet;
      error err = udp_recv(socknum, packet, &size, &from_addr, &from_port);
      if (err) NyFatalD("cannot udp_recv data: %d\n", err);

      int n = 0;
      if (show_from) {
        byte* p = (byte*)&from_addr;
        sprintf(buffer, "%d.%d.%d.%d:%d ", p[0], p[1], p[2], p[3], from_port);
        n = strlen(buffer);
      }

      memcpy(buffer+n, (char*)packet, size);
      int bytes_written = 0;
      err = Os9WritLn(1/*=stdout*/, buffer, n+size, &bytes_written);
      if (err) NyFatalD("cannot writln to stdout: err %d", err);
      if (bytes_written != n+size) {
        NyFatalD("short writln to stdout: short by %d",
           n+size - bytes_written);
      }
  }  // while(true)
}

static void UsageAndExit() {
    printf("Usage:  f.recv -wWiznetPortHex -pLocalPort | ...\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  word localport = 0;
  while (argc && argv[0][0]=='-') {
    p = argv[0]+2;  // In case needed for parsing.
    switch (argv[0][1]) {
    case 'f':
      show_from = true;
      break;
    case 'p':
      localport = NyParseDecimalWord(&p);
      break;
    case 'v':
      wiz_verbose = true;
      break;
    case 'w':
      wiz_hwport = (byte*)NyParseHexWord(&p);
      break;
    default:
      UsageAndExit();
    }
    argc--, argv++;  // Discard consumed flag.
  }

  if (argc != 0) {
    UsageAndExit();
  }
  if (!localport) {
    printf("ERROR: local port is 0\n");
    UsageAndExit();
  }
  byte socknum = OpenLocalSocket(localport);
  MainLoop(socknum);

  return OKAY;
}
