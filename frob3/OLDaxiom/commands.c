#include "frob3/axiom/bootrom3.h"

#define BUF (Vars->line_buf)
#define PTR (Vars->line_ptr)

#define maybe_static

void NativeMode() {
  asm volatile("ldmd #1");
}

void GetUpperCaseLine(char initialChar) {
  memset(BUF, 0, sizeof BUF);
  char* p = BUF;  // Rewind.

  if (33 <= initialChar && initialChar <= 126) {
    *p++ = initialChar;
     PutChar(initialChar);
  }

  while (true) {
    char ch = PolCat();
    if (!ch) continue;
    if (ch==13) break; // CR

    if (ch == 8) { // backspace
      if (p > BUF) {
        --p;
        *p = '\0';
        PutChar(8);
      }
      continue;
    }

    if (ch < 32) continue;  // no control
    if (ch > 126) continue; // only printable ASCII

    if ('a' <= ch && ch <= 'z') ch -= 32; // to UPPER case

    if (p < BUF + sizeof BUF - 2) {
      *p = ch;
      p++;
      PutChar(ch);
    }
  }
  PutChar(13);

  PTR = BUF;  // Rewind.
}

void DoError(errnum e) {
  PrintF(" *** ERROR %d\n", e);
}

maybe_static void SkipWhite() {
  while (*PTR == ' ' || *PTR == '.' || *PTR == '/' || *PTR == ':') ++PTR;
}

#if 0
// *num_out is set to 0, if no number.
maybe_static bool GetNum1Byte(byte* num_out) {
  SkipWhite();
  bool gotnum = false;
  byte x = 0;
  while ('0' <= *PTR && *PTR <= '9') {
    x = x * 10 + (*PTR - '0');
    gotnum = true;
    ++PTR;
  }
  *num_out = x;
  return gotnum;
}
#endif

maybe_static bool GetNum2Bytes(word* num_out) {
  SkipWhite();
  bool gotnum = false;
  word x = 0;
  if (*PTR == '$') {
    ++PTR;
    while (('0' <= *PTR && *PTR <= '9') ||
           ('A' <= *PTR && *PTR <= 'F')) {
      if (*PTR <= '9') {
        x = x * 16 + (*PTR - '0');
      } else {
        x = x * 16 + (*PTR - 'A' + 10);
      }
      gotnum = true;
      ++PTR;
    }
  } else {
    while ('0' <= *PTR && *PTR <= '9') {
      x = x * 10 + (*PTR - '0');
      gotnum = true;
      ++PTR;
    }
  }
  *num_out = x;
  return gotnum;
}
maybe_static bool GetNum1Byte(byte* num_out) {
  word x;
  bool b = GetNum2Bytes(&x);
  *num_out = (byte)x;
  return b;
}

maybe_static bool GetAddyInXid() {
  memset(Vars->xid, 0, 4);
  if (GetNum1Byte(Vars->xid+0) && 
      GetNum1Byte(Vars->xid+1) && 
      GetNum1Byte(Vars->xid+2) && 
      GetNum1Byte(Vars->xid+3) ) {
    return true;
  }
  return false;
}

maybe_static byte MaskBits(int n) {
  byte z = 0;
  while (n>0) {
    z = (z >> 1) | 0x80;
    n--;
  }
  return z;
}

maybe_static void SetMask(byte width) {
      if (width == 0) width = 24;
      Vars->mask_num = width;
      Vars->ip_mask[0] = MaskBits((int)width);
      Vars->ip_mask[1] = MaskBits((int)width-8);
      Vars->ip_mask[2] = MaskBits((int)width-16);
      Vars->ip_mask[3] = MaskBits((int)width-24);
}

#if __GOMAR__
maybe_static void UseLoopbackForEmulator() {
  Vars->ip_addr[0] = 127;
  Vars->ip_addr[1] = 0;
  Vars->ip_addr[2] = 0;
  Vars->ip_addr[3] = 1;

  Vars->ip_gateway[0] = 0;  // unusable.
  Vars->ip_gateway[1] = 7;
  Vars->ip_gateway[2] = 11;
  Vars->ip_gateway[3] = 13;

  Vars->ip_waiter[0] = 127;
  Vars->ip_waiter[1] = 0;
  Vars->ip_waiter[2] = 0;
  Vars->ip_waiter[3] = 1;
  Vars->waiter_port = WAITER_TCP_PORT;

  SetMask(24);
}

void DoKeyboardCommands() {
  UseLoopbackForEmulator();

  PrintF("Use Loopback for Emulator.\n");
  PrintF("Launch...\n");
}

#else // __GOMAR__

maybe_static errnum DoNetwork(byte a, byte b) {
  byte tail;
  if (!GetNum1Byte(&tail)) {
    tail = 30 + (byte)(WizTicks() % 200u);  // random 30..229
  }
  Vars->ip_addr[0] = a;
  Vars->ip_addr[1] = b;
  Vars->ip_addr[2] = 23;
  Vars->ip_addr[3] = tail;

  Vars->ip_gateway[0] = a;
  Vars->ip_gateway[1] = b;
  Vars->ip_gateway[2] = 23;
  Vars->ip_gateway[3] = 1;

  Vars->ip_waiter[0] = a;
  Vars->ip_waiter[1] = b;
  Vars->ip_waiter[2] = 23;
  Vars->ip_waiter[3] = 23;

  SetMask(24);
  return OKAY;
}

