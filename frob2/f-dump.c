// f-dump (like tcpdump).

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

bool show_from;
byte packet[1550];

byte OpenLocalSocket() {
  byte socknum = 0;
  errnum err = macraw_open(&socknum);
  if (err) LogFatal("cannot macraw_open: %d\n", err);
  return socknum;
}

void MacRawDump(byte* packet, word size) {
  word sz = *(word*)(packet);
  byte* d = packet+2;
  byte* s = packet+8;
  word type = *(word*)(packet+14);
  byte* ip = packet+16;

  printf("sz=%x dest=%02x:%02x:%02x:%02x:%02x:%02x src=%02x:%02x:%02x:%02x:%02x:%02x type=%x\n",
      sz, 
      d[0], d[1], d[2], d[3], d[4], d[5],
      s[0], s[1], s[2], s[3], s[4], s[5],
      type);

  printf("ip=");
  for (byte i=0; i<20; i++) {
    if ((i&3)==0) printf(" ");
    printf("%02x", ip[i]);
  }
  for (byte i=0; i<sz-16-20; i++) {
    if ((i&15)==0) printf("\n");
    if ((i&1)==0) printf(" ");
    if ((i&3)==0) printf(" ");
    printf("%02x", ip[i+20]);
  }
  printf("\n");
  for (byte i=0; i<sz-16-20; i++) {
    byte c = ip[i+20];
    if (' ' <= c && c <= '~') {
      printf("%c", ip[i+20]);
    } else {
      printf("~");
    }
  } 
  printf("\n");
  printf("\n");
}

void MainLoop(byte socknum) {
  while (true) {
      word size = sizeof packet;
      errnum err = macraw_recv(socknum, packet, &size);
      if (err) LogFatal("cannot macraw_recv data: %d\n", err);

      MacRawDump(packet, size);
  }
}

static void FatalUsage() {
    LogFatal("Usage:  f-recv -w0xFF68 -pLocalPort | ...\n");
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

  if (argc != 0) {
    FatalUsage();
  }
  byte socknum = OpenLocalSocket();
  if (socknum != 0) {
    LogFatal("Only socknum 0 can run f-dump, but got socknum %d", socknum);
  }
  MainLoop(socknum);

  return OKAY;
}
