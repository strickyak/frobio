typedef unsigned char byte;
typedef unsigned int word;

void printk(const char* s, ...);
void Fatal(const char* s, word x);
void Delay(word d);

struct expect {
  word page;
  word sum;
  word xor;
};

void Checksum() {
  struct expect* expect = (struct expect*) 0xE000;
  for (word a = 0xC000; a < 0xDF00; a+=0x0100) {
    word sum = 0;
    word xor = 0;
    for (word b=0; b < 256; b+=2) {
      word w = *(word*)(a+b);
      sum += w;
      xor ^= w;
    }
    --expect;
    if ((a&0x3FFF)!=expect->page || sum!=expect->sum || xor!=expect->xor) {
      printk("%x/%x: %x/%x %x/%x", a, expect->page, sum, expect->sum, xor, expect->xor);
    } else {
      printk("%x", a);
    }
    Delay(1000);
  }
  Delay(5000);
}