maybe_static void GetHostname() {
  byte* h = Vars->hostname;
  byte n = sizeof(Vars->hostname);

  memset(h, '_', n);
  SkipWhite();
  while ('!' <= *PTR && *PTR <= '~') {
    for (byte i = 0; i < n-1; i++) {
      h[i] = h[i+1];  // Shift name to the left.
    }
    h[n-1] = *PTR;
    ++PTR;
  }
}

maybe_static void ShowNetwork() {
  if ((word)Vars->wiz_port == 0xFF78) {
    PrintF("u\1\n");
  }
  if (Vars->need_dhcp) {
    PrintF("d\1\n");
  } else {
    PrintF("i\1 %a/%d %a\n", Vars->ip_addr, Vars->mask_num, Vars->ip_gateway);
  }
  PrintF("w\1 %a:%d\n", Vars->ip_waiter, Vars->waiter_port);
}

// returns true when commands are done.
bool DoOneCommand(char initialChar) {
  bool done = false;
  errnum e = OKAY;
  word peek_addr = 0;

  GetUpperCaseLine(initialChar);

  SkipWhite();
  char cmd = *PTR;
  PTR++;

  if (cmd == 'U') {
      Vars->wiz_port = (struct wiz_port*)0xFF78;
  } else if (cmd == 'D') {
      GetHostname();
      Vars->need_dhcp = true;
  } else if (cmd == 'I') {
      if (!GetAddyInXid()) { e = 11; goto END; }
      memcpy(Vars->ip_addr, Vars->xid, 4);
      memset(Vars->ip_gateway, 0, 4);
      byte width;
      if (GetNum1Byte(&width)) {
        if (GetAddyInXid()) {
          memcpy(Vars->ip_gateway, Vars->xid, 4);
        }
      }
      SetMask(width);
      
  } else if (cmd == 'W') {
    if (!GetAddyInXid()) { e = 12; goto END; }
    memcpy(Vars->ip_waiter, Vars->xid, 4);
    word port;
    if (GetNum2Bytes(&port)) {
        Vars->waiter_port = port;
    }

  } else if (cmd == 'S') {
    ShowNetwork();

  } else if (cmd == 'A') {
    e = DoNetwork(10, 23);

  } else if (cmd == 'B') {
    e = DoNetwork(176, 23);

  } else if (cmd == 'C') {
    e = DoNetwork(192, 168);

  } else if (cmd == 'X') {
    e = DoNetwork(127, 23);

  } else if (cmd == 'Y') {
    e = DoNetwork(44, 23);

  } else if (cmd == 'Z') {
    e = DoNetwork(0, 23);

  } else if (cmd == '@') {
    done = true;
    goto END;

  } else if (cmd == 'Q') {
    *(byte*)0xFFD9 = 1;

  } else if (cmd == 'N') {
    NativeMode();

  } else if (cmd == '<') {
    word tmp = peek_addr;
    if (!GetNum2Bytes(&peek_addr)) peek_addr = tmp;

    PrintF("%x: ", peek_addr);
    for (byte i = 0; i < 8; i++) {
      PrintF(" %x", *(byte*)peek_addr);
      ++peek_addr;
    }
    PutChar(13);

  } else if (cmd == '>') {
      byte b;
    if (GetNum2Bytes(&peek_addr) && GetNum1Byte(&b)) {
      *(byte*)peek_addr = b;
    } else {
      PrintF("?\n");
    }

  } else if (cmd == 0) {
    // end of line
  } else {
    PrintF("U\1 :upper wiznet port $FF78\n");
    PrintF("Q\1 :quick: poke 1 to $FFD9\n");
    PrintF("N\1 :native mode for H6309\n");
    PrintF("D\1 :use DHCP\n");
    PrintF("I\1 1.2.3.4/24 5.6.7.8\n");
    PrintF("  :Set IP addr, mask, gateway\n");
    PrintF("W\1 3.4.5.6:%d :set waiter\n", WAITER_TCP_PORT);
    PrintF("A\1 :preset 10.23.23.*\n");
    PrintF("B\1 :preset 176.23.23.*\n");
    PrintF("C\1 :preset 192.168.23.*\n");
    PrintF("X\1 or Y\1 or Z\1 :goofy presets\n");
    PrintF("S\1 :show settings\n");
    PrintF("Finally:  @\1 : launch!\n");
  }

END:
  if (e) {
    DoError(e);
  }
  return done;
}

maybe_static char CountdownOrInitialChar() {
  PrintF("\n\nTo take control, hit space bar.\n\n");
  for (byte i = 0; i < 5; i++) {
    PrintF("%d... ", 5-i);
    for (word j = 0; j < 1500; j++) {
      char initialChar = PolCat();
      if (initialChar) return initialChar;
    }
  }
  PrintF("0.\n");
  return '\0';
}

maybe_static const byte waiter_default [4] = { 134, 122, 16, 44 }; // lemma.yak.net
void DoKeyboardCommands() {
  memcpy(Vars->ip_waiter, waiter_default, 4);
  Vars->waiter_port = WAITER_TCP_PORT;
  SetMask(24);

  char initialChar = CountdownOrInitialChar();
  if (!initialChar) {
    Vars->need_dhcp = true;
    return;
  }

  PrintF("Enter H for HELP.\n");

  while (true) {
    PrintF(">axiom> ");
    bool done = DoOneCommand(initialChar);
    if (done) break;
    initialChar = 0;
  }
  PrintF("Launch... ");
}

#endif // __GOMAR__
