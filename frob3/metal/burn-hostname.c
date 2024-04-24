// Burn hostname, mac, & secret into the CocoIOr EEPROM.

// clang-format off
#include "frob3/metal/metal.h"

#include "frob3/metal/arcfour.h"
#include "frob3/metal/console.h"
#include "frob3/metal/eeprom.h"
#include "frob3/metal/romtail.h"
#include "frob3/metal/standard.h"
// clang-format on

char hailing[8];
char hostname[8];
struct rc4_engine engine;
struct axiom4_rom_tail rom;

/////////////////////////////////////////

//#define PEEK1(A) (*(volatile byte*)(A))
//#define POKE1(A, X) (*(volatile byte*)(A)) = (X)

#include "frob3/wiz/w5100s_defs.h"

/////////////////////////////////////////

byte Randomize(void* p, size_t len) { rc4_mix_key(p, len, &engine); }

byte RandomByte() {
  byte x = 0;
  rc4(&x, 1, &engine);
  return x;
}

void BurnNow() {
  EepromDisableProtection();
  EepromDisableProtection();
  EepromDisableProtection();

  EepromBurn(0xDFC0, (word)&rom, sizeof rom);

  EepromEnableProtection();
  EepromEnableProtection();
  EepromEnableProtection();
}

byte ToUpper(byte c) {
  if ('a' <= c && c <= 'z') return c - 32;
  return c;
}

bool IsAlfa(byte c) {
  c = ToUpper(c);
  return ('A' <= c && c <= 'Z');
}

bool IsNum(byte c) { return ('0' <= c && c <= '9'); }

bool EnterAlfa(byte* dest, bool can_num, bool can_enter) {
  bool z;
  struct {
    word w;
    byte c;
  } vars;
  vars.w = 0;
  while (true) {
    vars.w++;
    vars.c = PolCat();
    if (can_enter && (vars.c == 10 || vars.c == 13)) {
      z = true;
      break;
    }
    if (can_num && IsNum(vars.c)) {
      *dest = vars.c;
      z = false;
      break;
    }
    if (IsAlfa(vars.c)) {
      *dest = vars.c;
      z = false;
      break;
    }
  }

  Randomize(&vars, sizeof vars);
  return z;
}

int main() {
  ClearScreen(':');
  rc4_init_engine(&engine);

  PrintAt(0, 0, "Configure your CocoIOr ROM:");

  PrintAt(0, 2, "Enter 3 to 8 letter hostname.");
  PrintAt(0, 3, "Use only A-Z and 0-9.");

  PrintAt(0, 5, "Also set your CocoIO to burn.");
  PrintAt(0, 5, "Hit ENTER when done.");

  PrintAt(0, 7, "  \x9f \x9f \x9f \x9f \x9f \x9f \x9f \x9f");

  byte* p = (byte*)ScreenAt(2, 7);
  bool done = false;
  if (!done) done = EnterAlfa(p + 0, false, false);
  if (!done) done = EnterAlfa(p + 2, true, false);
  if (!done) done = EnterAlfa(p + 4, true, false);
  if (!done) done = EnterAlfa(p + 6, true, false);
  if (!done) done = EnterAlfa(p + 8, true, false);
  if (!done) done = EnterAlfa(p + 10, true, false);
  if (!done) done = EnterAlfa(p + 12, true, false);
  if (!done) done = EnterAlfa(p + 14, true, false);

  memset(&rom, 0, sizeof rom);
  for (byte i = 0; i < 8; i++) {
    byte c = Peek(p + (i << 1));
    if (IsAlfa(c) || IsNum(c)) {
      rom.rom_hostname[i] = c;
    } else {
      rom.rom_hostname[i] = 32;
    }
  }
  for (byte i = 0; i < 5; i++) {
    rom.rom_mac_tail[i] = RandomByte();
  }
  for (byte i = 0; i < 16; i++) {
    rom.rom_secrets[i] = RandomByte();
  }
  for (byte i = 0; i < 4; i++) {
    rom.rom_dns[i] = 8;  // Google 8.8.8.8
  }
  for (byte i = 0; i < 8; i++) {
    rom.rom_hailing[i] = '-';  // for now
  }
  byte lemma_yak_net[4] = {134, 122, 16, 44};
  for (byte i = 0; i < 4; i++) {
    rom.rom_waiter[i] = 8;  // Google 8.8.8.8
  }

  BurnNow();

  PrintAt(0, 14, "DONE");
  while (1) continue;
}
