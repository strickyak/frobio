#define MY_COCOIO_PORT  0xFF68
#define MY_ADDR         IP4ADDR(10, 2, 2, 7)
#define MY_MASK         IP4ADDR(255, 255, 255, 0)
#define MY_GATEWAY      IP4ADDR(10, 2, 2, 1)
#define MY_MAC          "wiznet"

#define SERVER_ADDR      IP4ADDR(10, 2, 2, 2)
#define SERVER_PORT      69

#include <cmoc.h>
#include "frobio/frobio.h"
#include "frobio/os9call.h"
#include "frobio/os9defs.h"
#include "frobio/nylib.h"

// Operations defined by TFTP protocol:
#define OP_READ 1
#define OP_WRITE 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

#define SOCK 0  // For now, use socket number 0.

byte packet[2000];

void Reset() {
  wiz_verbose = 1;

  struct FrobioConfig config;
  config.ip_addr = MY_ADDR;
  config.ip_mask = MY_MASK;
  config.ip_gateway = MY_GATEWAY;
  for (byte i = 0; i < 6; i++ )
    config.ether_mac[i] = MY_MAC[i];

  // Reset and configure.  Test ping.
  wiz_reset(MY_COCOIO_PORT);
  wiz_configure(&config);
  wiz_ping(SERVER_ADDR);
  error err = udp_open(SOCK, 0x6789);
  if (err) NyFatalD("cannot udp_open: %d\n", err);
}

void SendAck(word block, word tid) {
  word* wp = (word*)packet;
  wp[0] = OP_ACK;
  wp[1] = block;
  error err = udp_send(SOCK, packet, 4, SERVER_ADDR, tid);
  if (err) NyFatalD("cannot udp_send request: %d\n", err);
}
void SendRequest(word opcode, char* filename, bool ascii) {
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

#if 0
  printf("\n request: ");
  for (byte* q = packet; q < (byte*)p; q++ ) {
    printf("%02x ", *q);
  }
#endif

  error err = udp_send(SOCK, packet, p-(char*)packet, SERVER_ADDR, SERVER_PORT);
  if (err) NyFatalD("cannot udp_send request: %d\n", err);
}

int Get(char* filename) {
  wiz_verbose = 0;
  SendRequest(OP_READ, filename, /*ascii=*/0);

  while (1) {
    word size = sizeof packet;
    quad from_addr = 0;
    word from_port = 0;
    error err = udp_recv(SOCK, packet, &size, &from_addr, &from_port);
    if (err) NyFatalD("cannot udp_recv data: %d\n", err);
    word type = ((word*)packet)[0];
    word arg = ((word*)packet)[1];

    // printf("\n GOT %d BYTES FROM %lx:%x", size, from_addr, from_port);
    printf("G:%d:%d,%d ", size, type, arg);
#if 0
    printf("\n got: {");
    for (int i = 0; i < size; i++) {
      printf("%02x ", packet[i]);
      if ((i&3)==3) printf(" ");
      if (i>63) break;
    }
    printf("}\n");
#endif
    switch (type) {
    case OP_DATA:
        // arg is block number.
        word len = size - 4;
        SendAck(arg, from_port);
        // printf(" [recv block %d len %d] ", arg, len);
        if (len < 512) goto END_LOOP;
        break;
    case OP_ERROR:
        // arg is error number.
      printf(" {%s} ", packet+4);
      NyFatalD("Get() got error %d", arg);
      break;
    default:
      NyFatalD("Get() did not expect to recv type %d", type);
    }
  }  // while(1)
END_LOOP:
  printf("Get Finished.  ");
  return 0;
}

int main(int argc, char* argv[]) {
  Reset();

    printf("argc = %d\n", argc);
    for (byte i = 0; i < argc; i++) {
      printf("argv [%d] {%s}\n", i, argv[i]);
    }

    if (argc<3) {
      printf("tftp: wants two args\n");
    } else if (NyStrEqIgnoreCase(argv[1], "get")) {
      return Get(argv[2]);
    } else {
      printf("tftp: unknown command\n");
    }
    return 255;
}
