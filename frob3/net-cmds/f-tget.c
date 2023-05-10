// f-tget server_addr:69 remote-file local-file

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

#define DEFAULT_SERVER_PORT 69 /* TFTPD */

// Operations defined by TFTP protocol:
#define OP_READ 1
#define OP_WRITE 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

byte packet[666];

byte OpenLocalSocket() {
  byte socknum = 0;
  word client_port = suggest_client_port();
  prob ps = UdpOpen(client_port, &socknum);
  if (ps) LogFatal("cannot UdpOpen: %s", ps);
  return socknum;
}

void SendAck(byte socknum, word block, quad addr, word tid) {
  word* wp = (word*)packet;
  wp[0] = OP_ACK;
  wp[1] = block;
  prob ps = UdpSend(socknum, packet, 4, addr, tid);
  if (ps) LogFatal("cannot UdpSend ack: %s", ps);
}

void SendRequest(byte socknum, quad host, word port, word opcode, const char* filename, bool ascii) {
  char* p = (char*)packet;
  *(word*)p = opcode;
  p += 2;

  int n = strlen(filename);
  strcpy(p, filename);
  p += n+1;

  const char* mode = ascii ? "netascii" : "octet";
  n = strlen(mode);
  strcpy(p, mode);
  p += n+1;

  prob ps = UdpSend(socknum, packet, p-(char*)packet, host, port);
  if (ps) LogFatal("cannot UdpSend request: %s", ps);
}

void TGet(byte socknum, quad server_host, word server_port, const char* filename, const char* localfile) {
  errnum err;
  prob ps;
  int fd = -1;
  err = Os9Create(localfile, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
  if (err) LogFatal("Cannot create %q: %s", localfile, err);

  SendRequest(socknum, server_host, server_port, OP_READ, filename, /*ascii=*/0);

  word expected_block = 1;
  while (1) {
    word size = sizeof packet;
    quad from_addr = 0;
    word from_port = 0;
    ps = UdpRecv(socknum, packet, &size, &from_addr, &from_port);
    if (ps) LogFatal("cannot UdpRecv: %s", ps);
    word type = ((word*)packet)[0];
    word arg = ((word*)packet)[1];

    LogDetail("G:%u:%u,%u ", size, type, arg);

    switch (type) {
    case OP_DATA:
        if (arg != expected_block) {
            // Don't ack the wrong block.
            break;
        }
        word len = size - 4;
        SendAck(socknum, arg, from_addr, from_port);
        // TODO -- send more Acks for last block, if we don't get the next one.
        int bytes_written = 0;
        err = Os9Write(fd, (char*)packet+4, len, &bytes_written);
        if (err) {
            LogFatal("ERROR, Cannot write file: errnum %d", err);
        }
        if (bytes_written != len) {
            LogFatal("ERROR, short write: errnum %d", err);
        }
        if (len < 512) goto END_LOOP;
        ++expected_block;
        break;
    case OP_ERROR:
        // arg is error number.
      LogFatal("*** Server error %d: %s", arg, packet+4);
      break;
    default:
      LogFatal("TGet() did not expect to recv type %d", type);
    }
  }  // while(1)
END_LOOP:
  err = Os9Close(fd);
  if (err) {
    LogFatal("ERROR, Cannot close written file: errnum %d", err);
  }
  ps = UdpClose(socknum);
  if (ps) {
    LogFatal("ERROR, Cannot close UDP socket: %s", ps);
  }
  LogStatus("OKAY: wrote %q", localfile);
}

static void FatalUsage() {
    LogFatal("Usage:  f-tget -w0xFF68 server:69 remote-file local-file");
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
  const char* p = argv[0];
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&p, &server_port); 
  byte socknum = OpenLocalSocket();
  TGet(socknum, server_addy, server_port, argv[1], argv[2]);
  return 0;
}
