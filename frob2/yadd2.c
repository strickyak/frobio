#include "frob2/froblib.h"
#include "frob2/frobos9.h"

void ShowHex(char ch, word a);

#if 1
// This version works.
const char Greeting[] = "HELLO";
#else
// This version BREAKS.
const char* Greeting = "HELLO";
#endif

word SayHello() {
  word x = 0;
  for (word i = 0; i < 10 && Greeting[i]; i++) {
    ShowHex('i', i);
    ShowHex('&', (word)Greeting + i);
    ShowHex('g', Greeting[i]);
    x += Greeting[i];
  }
  return x;
}

int main() {
  ShowHex('A', (word) Greeting);
  word x = SayHello();
  ShowHex('x', x);
  word z = x - 0x0174;
  ShowHex('0', z);
  Os9Exit(z ? 13 : 88);
}

int goose;
__attribute__((section("hasxxx"))) int goosexxx;

int gander;
__attribute__((section("hasxxx"))) int ganderxxx;
