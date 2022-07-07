// ... | f.send peeraddr:port

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

int bytes_read;
byte buffer[2000];

byte OpenLocalSocket(word localport) {
  byte socknum = 0;
  if (!localport) {
    localport = suggest_client_port();
  }
  error err = udp_open(localport, &socknum);
  if (err) NyFatalD("cannot udp_open: %d\n", err);
  return socknum;
}

void SendRequest(byte socknum, quad host, word port) {
  error err = udp_send(socknum, buffer, bytes_read, host, port);
  if (err) NyFatalD("cannot udp_send request: %d\n", err);
}

void MainLoop(byte socknum, quad peer_host, word peer_port) {
  error err = Os9ReadLn(0/*stdin*/, (char*)buffer, sizeof buffer, &bytes_read);
  if (err) NyFatalD("cannot ReadLn stdin: %d\n", err);

  SendRequest(socknum, peer_host, peer_port);
}

static void UsageAndExit() {
    printf("Usage:  ... | f.send -wWiznetPortHex peeraddr:port\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  word localport = 0;
  while (argc && argv[0][0]=='-') {
    p = argv[0]+2;  // In case needed for parsing.
    switch (argv[0][1]) {
    case 'p':
      localport = NyParseHexWord(&p);
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

  if (argc != 1) {
    UsageAndExit();
  }
  p = argv[0];
  word peer_port = 0;
  quad peer_addy = NyParseDottedDecimalQuadAndPort(&p, &peer_port); 
  if (!peer_port) {
    printf("ERROR: Peer port required.\n");
    UsageAndExit();
  }
  byte socknum = OpenLocalSocket(localport);
  MainLoop(socknum, peer_addy, peer_port);

  return OKAY;
}
