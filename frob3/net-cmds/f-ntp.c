// f-ntp server-addr:123

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

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
  prob ps = UdpOpen(client_port, &socknum);
  if (ps) LogFatal("cannot UdpOpen: %s", ps);
  return socknum;
}

void SendRequest(byte socknum, quad host, word port) {
  struct ntp_packet x;
  memset(&x, 0, sizeof x);
  x.li_vn_mode = CLIENT_LI_VN_MODE;

  prob ps = UdpSend(socknum, (byte*)&x, sizeof x, host, port);
  if (ps) LogFatal("cannot UdpSend request: %s", ps);
}

void SNTP(byte socknum, quad server_host, word server_port) {
  SendRequest(socknum, server_host, server_port);

  word size = sizeof packet;
  quad from_addr = 0;
  word from_port = 0;
  prob ps = UdpRecv(socknum, packet, &size, &from_addr, &from_port);
  if (ps) LogFatal("cannot UdpRecv data: %s", ps);
  
  if (size < sizeof (struct ntp_packet)) {
    LogFatal("received size too small: %d", size);
  }

  ps = UdpClose(socknum);
  if (ps) {
    LogFatal("Cannot close UDP socket: %s", ps);
  }
  struct ntp_packet *a = (struct ntp_packet*)packet;
  LogDetail("NTP: %lu %lu %lu %lu", a->ref_ts, a->orig_ts, a->recv_ts, a->xmit_ts);

  // f-ntp 10.2.2.2
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
    if ((year&3) == 0) t_next -= 86400UL;  // 1 extra day.
    if (t_next > t0) break;  // if underflowed.
    t = t_next;
    year++;
  }

  byte month;
  for (month=1; month<=12; month++) {
    byte days = dpm[month];
    if (month==2 && (year&3)==0) ++days;  // If Feb hath 29.
    quad t_next = t - 86400UL * days;
    if (t_next > t0) break;  // if underflowed.
    t = t_next;
  }
  Assert(month < 13);

  byte day = (byte)1 + (byte)(t / 86400UL);
  t = t % 86400UL;

  byte hour = (byte)(t / 3600UL);
  t = t % 3600UL;

  byte min = (byte)(t / 60UL);
  t = t % 60UL;

  Assert(t < 60);
  byte sec = (byte)t;

  printf("%04d-%02d-%02d %02d:%02d:%02d UTC\n", year, month, day, hour, min, sec);

  if (set_time) {
      byte time_pack[6];
      time_pack[0] = (byte)(year - 1900);
      time_pack[1] = month;
      time_pack[2] = day;
      time_pack[3] = hour;
      time_pack[4] = min;
      time_pack[5] = sec;
      errnum err = Os9STime(time_pack);
      if (err) LogFatal("Cannot set time: errnum %d", err);
  }
}

static void FatalUsage() {
    LogFatal("Usage:  f-ntp -w0xFF68 server_addr:123");
}

int main(int argc, char* argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "sv:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
    case 's':
      set_time = true;
      break;
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
  if (argc != 1) FatalUsage();

  const char* p = argv[0];
  word server_port = DEFAULT_SERVER_PORT;
  quad server_addy = NyParseDottedDecimalQuadAndPort(&p, &server_port); 
  byte socknum = OpenLocalSocket();
  SNTP(socknum, server_addy, server_port);

  return 0;
}
