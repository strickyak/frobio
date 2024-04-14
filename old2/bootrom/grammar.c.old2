#include "frob2/frobtype.h"
#include "frob2/frobjot/frobjot.h"


const byte* P;

byte Addy[4];
byte Mask[4];
byte Gate[4];
byte Resolver[4];
byte Waiter[4];
byte Name[4];
word EtherPort;

word GetDecimal() {
  byte ch;
  word z = 0;
  while ((ch = *P), '0' <= ch && ch <= '9') {
    z = (10 * z) + ch - '0';
    ++P;
  }
  return z;
}

void Take(byte a) {
  if (*P == a) ++P;
}

void GetName() {
  for (byte i = 0; i < 4; i++) {
    byte ch = *P;
    if ('A' <= ch && ch <= 'Z') {
      Name[i] = ch;
      ++P;
    } else {
      Name[i] = '?';
    }
  }
}

void GetDottedQuad(byte* q) {
  for (byte i=0; i<4; i++) {
    if (i) Take('.');
    q[i] = (byte) GetDecimal();
  }
}

void PParse() {
  byte ch;
  while (ch = *P) {
    switch (ch) {
      case 'A': Take('='); GetDottedQuad(Addy); break;
      case 'E': Take('='); EtherPort = GetDecimal(); break;
      case 'G': Take('='); GetDottedQuad(Gate); break;
      case 'M': Take('='); GetDottedQuad(Mask); break;
      case 'N': Take('='); GetName(); break;
      case 'R': Take('='); GetDottedQuad(Resolver); break;
      case 'W': Take('='); GetDottedQuad(Waiter); break;
    }
  }
}
