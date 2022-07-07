// f.tget server_addr:69 remote-file local-file

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

#define DEFAULT_SERVER_PORT 69 /* TFTPD */

// Operations defined by TFTP protocol:
#define OP_READ 1
#define OP_WRITE 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

byte packet[2000];

byte OpenLocalSocket() {
  byte socknum = 0;
  word client_port = suggest_client_port();
  error err = udp_open(client_port, &socknum);
  if (err) NyFatalD("cannot udp_open: %d\n", err);
  return socknum;
}

void SendAck(byte socknum, word block, quad addr, word tid) {
  word* wp = (word*)packet;
  wp[0] = OP_ACK;
  wp[1] = block;
  error err = udp_send(socknum, packet, 4, addr, tid);
  if (err) NyFatalD("cannot udp_send ack: %d\n", err);
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

  error err = udp_send(socknum, packet, p-(char*)packet, host, port);
  if (err) NyFatalD("cannot udp_send request: %d\n", err);
}

int TGet(byte socknum, quad server_host, word server_port, const char* filename, const char* localfile) {
  int fd = -1;
  error err = Os9Create(localfile, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);

  SendRequest(socknum, server_host, server_port, OP_READ, filename, /*ascii=*/0);

  word expected_block = 1;
  while (1) {
    word size = sizeof packet;
    quad from_addr = 0;
    word from_port = 0;
    err = udp_recv(socknum, packet, &size, &from_addr, &from_port);
    if (err) NyFatalD("cannot udp_recv data: %d\n", err);
    word type = ((word*)packet)[0];
    word arg = ((word*)packet)[1];

    printf("G:%d:%d,%d ", size, type, arg);

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
            NyFatalD("ERROR, Cannot write file: error %d", err);
        }
        if (bytes_written != len) {
            NyFatalD("ERROR, short write: error %d", err);
        }
        if (len < 512) goto END_LOOP;
        ++expected_block;
        break;
    case OP_ERROR:
        // arg is error number.
      printf("*** SERVER ERROR %d: %s\n", arg, packet+4);
      NyFatalD("TGet() got error %d", arg);
      break;
    default:
      NyFatalD("TGet() did not expect to recv type %d", type);
    }
  }  // while(1)
END_LOOP:
  err = Os9Close(fd);
  if (err) {
    NyFatalD("ERROR, Cannot close written file: error %d", err);
  }
  err = udp_close(socknum);
  if (err) {
    NyFatalD("ERROR, Cannot close UDP socket: error %d", err);
  }
  printf("TGet Finished.  ");
  return 0;
}

static void UsageAndExit() {
    printf("Usage:  f.tget -wWiznetPortHex server:69 remote-file local-file\n");
    exit(1);
}

int main(int argc, char* argv[]) {
  const char* p;   // for parsing.
  argc--, argv++;  // Discard argv[0], unused on OS-9.

  while (argc && argv[0][0]=='-') {
    p = argv[0]+2;  // In case needed for parsing.
    switch (argv[0][1]) {
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

  if (argc != 3) {
    UsageAndExit();
  }
  p = argv[0];
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&p, &server_port); 
  byte socknum = OpenLocalSocket();
  TGet(socknum, server_addy, server_port, argv[1], argv[2]);

  return OKAY;
}
