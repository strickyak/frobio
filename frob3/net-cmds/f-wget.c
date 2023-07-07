// f-tget server_addr:69 remote-file local-file

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

#define DEFAULT_SERVER_PORT 80 /* HTTP */

char packet[666];

byte HttpOpen(quad host, word port) {
  byte socknum = 0;
  word client_port = suggest_client_port();
  prob ps = TcpOpen(&socknum);
  if (ps) LogFatal("cannot TcpOpen: %s", ps);
  FPuts("+", StdErr);

  ps = TcpDial(socknum, host, port);
  if (ps) LogFatal("cannot TcpDial: %s", ps);
  FPuts("+", StdErr);

  while (1) {
    ps = TcpEstablishOrNotYet(socknum);
    if (ps == NotYet) {
      FPuts("-", StdErr);
      Os9Sleep(1);
    } else {
      break;
    }
  }
  FPuts(">", StdErr);

  return socknum;
}

void SendStr(byte socknum, char* bb, word bblen) {
  while (1) {
  prob ps = TcpSendOrNotYet(socknum, bb, bblen);
  if (ps == NotYet) {
    FPuts("=", StdErr);
    Os9Sleep(1);
    continue;
  }
  if (ps) LogFatal("cannot TcpSend");
  FPuts(">", StdErr);
  return;
  }
}

const char RequestTemplate1[] =
    "GET %s HTTP/1.0\r\n"
    "Host: %d.%d.%d.%d:%d\r\n"
    "\r\n";
const char RequestTemplate2[] =
    "GET /%s HTTP/1.0\r\n"
    "Host: %d.%d.%d.%d:%d\r\n"
    "\r\n";

void SendRequest(byte socknum, quad host, word port, const char* urlpath) {
  const char* tpl = (urlpath[0]=='/') ? RequestTemplate1 : RequestTemplate2;
  byte* hp = (byte*)&host;
  SPrintf(packet, tpl, urlpath, hp[0], hp[1], hp[2], hp[3], port);
  SendStr(socknum, packet, strlen(packet));
}

void WGet(byte socknum, quad server_host, word server_port, const char* urlpath, const char* localfile) {
  errnum err;
  prob ps;
  int fd = -1;
  err = Os9Create(localfile, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
  if (err) LogFatal("Cannot create %q: %s", localfile, err);

  SendRequest(socknum, server_host, server_port, urlpath);

  word expected_block = 1;
  quad total_bytes = 0;
  while (1) {
    memset(packet, 0, sizeof packet);
    word cc;
    ps = TcpRecvOrNotYet(socknum, packet, sizeof packet, &cc);

    total_bytes += cc;
    LogInfo("total_bytes %04x%04x ; cc=%x", (word)(total_bytes>>16), (word)(total_bytes), cc);
    // Assume the world is faster than the coco.
    // TODO -- find the Closed bit.
    // TODO -- make it work in Gomar!
    if (total_bytes > 0 && cc == 0) break; // TODO Maybe too early!

    if (ps == NotYet) {
      FPuts(".", StdErr);
      Os9Sleep(1);
      continue;
    }
    if (ps) LogFatal("cannot TcpRecv: %s", ps);

    if (cc==0) break;
    if (packet[0]==0) break;

    FPuts(":", StdErr);
    int bytes_written = 0;
    err = Os9Write(fd, packet, cc, &bytes_written);
    if (err) {
      LogFatal("ERROR, Cannot write file: errnum %d", err);
    }
    if (bytes_written != cc) {
      LogFatal("ERROR, short write: errnum %d", err);
    }

    // Stop after one packet.
    FPuts(" LOOP\r", StdErr);
  }
  FPuts("\r", StdErr);

  err = Os9Close(fd);
  if (err) {
    LogFatal("ERROR, Cannot close written file: errnum %d", err);
  }
  ps = TcpClose(socknum);
  if (ps) {
    LogFatal("ERROR, Cannot close TCP socket: %s", ps);
  }
  LogStatus("OKAY: wrote %q", localfile);
}

static void FatalUsage() {
    LogFatal("Usage:  f-wget -w0xFF68 server:80 remote-file local-file");
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
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

  if (argc != 3) {
    FatalUsage();
  }
  const char* p = /*hostport*/ argv[0];
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&p, &server_port); 
  byte socknum = HttpOpen(server_addy, server_port);
  WGet(socknum, server_addy, server_port, /*urlpath*/ argv[1], /*out filename*/argv[2]);
  return 0;
}
