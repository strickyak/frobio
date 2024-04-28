// coypu-for-copico.c -- a COYPU client for Copico.
// See https://github.com/strickyak/coypu-daemon

// clang-format off
#include "frob3/metal/metal.h"

#include "frob3/metal/console.h"
#include "frob3/metal/standard.h"
#include "frob3/metal/tty.h"
// clang-format on

byte request[64];
byte reply[1024];

void PutHex2(byte x) {
  PutHex(15 & (x >> 4));
  PutHex(15 & (x >> 0));
}

void QuotedChars(const char* s, word len) {
  PutChar('\"');
  for (byte i = 0; i < len; i++) {
    byte ch = s[i];
    if (32 <= ch && ch <= 'Z') {
      PutChar(ch);
    } else {
      PutChar('\\');
      PutChar('x');
      PutHex2(ch);
    }
  }
  PutChar('\"');
}

#define CONTROL 0xFF7A
#define DATA 0xFF7B

// Send 64 bytes.
void SendRequest(byte* p) {
  Poke(CONTROL, 0);
  Delay(500);
  word count = 64;
  while (count) {
    Delay(1);
    Poke(DATA, *p++);
    count--;
  }
}

// Receive 1024 bytes.
void GetReply(byte* p) {
  Poke(CONTROL, 1);
  Delay(5000);
  word count = 1024;
  while (count) {
    Delay(1);
    byte x = Peek(DATA);
    if (!x) continue;
    *p++ = x;
    count--;
  }
}

void main() {
  ClearScreen('/');
  Delay(2000);
  PrintF("Coypu for Copico. %x %x\n", request, reply);
  Delay(2000);

  while (1) {
    PrintF("Requesting...\n");

    memset(request, ' ', 64);
    memcpy(request, "30", 2);
    SendRequest(request);

    Delay(20000);
    PrintF("Getting Reply...\n");
    GetReply(reply);

    for (word i = 0; i < 2; i++) {
      PutChar(' ');
      QuotedChars(reply + (i << 6), 64);
      PutChar(' ');
    }
    Delay(20000);
    PrintF(" OKAY... ");
  }
}
