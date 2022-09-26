// ... | f-send peeraddr:port

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

int bytes_read;
byte buffer[2000];

byte OpenLocalSocket(word localport) {
  byte socknum = 0;
  if (!localport) {
    localport = suggest_client_port();
  }
  errnum err = udp_open(localport, &socknum);
  if (err) LogFatal("cannot udp_open: %d", err);
  return socknum;
}

void SendRequest(byte socknum, quad host, word port) {
  errnum err = udp_send(socknum, buffer, bytes_read, host, port);
  if (err) LogFatal("cannot udp_send request: %d", err);
}

void MainLoop(byte socknum, quad peer_host, word peer_port) {
  errnum err = Os9ReadLn(0/*stdin*/, (char*)buffer, sizeof buffer, &bytes_read);
  if (err) LogFatal("cannot ReadLn stdin: %d", err);

  SendRequest(socknum, peer_host, peer_port);
}

static void FatalUsage() {
    LogFatal("Usage:  ... | f-send -w0xFF68 -pLocalPort peeraddr:port\n");
}

int main(int argc, char* argv[]) {
  word localport = 0;
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "p:v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'p':
        localport = NyParseHexWord(&FlagArg);
        break;
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      case 'w':
         wiz_hwport = (byte*)prefixed_atoi(FlagArg);
         break;
      default:
        FatalUsage();
    }
  }
  if (argc != 1) {
    FatalUsage();
  }

  const char* parse = argv[0];
  word peer_port = 0;
  quad peer_addy = NyParseDottedDecimalQuadAndPort(&parse, &peer_port); 
  if (!peer_port) {
    LogFatal("ERROR: Peer port required.");
  }
  byte socknum = OpenLocalSocket(localport);
  MainLoop(socknum, peer_addy, peer_port);

  return OKAY;
}
