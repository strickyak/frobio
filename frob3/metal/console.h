char PolCat() {
  char inkey = 0;

#ifdef __GNUC__
  asm volatile(R"(
      jsr [$A000]
      sta %0
    )"
               : "=m"(inkey));
#else
  asm {
      jsr [$A000]
      sta :inkey
  }
#endif

  return inkey;
}

// Returns 0 if not Gomar, 'G' if is Gomar.
byte IsThisGomar() {
#ifdef __CMOC__
  byte result;
  asm {
     CLRA    ; optional, for 16-bit return value in D.
     CLRB
     NOP     ; begin hyper sequence...
     FCB $21 ; brn...
     FCB $FF ;    results in "LDB #$47" if on Gomar.
     STB :result
  }
  return result;
#else  // for GCC
  word result;
  asm volatile("CLRA\n  CLRB\n  NOP\n  FCB $21\n  FCB $ff\n std %0"
               : "=m"(result)  // the output
               :               // no inputs
               : "d"           // Clobbers D register.
  );
  return (byte)result;
#endif
}

void Delay(word n) {
  if (IsThisGomar()) return;  // delay not needed if Gomar.

  while (n--) {
#ifdef __GNUC__
    asm volatile("mul" : : : "d");
    asm volatile("mul" : : : "d");
    asm volatile("mul" : : : "d");
    asm volatile("mul" : : : "d");
    asm volatile("mul" : : : "d");
#else
    asm {
      mul
      mul
      mul
      mul
      mul
    }
#endif
  }
}

word ScreenAt(byte x, byte y) { return VDG_RAM + (y << 5) + x; }
void PrintAt(byte x, byte y, const char* s) {
  word screen = ScreenAt(x, y);
  for (; *s; s++) {
    if (*s == 1) {
    	Poke(screen-1, 0x40 ^ Peek(screen-1));  // Invert previous char.
	continue;
    }
    byte ch = (128 <= *s) ? (*s) : (96 <= *s) ? (*s-96) : (63 & *s);
    Poke(screen, ch);
    screen++;
    if ((screen & 31) == 0) break;  // wrapped end of line.
  }
}
void SimpleFatal(const char* s) {
  PrintAt(0, 14, "***  FATAL:  ***");
  PrintAt(0, 15, s);
  while (1) {
  }
}

void ClearScreen(byte ch) {
  for (word ptr = VDG_RAM; ptr < VDG_END; ptr++) {
    Poke(ptr, ch);
  }
}

const char VdgHex[] = "0123456789\001\002\003\004\005\006";

// Low level.
void SimpleShowHex(word loc, word val) {
  byte a = 15 & (val >> 12);
  byte b = 15 & (val >> 8);
  byte c = 15 & (val >> 4);
  byte d = 15 & (val >> 0);
  Poke(loc + 0, ' ');
  Poke(loc + 1, '(');
  Poke(loc + 2, VdgHex[a]);
  Poke(loc + 3, VdgHex[b]);
  Poke(loc + 4, VdgHex[c]);
  Poke(loc + 5, VdgHex[d]);
  Poke(loc + 6, ')');
  Poke(loc + 7, ' ');
}

void Pia1bOn(byte x) { *(volatile byte*)0xFF22 |= x; }
void Pia1bOff(byte x) { *(volatile byte*)0xFF22 &= ~x; }
void SetOrangeScreen() { Pia1bOn(0x08); }
void SetGreenScreen() { Pia1bOff(0x08); }

void Enable1BitSound() {
    *(volatile byte*)0xFF23 &= ~0x04;  // Clear bit 2 to enable Data Direction access
    *(volatile byte*)0xFF22 |= 0x02;   // Bit 1 and bits 3-7 are outputs.
    *(volatile byte*)0xFF23 |= 0x04;  // Clear bit 2 to enable Data Direction access
}

void Beep(byte n, byte f) {
    Enable1BitSound();

  for (byte i = 0; i < n; i++) {
    Pia1bOn(0x02);
    Delay(f);
    Pia1bOff(0x02);
    Delay(f);
  }
}
