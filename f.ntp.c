// f.ntp server-addr:123

#include <cmoc.h>
#include <assert.h>
#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

#define DEFAULT_SERVER_PORT 123 /* NTPD */

struct ntp_packet {
  byte li_vn_mode;
  byte stratum;
  byte poll;
  byte precision;
  quad root_delay;
  quad root_dispersion;
  quad reference_id;
  quad ref_ts;
  quad ref_ts_frac;
  quad orig_ts;
  quad orig_ts_frac;
  quad recv_ts;
  quad recv_ts_frac;
  quad xmit_ts;
  quad xmit_ts_frac;
};

#define CLIENT_LI_VN_MODE 0xE3
#define SERVER_LI_VN_MODE 0x04
#define SERVER_LI_VN_MASK 0x07

byte packet[2000];

byte OpenLocalSocket() {
  byte socknum = 0;
  word client_port = suggest_client_port();
  error err = udp_open(client_port, &socknum);
  if (err) NyFatalD("cannot udp_open: %d\n", err);
  return socknum;
}

void SendRequest(byte socknum, quad host, word port) {
  struct ntp_packet x;
  NyZero(&x, sizeof x);
  x.li_vn_mode = CLIENT_LI_VN_MODE;

  error err = udp_send(socknum, (byte*)&x, sizeof x, host, port);
  if (err) NyFatalD("cannot udp_send request: %d\n", err);
}

void SNTP(byte socknum, quad server_host, word server_port) {
  SendRequest(socknum, server_host, server_port);

  word size = sizeof packet;
  quad from_addr = 0;
  word from_port = 0;
  error err = udp_recv(socknum, packet, &size, &from_addr, &from_port);
  if (err) NyFatalD("cannot udp_recv data: %d\n", err);
  
  if (size < sizeof (struct ntp_packet)) {
    NyFatalD("received size too small", size);
  }

  err = udp_close(socknum);
  if (err) {
    NyFatalD("ERROR, Cannot close UDP socket: error %d", err);
  }
  struct ntp_packet *a = (struct ntp_packet*)packet;
  printf("NTP: %lu %lu %lu %lu\n", a->ref_ts, a->orig_ts, a->recv_ts, a->xmit_ts);
}

static void UsageAndExit() {
    printf("Usage:  f.ntp -wWiznetPortHex server_addr:123\n");
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

  if (argc != 1) {
    UsageAndExit();
  }
  p = argv[0];
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&p, &server_port); 
  byte socknum = OpenLocalSocket();
  SNTP(socknum, server_addy, server_port);

  return OKAY;
}
