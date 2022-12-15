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
  while (GetFlag(&argc, &argv, "p:v:w:")) {
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
  if (argc) FatalUsage();
  if (!port) FatalUsage();

  byte socknum = 255;
  prob err = tcp_open(&socknum);
  if (err) LogFatal("Cannot open TCP socket: %q", err);
  LogStep("opened");

  err = tcp_listen(socknum, port);
  if (err) LogFatal("Cannot listen on TCP server port %d: %q", port, err);
  LogStep("listened");

  err = tcp_establish_blocking(socknum);
  if (err) LogFatal("Cannot accept on TCP server port %d: %q", port, err);
  LogStep("accepted");

  const char* banner = "(WELCOME)\r\n";
  err = tcp_send_blocking(socknum, banner, strlen(banner));
  if (err) LogFatal("Cannot send banner: %q", err);
  LogStep("welcomed");

  while (true) {
    size_t cc = 0;
    memset(buf, 0, sizeof buf);
    err = tcp_recv_blocking(socknum, buf, sizeof buf - 1, &cc);
    if (err) {
        LogInfo("... recv->%q", err);
        continue;
    }
    LogStep("recv cc=%x: %q", cc, buf);

    if (cc) {
      err = tcp_send_blocking(socknum, buf, cc);
      if (err) LogFatal("Cannot tcp_send: %q", err);
      LogStep("send: %q", buf);
    }

    if (buf[0]=='q' || buf[0]=='Q') break;
  }

  banner = "(BYE)\r\n";
  err = tcp_send_blocking(socknum, banner, strlen(banner));
  if (err) LogFatal("Cannot send (BYE): %q", err);
  LogStep("bye");

  err = tcp_close(socknum);
  if (err) LogFatal("Cannot tcp_close: %q", err);
  LogStep("closed");
  LogStatus("Excellent");

  return 0;
}
