// ... | f.send peeraddr:port

#include "frobio/frobio.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"

bool show_from;
byte packet[2000];

byte OpenLocalSocket() {
  byte socknum = 0;
  error err = macraw_open(&socknum);
  if (err) NyFatalD("cannot macraw_open: %d\n", err);
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
      error err = macraw_recv(socknum, packet, &size);
      if (err) NyFatalD("cannot macraw_recv data: %d\n", err);

      MacRawDump(packet, size);
  }
}

static void UsageAndExit() {
    printf("Usage:  f.recv -wWiznetPortHex -pLocalPort | ...\n");
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

  if (argc != 0) {
    UsageAndExit();
  }
  byte socknum = OpenLocalSocket();
  MainLoop(socknum);

  return OKAY;
}
