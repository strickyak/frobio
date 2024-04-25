// Burn hostname, mac, & secret into the CocoIOr EEPROM.

// clang-format off
#include "frob3/metal/metal.h"

#include "frob3/metal/arcfour.h"
#include "frob3/metal/console.h"
#include "frob3/metal/eeprom.h"
#include "frob3/metal/romtail.h"
#include "frob3/metal/standard.h"
// clang-format on

#define HAILING "SWI4SWI5"
char hostname[8];
struct rc4_engine engine;
struct axiom4_rom_tail rom;

byte Randomize(void* p, size_t len) { rc4_mix_key(p, len, &engine); }

byte RandomByte() {
  byte x = 0;
  rc4(&x, 1, &engine);
  return x;
}

void BurnNow(char how) {
  EepromDisableProtection();
  EepromDisableProtection();
  EepromDisableProtection();

  switch (how) {
    case 'H':
    	EepromBurn(0xDFE0, (word)&rom.rom_hostname, 8);
	break;
    case 'I':
    	EepromBurn(0xDFC0, (word)&rom, sizeof rom);
	break;
  }

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
    SimpleShowHex(0x0600-8, vars.w);

    vars.c = PolCat();
    if (!vars.c) continue;

    if (can_enter && (vars.c <= 32)) { // space, CR, LF, any control.
      z = true;
      break;
    }
    if (can_num && IsNum(vars.c)) {
      *dest = vars.c;
      z = false;
      break;
    }
    if (IsAlfa(vars.c)) {
      *dest = vars.c & 63;
      z = false;
      break;
    }
  }

  Randomize(&vars, sizeof vars);
  return z;
}

int main() {
  ClearScreen(' ');
  rc4_init_engine(&engine);

  PrintAt(0, 0, "Configure your CocoIOr ROM:");

  PrintAt(0, 2, "Choose 3 to 8 letter hostname.");
  PrintAt(0, 3, "Use only A\001-Z\001 and 0\001-9\001.");

  PrintAt(0, 5, "Hit E\001N\001T\001E\001R\001 when done.");

  PrintAt(0, 7, "  \xAF \xAF \xAF \xAF \xAF \xAF \xAF \xAF");

  byte* p = (byte*)ScreenAt(2, 7);
  bool done = false;
  if (!done) done = EnterAlfa(p + 0, false, false);
  if (!done) done = EnterAlfa(p + 2, true, false);
  if (!done) done = EnterAlfa(p + 4, true, false);
  if (!done) done = EnterAlfa(p + 6, true, true);
  if (!done) done = EnterAlfa(p + 8, true, true);
  if (!done) done = EnterAlfa(p + 10, true, true);
  if (!done) done = EnterAlfa(p + 12, true, true);
  if (!done) done = EnterAlfa(p + 14, true, true);

  memset(&rom, 0, sizeof rom);
  rom.rom_waiter_port = 2321;  // V41 port number.

  for (byte i = 0; i < 8; i++) {
    byte c = Peek(p + (i << 1));
    if (c<128) {
      rom.rom_hostname[i] = (c<32) ? (c+64) : c;
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
    rom.rom_hailing[i] = HAILING[i];
  }
  byte lemma_yak_net[4] = {134, 122, 16, 44};
  for (byte i = 0; i < 4; i++) {
    rom.rom_waiter[i] = lemma_yak_net[i];
  }

  PrintAt(0, 9, "If needed, switch your CocoIOr");
  PrintAt(0, 10, "to burn.  To only change the");
  PrintAt(0, 11, "hostname, hit H\001.  To init the");
  PrintAt(0, 12, "entire config, with a new random");
  PrintAt(0, 13, "secret, hit I\001.");

  char how;
  while (true) {
  	how = 31 & PolCat();
	if (how==(31&'H') || how==(31&'I')) {
		break;
	}
  }

  BurnNow(how+64);

  PrintAt(0, 14, "D\001O\001N\001E\001");
  PrintAt(0, 15, "(Undo the switch and REBOOT!)    ");
  while (1) continue;
}
