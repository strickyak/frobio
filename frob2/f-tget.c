// f-tget server_addr:69 remote-file local-file

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

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
  errnum err = udp_open(client_port, &socknum);
  if (err) LogFatal("cannot udp_open: %d", err);
  return socknum;
}

void SendAck(byte socknum, word block, quad addr, word tid) {
  word* wp = (word*)packet;
  wp[0] = OP_ACK;
  wp[1] = block;
  errnum err = udp_send(socknum, packet, 4, addr, tid);
  if (err) LogFatal("cannot udp_send ack: errnum %d", err);
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

  errnum err = udp_send(socknum, packet, p-(char*)packet, host, port);
  if (err) LogFatal("cannot udp_send request: errnum %d", err);
}

errnum TGet(byte socknum, quad server_host, word server_port, const char* filename, const char* localfile) {
  int fd = -1;
  errnum err = Os9Create(localfile, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
  if (err) LogFatal("Cannot create %q: errnum %d", localfile, err);

  SendRequest(socknum, server_host, server_port, OP_READ, filename, /*ascii=*/0);

  word expected_block = 1;
  while (1) {
    word size = sizeof packet;
    quad from_addr = 0;
    word from_port = 0;
    err = udp_recv(socknum, packet, &size, &from_addr, &from_port);
    if (err) LogFatal("cannot udp_recv data: errnum %d", err);
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
  err = udp_close(socknum);
  if (err) {
    LogFatal("ERROR, Cannot close UDP socket: errnum %d", err);
  }
  LogStatus("OKAY: wrote %q", localfile);
  return 0;
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
         wiz_hwport = (byte*)prefixed_atoi(FlagArg);
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
  return TGet(socknum, server_addy, server_port, argv[1], argv[2]);
}
