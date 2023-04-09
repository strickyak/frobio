// A network chat program using Broadcast UDP.
// Each keystroke sends a UDP packet with a 5-byte payload.
//
// This uses the netboot3 API for binary programs.
// It uses the default VDG text screen, so it should work
// on almost any Coco.   (16K RAM required?)

#include "frob2/bootrom/bootrom3.h"
#include "frob2/bootrom/romapi3.h"

#define CHAT255_PORT 25538
#define CHAT255_MAGIC 99

// Protocol:
//   { 99, ch, W1, W2, W3 }
//   99 is magic number.
//   ch is character payload.
//   W1W2W3 is initials of sender.

struct globals {
  struct UdpRecvHeader hdr;
  char buf[5];
  char last[3];
  char me[3];
};

// #define A ((struct RomApi3*)0xC100)
#define G ((struct globals*)0x600)
#define A (Vars->rom_api_3)

const byte BroadcastIP[4] = {255, 255, 255, 255};

void *memset(void *s, int c, size_t n) {
  return A->api_memset(s, c, n);
}

void SetUp() {
  const struct sock* sock = &A->api_SockStructs[0];
  const struct proto* proto = A->api_BroadcastUdpProto;

  A->api_ConfigureTextScreen(VDG_RAM, /*orange=*/false);
  memset((char*)G, 0, sizeof *G);

  A->api_WizOpen(sock, proto, CHAT255_PORT);
  A->api_UdpDial(sock, proto, BroadcastIP, CHAT255_PORT);

  A->api_PutStr("\n\nEnter 3 letters\nfor your initials> ");
  for (byte i = 0; i < 3; i++) {
    char ch;
    do {
      ch = A->api_PolCat();
      if (ch > 96) ch -= 32;  // convert lower to upper.
    } while (ch < 'A' || 'Z' < ch);
    G->me[i] = ch;
    A->api_PutChar(ch);
  }
  A->api_PutChar('\n');
  A->api_PutChar('\n');
}

void ShowPayload() {
    if (G->last[0] != G->buf[2]  // did sender change?
        || G->last[1] != G->buf[3]
        || G->last[2] != G->buf[4]) {
      A->api_PutChar('(');  // Show new sender.
      A->api_PutChar(G->buf[2]);
      A->api_PutChar(G->buf[3]);
      A->api_PutChar(G->buf[4]);
      A->api_PutChar(')');
    }
    A->api_PutChar(G->buf[1]); // char payload
    G->last[0] = G->buf[2];
    G->last[1] = G->buf[3];
    G->last[2] = G->buf[4];
}

void Loop() {
  const struct sock* sock = &A->api_SockStructs[0];
  const struct proto* proto = A->api_BroadcastUdpProto;

  bool ok = A->api_WizRecvChunkTry(sock, (char*)(&G->hdr), sizeof G->hdr);

  if (ok) {
    if (G->hdr.len == 5) {
      A->api_WizRecvChunk(sock, G->buf, 5);

      if (G->buf[0] == CHAT255_MAGIC) ShowPayload();
      else A->api_PutChar('?');

    } else {
      A->api_Fatal("bad recv hdr.len", G->hdr.len);
    }
  }

  char ch = A->api_PolCat();

  if (32 <= ch && ch <= 126) {
    G->buf[0] = CHAT255_MAGIC;
    G->buf[1] = ch;
    G->buf[2] = G->me[0];
    G->buf[3] = G->me[1];
    G->buf[4] = G->me[2];

    ShowPayload();
    A->api_WizSendChunk(sock, proto, G->buf, 5);
  }
}


int main() {
  SetUp();
  while (true) {
    Loop();
  }
  return 0;
}
