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
    byte ch = 63 & *s;
    Poke(screen, ch);
    screen++;
  }
}
void Fatal(const char* s) {
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
void PrintHex(word loc, word val) {
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
