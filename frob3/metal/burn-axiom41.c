// Stand-alone DECB program to burn axiom41.
// #notyet# The program goes crazy around writing (D880,3D04,40) ?????

// clang-format off
#include "frob3/metal/metal.h"

#define VERBOSE_EEPROM_BURN 1

#include "frob3/metal/tty.h"  // before eeprom if VERBOSE_EEPROM_BURN
#include "frob3/metal/console.h"
#include "frob3/metal/eeprom.h"
#include "frob3/metal/romtail.h"
#include "frob3/metal/standard.h"

// clang-format on

byte Payload[] = {  // 8K bytes, from $C000 to $DFFF.
#include "generated-axiom41-bytes.h"
};

void BurnNow() {
  EepromDisableProtection();
  EepromDisableProtection();
  EepromDisableProtection();

  word loc = ScreenAt(12, 13);

  for (word i = 0; i < 0x2000; i += 64) {
  	EepromBurn(i+0xC000, (word)Payload+i, 64);
	SimpleShowHex(loc, 0x2000 - i);
  }

  EepromEnableProtection();
  EepromEnableProtection();
  EepromEnableProtection();
}

int main() {
  word stack = 0;
  asm("  sts %0" : "=m" (stack));

  ClearScreen(' ');
  {
  	word loc = ScreenAt(20, 0);
	SimpleShowHex(loc, stack);
  }

  PrintAt(0, 0, "*** DANGER ***");
  PrintAt(0, 1, "This program will erase and");
  PrintAt(0, 2, "rewrite your entire eeprom.");

  PrintAt(0, 4, "If you do not want that,");
  PrintAt(0, 5, "Reboot NOW!");

  PrintAt(0, 7, "To continue, flip the switch");
  PrintAt(0, 8, "for writing the EEPROM (if"); 
  PrintAt(0, 9, "needed), and hit the X key");
  PrintAt(0, 10, "to start.");


  while (true) {
  	char x = 31 & PolCat();
	if (x==(31&'X')) break; 
  }

  PrintAt(0, 12, "burning... please wait...");
  BurnNow();

  PrintAt(0, 14, "D\001O\001N\001E\001");
  PrintAt(0, 15, "(Undo the switch and REBOOT!)    ");
  while (1) continue;
}
