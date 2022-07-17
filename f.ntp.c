// f.ntp server-addr:123

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

#define CLIENT_LI_VN_MODE 0xE3 // LI(2bit)=3 VN(3bit)=4 MODE(3bit)=3 client.
#define SERVER_LI_VN_MODE 0x04 // MODE(3bit)=4 server.
#define SERVER_LI_VN_MASK 0x07 // Just the MODE(3bit) bits.

// Start the array with unused month 0.
byte dpm[13] = {255, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
bool set_time;

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

  // f.ntp 10.2.2.2
  // NTP: 3866319812 516934400 3866319934 3866319934

  // >>> 3866319812 / 86400 / 365.25
  // 122.5162817197759  ( approx Fri Jul  8 18:4x PDT 2022 )

  // Since this code is written in 2022, we will never have to
  // deal with years before 2020.  We can make this last until
  // 2099, if the seconds last that long, without having to 
  // worry about y%100 and y%400 leap year exceptions.

  // There are 3786825600 seconds from 1 Jan 1900 to 1 Jan 2020,
  // so rapidly skip to 1 Jan 2020 by subtracting that.
  // The "3036 bug" should not be a bug in this code, because
  // quad is an unsigned 4-byte number, and it wraps around.
  quad t0 = a->xmit_ts - 3786825600UL;
  quad t = t0;  // Detect underflow with t0.
  quad prev_t;
  word year = 2020;
  while (true) {
    quad t_next = t;
    t_next -= 31536000UL;  // 365 days.
    if ((year%3) == 0) t_next -= 86400UL;  // 1 extra day.
    if (t_next > t0) break;  // if underflowed.
    t = t_next;
    year++;
  }

  byte month;
  for (month=1; month<=12; month++) {
    byte days = dpm[month];
    if (month==2 && (year%3)==0) ++days;  // If Feb hath 29.
    quad t_next = t - 86400UL * days;
    if (t_next > t0) break;  // if underflowed.
    t = t_next;
  }
  assert(month < 13);

  byte day = (byte)1 + (byte)(t / 86400UL);
  t = t % 86400UL;

  byte hour = (byte)(t / 3600UL);
  t = t % 3600UL;

  byte min = (byte)(t / 60UL);
  t = t % 60UL;

  assert(t < 60);
  byte sec = (byte)t;

  printf("y=%d m=%d d=%d %02d:%02d:%02d UTC\n", year, month, day, hour, min, sec);

  if (set_time) {
      byte time_pack[6];
      time_pack[0] = (byte)(year - 1900);
      time_pack[1] = month;
      time_pack[2] = day;
      time_pack[3] = hour;
      time_pack[4] = min;
      time_pack[5] = sec;
      asm {
        leax time_pack
        swi2     ; OS9 ...
        fcb $16  ; ... F$STime
      }
  }
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
    case 's':
      set_time = true;
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
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&p, &server_port); 
  byte socknum = OpenLocalSocket();
  SNTP(socknum, server_addy, server_port);

  return OKAY;
}
