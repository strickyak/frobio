#include "frob2/froblib.h"
#include "frob2/frobos9.h"

void ShowHex(char ch, word a);

#define Count (*(int*)0x0022)

void Increment(int addend) {
  Count = Count + addend;
}

int main() {
  Count = 0;
  for (int i=1; i<=100; i++) {
    Increment(i);
    ShowHex('i', i);
    ShowHex('c', Count);
  }
  ShowHex('C', Count);
  word z = Count - 5050;
  ShowHex('0', z);
  Os9Exit(z ? 13 : 88);
}

int goose;
__attribute__((section("hasxxx"))) int goosexxx;

int gander;
__attribute__((section("hasxxx"))) int ganderxxx;
