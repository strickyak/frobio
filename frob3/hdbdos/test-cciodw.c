// Conventional types and constants for Frobio
#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1
typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
typedef void (*func_t)();


void DWRead(byte* buf, word count) {
  asm(
    "  ldx %0\n"
    "  ldy %1\n"
    "  jsr DWRead"
    : // no outputs
    : "m" (buf), "m" (count) // inputs
    : "d", "x", "y" // clobbers
  );
}

void DWWrite(byte* buf, word count) {
  asm(
    "  ldx %0\n"
    "  ldy %1\n"
    "  jsr DWWrite"
    : // no outputs
    : "m" (buf), "m" (count) // inputs
    : "d", "x", "y" // clobbers
  );
}

void Poke(word addr, byte val) {
	*(volatile byte*)addr = val;
}

char alfa[] = "0123456789ABCDEF";
void Hex(word offset, byte val) {
	byte a = 15 & (val>>8);
	byte b = 15 & val;
	Poke(0x400+offset+0, alfa[a]);
	Poke(0x400+offset+1, alfa[b]);
}
void MemSet(word addr, byte val, word count) {
	while (count--) Poke(addr++, val);
}

int main() {
  char quint[5] = { 254, 0, 0, 0, 0 };
  MemSet(0x400, '-', 0x200);
  DWWrite(quint, 5);

  MemSet(0x400, '+', 0x200);

  DWRead(quint, 5);
  MemSet(0x400, '.', 0x200);

  Hex(3, quint[0]);
  Hex(6, quint[1]);
  Hex(9, quint[2]);
  Hex(12, quint[3]);
  Hex(15, quint[4]);

  while (1) continue;
  return 0;
}
