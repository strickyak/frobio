struct axiom4_rom_tail {     // $DFC0..$DFFF
  byte rom_reserved_16[16];  // $DFC0
  byte rom_waiter[4];        // $DFD0
  byte rom_dns[4];
  byte rom_hailing[8];
  byte rom_hostname[8];  // $DFE0
  byte rom_reserved_3[3];
  byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
  byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
};
#define RomTail ((struct axiom4_rom_tail*)0xDFC0)
