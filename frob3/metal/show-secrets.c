// Burn hostname, mac, & secret into the CocoIOr EEPROM.

// clang-format off
#include "frob3/metal/metal.h"

#include "frob3/metal/console.h"
#include "frob3/metal/tty.h"
#include "frob3/metal/romtail.h"
#include "frob3/metal/standard.h"
// clang-format on

void PutHex2(byte x) {
	PutHex(15&(x>>4));
	PutHex(15&(x>>0));
}

void QuotedChars(const char* s, word len) {
  PutChar('\"');
  for (byte i = 0; i < 8; i++) {
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

int main() {
  ClearScreen(' ');

  PrintF("Hostname is ");
  QuotedChars(RomTail->rom_hostname, 8);

  PrintF("\nMAC is 02");
  for (byte i = 0; i < 5; i++) {
  	PutChar(':');
	PutHex2(RomTail->rom_mac_tail[i]);
  }

  PrintF("\nSecret is ");
  for (byte i = 0; i < 16; i++) {
	PutHex2(RomTail->rom_secrets[i]);
  	PutChar((i&3)==3 ? ' ' : ':');
  }

  PrintF("\nHailing code is ");
  QuotedChars(RomTail->rom_hailing, 8);

  PrintF("\nWaiter is %d.%d.%d.%d:%u\n",
    RomTail->rom_waiter[0],
    RomTail->rom_waiter[1],
    RomTail->rom_waiter[2],
    RomTail->rom_waiter[3],
    RomTail->rom_waiter_port);
  PrintF("\n(not used yet) DNS is %d.%d.%d.%d\n",
    RomTail->rom_dns[0],
    RomTail->rom_dns[1],
    RomTail->rom_dns[2],
    RomTail->rom_dns[3]);
  PrintF("\n(not used yet) Flags[3] are ($%x, $%x, $%x).\n",
    RomTail->rom_reserved_3[0],
    RomTail->rom_reserved_3[1],
    RomTail->rom_reserved_3[2]);

  PrintAt(0, 15, "D\001O\001N\001E\001   (Reboot!)");
  while (1) continue;
}
