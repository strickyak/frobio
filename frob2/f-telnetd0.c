// f-ntp server-addr:123

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

char buf[130];

static void FatalUsage() {
    LogFatal("Usage:  f.telnet0 -w0xFF68 -p23");
}

int main(int argc, char* argv[]) {
  word port = 23; // default telnet port.
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "sv:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
    case 'p':
      port = (word)prefixed_atoi(FlagArg);
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
  if (argc != 1) FatalUsage();
  if (!port) FatalUsage();

  byte socknum = 255;
  prob err = tcp_open_server(port, &socknum);
  if (err) LogFatal("Cannot open TCP server port %d: %s", port, err);

  err = tcp_accept(socknum);
  if (err) LogFatal("Cannot accept TCP connection: %s", err);

  const char* banner = "(WELCOME)\r\n";
  err = tcp_send(socknum, banner, strlen(banner));
  if (err) LogFatal("Cannot send banner: %s", err);

  while (true) {
    int cc = 0;
    err = tcp_recv(socknum, buf, sizeof buf, &cc);
    if (err) LogFatal("Cannot tcp_recv: %s", err);

    if (cc) {
      err = tcp_send(socknum, buf, cc);
      if (err) LogFatal("Cannot tcp_send: %s", err);
    }

    if (buf[0]=='q' || buf[0]=='Q') break;
  }

  banner = "(BYE)\r\n";
  err = tcp_send(socknum, banner, strlen(banner));
  if (err) LogFatal("Cannot send (BYE): %s", err);

  err = tcp_close(socknum);
  if (err) LogFatal("Cannot tcp_close: %s", err);

  return 0;
}
