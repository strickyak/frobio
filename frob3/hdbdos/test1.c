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

void Hex(byte label, word val);

void LemmaRead(byte* buf, word count) {
  asm volatile (
    "  ldx %0\n"
    "  ldy %1\n"
    "  jsr LemmaRead"
    : // no outputs
    : "m" (buf), "m" (count) // inputs
    : "d", "x", "y" // clobbers
  );
}

void LemmaWrite(byte* buf, word count) {
  Hex('B', (word)buf);
  Hex('C', count);
  asm volatile (
    "  ldx %0\n"
    "  ldy %1\n"
    "  jsr LemmaWrite"
    : // no outputs
    : "m" (buf), "m" (count) // inputs
    : "d", "x", "y" // clobbers
  );
}

byte Peek(word addr) {
	volatile byte* p = (byte*)addr;
	return *p;
}
word Peek2(word addr) {
	word hi = Peek(addr);
	word lo = Peek(addr+1);
	return (hi<<8) | lo;
}
void Poke(word addr, byte val) {
	volatile byte* p = (byte*)addr;
	*p = val;
}
void Poke2(word addr, word val) {
	Poke(addr+0, (byte)(val>>8));
	Poke(addr+1, (byte)(val>>0));
}

#define CURSOR 255
#define PROMPT 255
void Emit(byte b) {
	if (96 <= b && b < 128) b -= 32;
	b &= 63;
	byte cursor = Peek(CURSOR);
	Poke(0x500+cursor, b);
	cursor++;
	Poke(0x500+cursor, PROMPT);
	Poke(CURSOR, cursor);
}

char alfa[] = "0123456789ABCDEF";
void Hex(byte label, word val) {
	byte a = 15 & (val>>4);
	byte b = 15 & val;
	// Poke(0x400+offset+0, alfa[a]);
	// Poke(0x400+offset+1, alfa[b]);
	Emit(' ');
	Emit(label);
	Emit('=');
	Emit(alfa[a]);
	Emit(alfa[b]);
}
void Hex2(byte label, word val) {
	byte a = 15 & (val>>12);
	byte b = 15 & (val>>8);
	byte c = 15 & (val>>4);
	byte d = 15 & val;
	Emit(' ');
	Emit(label);
	Emit('=');
	Emit(alfa[a]);
	Emit(alfa[b]);
	Emit(alfa[c]);
	Emit(alfa[d]);
}
void MemSet(word addr, byte val, word count) {
	while (count--) Poke(addr++, val);
}

void Delay(word n) {
  while (n--) {
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
  }
}

word say_n;
void Say() {
  ++say_n;
  Poke(0x480+say_n, 64+(byte)say_n);
  Delay(5000);
}

void Stop() {
  word d_, x_, y_, u_, cc_;
  asm volatile (
    "  std %0 \n"
    "  stx %1 \n"
    "  sty %2 \n"
    "  stu %3 \n"
    "  tfr cc,b \n"
    "  clra \n"
    "  std %4 "
    : // no outputs
    : "m" (d_), "m" (x_), "m" (y_), "m" (u_), "m" (cc_)
    : // no clobbers
  );
  Emit(' ');
  Emit('#');
  Emit('#');
  Hex2('d', d_);
  Hex2('x', x_);
  Hex2('y', y_);
  Hex2('u', u_);
  Hex('c', cc_);
  Emit(' ');
  Emit('#');
  while (1) continue;
}

void Mem(word p) {
	word offset = 256;
	Hex2(offset-32, p);
	for (word i = 0; i < 32; i+=2) {
		Hex2(offset, *(word*)(p+i));
		offset += 5;
	}
}

int main() {
  MemSet(0x400, '.', 0x200);
  Delay(3000);
  for (byte i = '0'; i < '9'; i++) { Emit(i); }
  Delay(3000);
  for (byte i = 'A'; i < 'Z'; i++) { Emit(i); }
  Delay(3000);

  for (word k = 0; k < 10000; k++ ) {
	  Hex2('k', k);

	  char quint[5] = { 217/*echo*/, 0, 4, 255&(k>>8), 255&k };
	  Hex2('q', (word)quint);
	  for (byte i = 0; i < 5; i++) { Hex('@', quint[i]); }
	  // Delay(3000);

	  LemmaWrite(quint, 5);
	  Hex('C', Peek((word)quint));
	  Hex2('N', Peek2((word)quint+1));
	  Hex2('P', Peek2((word)quint+3));
	  quint[0] = 33;
	  quint[1] = 34;
	  quint[2] = 35;
	  quint[3] = 130;
	  quint[4] = 255;
	  LemmaWrite(quint, 4);

	  LemmaRead(quint, 5);
	  Hex('^', quint[0]);
	  Hex('^', quint[1]);
	  Hex('^', quint[2]);
	  Hex('^', quint[3]);
	  Hex('^', quint[4]);
	  if (quint[0] != 0xCC) Stop();
	  if (quint[3] != (255&(k>>8))) Stop();
	  if (quint[4] != (255&(k>>0))) Stop();

	  LemmaRead(quint, 4);
	  Hex('=', quint[0]);
	  Hex('=', quint[1]);
	  Hex('=', quint[2]);
	  Hex('=', quint[3]);
	  if (quint[0] != 33+128) Stop();
	  if (quint[1] != 34+128) Stop();
	  if (quint[2] != 35+128) Stop();
	  if (quint[3] != 130-128) Stop();
  }

  Stop();

  while (1) continue;
  return 0;
}
