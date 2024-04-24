#ifndef _METAL_EEPROM_H_
#define _METAL_EEPROM_H_

#define CART_ROM_ADDR 0xC000

// Software Data Protection:
// See page 10 (sections 19, 20) of
// https://ww1.microchip.com/downloads/en/DeviceDoc/doc0270.pdf
// for these magic numbers.
void EepromEnableProtection() {
  Delay(500);
  Poke(CART_ROM_ADDR + 0x1555, 0xAA);
  Poke(CART_ROM_ADDR + 0x0AAA, 0x55);
  Poke(CART_ROM_ADDR + 0x1555, 0xA0);
  Delay(500);
}

void EepromDisableProtection() {
  Delay(500);
  Poke(CART_ROM_ADDR + 0x1555, 0xAA);
  Poke(CART_ROM_ADDR + 0x0AAA, 0x55);
  Poke(CART_ROM_ADDR + 0x1555, 0x80);
  Poke(CART_ROM_ADDR + 0x1555, 0xAA);
  Poke(CART_ROM_ADDR + 0x0AAA, 0x55);
  Poke(CART_ROM_ADDR + 0x1555, 0x20);
  Delay(500);
}

void EepromBurn(word dest, word src, word n) {
  byte* src_ptr = (byte*)src;
  while (n) {
    // Starting at p, what is the most we can burn in a 64 byte chunk?
    byte size = 64 - (63 & (byte)dest);
    // But we cannot burn more than n.
    if (n < size) size = n;
    // Recv the next n bytes and burn them.
    for (byte i = 0; i < size; i++) {
      Poke(dest + i, *src_ptr++);
    }
    n -= size;
    dest += size;
    Delay(500);
  }
}

#endif  // _METAL_EEPROM_H_
