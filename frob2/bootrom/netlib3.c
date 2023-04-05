#include "frob2/bootrom/bootrom3.h"
#include "frob2/bootrom/romapi3.h"

const char RevDate[16] = __DATE__;
const char RevTime[16] = __TIME__;

////////////////////////////////////////////////////////

void* memcpy(void* dest, const void* src, size_t n) {
  char* d = (char*)dest;
  char* s = (char*)src;
  for (size_t i = 0; i < n; i++) *d++ = *s++;
  return dest;
}

void *memset(void *s, int c, size_t n) {
  char* dest = (char*)s;
  for (size_t i = 0; i < n; i++) *dest++ = c;
  return s;
}

char *strcpy(char *restrict dest, const char *restrict src) {
  while (*src) *dest++ = *src++;
  return dest;
}

char *strncpy(char *restrict dest, const char *restrict src, size_t n) {
  void* dest0 = dest;
  int i = 0;
  while (*src) {
    *dest++ = *src++;
    i++;
    if (i>=n) break;
  }
  return dest;
}

size_t strlen(const char *s) {
  const char* p = s;
  while (*p) p++;
  return p-s;
}

////////////////////////////////////////////////////////
const byte HexAlphabet[] = "0123456789abcdef";

#if MULTI_SOCK
// If multiple sockets are supported,
// this table has the relevant facts about each.

// Sorry about the awkward construction of the SockState location,
// but I had to make it a constant integer.
const struct sock Socks[4] = {
    { 0x400, 0x4000, 0x6000, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*0, 0 },
    { 0x500, 0x4800, 0x6800, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*1, 1 },
    { 0x600, 0x5000, 0x7000, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*2, 2 },
    { 0x700, 0x5800, 0x7800, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*3, 3 },
};
#endif

#if X220NET
// For my coco3(10.1.2.3) with ethernet(255.0.0.0) wired to a laptop(10.2.2.2).
const byte BR_ADDR    [4] = { 10, 1, 2, 3 };
const byte BR_MASK    [4] = { 255, 0, 0, 0 };
const byte BR_GATEWAY [4] = { 10, 2, 2, 2 };
const byte BR_WAITER  [4] = { 10, 2, 2, 2 };
const byte BR_RESOLV  [4] = { 8, 8, 8, 8 };
#endif

#if LOCALNET
// For my emulator, which opens a port for TFTP services on localhost.
const byte BR_ADDR    [4] = { 127, 0, 0, 1 };
const byte BR_MASK    [4] = { 255, 0, 0, 0 };
const byte BR_GATEWAY [4] = { 127, 0, 0, 1 };
const byte BR_WAITER  [4] = { 127, 0, 0, 1 };
const byte BR_RESOLV  [4] = { 127, 0, 0, 1 };
#endif

// END

////////////////////////////////////////////////////////

#if EMULATED
#undef PrintH
void PrintH(const char* format, ...) {
  const char ** fmt_ptr = &format;
#ifdef __GNUC__
  asm volatile ("ldx %0\n  swi\n  fcb 111" : : "m" (fmt_ptr));
#else
  asm {
      ldx fmt_ptr
      swi
      fcb 111  // PrintH2 in emu/hyper.go
  }
#endif
}
#endif


////////////////////////////////////////////////////////

word StackPointer() {
  word result;
#ifdef __GNUC__
    asm ("tfr s,%0" : "=g" (result));
#else
    asm {
      tfr s,x
      stx result
    }
#endif
    return result;
}

char PolCat() {
  char inkey = 0;
#if !EMULATED
#ifdef __GNUC__
  asm volatile (R"(
    jsr [$A000]
    sta %0
  )" : "=m" (inkey) );
#else
  asm {
    jsr [$A000]
    sta :inkey
  }
#endif
#endif
  return inkey;
}

void Delay(word n) {
#if !EMULATED
  while (n--) {
#ifdef __GNUC__
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
    asm volatile ("mul" : : : "d", "b", "a");
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
#endif
}

void PutChar(char ch) {
#if EMULATED
  PrintH("CH: %x %c", ch, (' ' <= ch && ch <= '~') ? ch : '?');
  return;
#endif
    if (ch == 13) { // Carriage Return
      do {
        PutChar(' ');
      } while ((Vars->vdg_ptr & 31));
      return;
    }

    if (ch == 8) { // Backspace
      if (Vars->vdg_ptr > Vars->vdg_begin) {
        *(byte*)(Vars->vdg_ptr) = 32;
        -- Vars->vdg_ptr;
      }
      return;
    }

    if (ch < 32) return;  // Ignore other control chars.

    // Only use 64-char ASCII.
    if (96 <= ch && ch <= 126) ch -= 32;
    byte codepoint = (byte)ch;

    word p = Vars->vdg_ptr;
    *(byte*)p = (0x3f & codepoint);
    p++;

    if (p>=Vars->vdg_end) {
        for (word i = Vars->vdg_begin; i< Vars->vdg_end; i++) {
            if (i < Vars->vdg_end-32) {
                // Copy the line below.
                *(volatile byte*)i = *(volatile byte*)(i+32);
            } else {
                // Clear the last line.
                *(volatile byte*)i = 32;
            }
        }
        p = Vars->vdg_end-32;
    }
    *(byte*)p = 0xEF;  // display Blue Box cursor.
    Vars->vdg_ptr = p;
}

void PutStr(const char* s) {
    while (*s) PutChar(*s++);
}

void PutHex(word x) {
  if (x > 15u) {
    PutHex(x >> 4u);
  }
  PutChar( HexAlphabet[ 15u & x ] );
}
void PutDec(word x) {
  if (x > 9u) {
    PutDec(x / 10u);
  }
  PutChar('0' + (byte)(x % 10u));
}

void Fatal(const char* wut, word arg) {
    PutStr(" *FATAL* <");
    PutStr(wut);
    PutChar('$');
    PutHex(arg);
    PutStr("> ");
    while (1) continue;
}

void AssertEQ(word a, word b) {
  if (a != b) {
    PutHex(a);
    Fatal(":EQ", b);
  }
}

void AssertLE(word a, word b) {
  if (a > b) {
    PutHex(a);
    Fatal(":LE", b);
  }
}

void PrintF(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const char* s = format;
    while (*s) {
        if (*s == '%') {
            ++s;
            switch (*s) {
                case 'a': {  // "%a" -> IPv4 address as Dotted Quad.
                    byte* x;
                    x = va_arg(ap, byte*);
                    PutDec(x[0]);
                    PutChar('.');
                    PutDec(x[1]);
                    PutChar('.');
                    PutDec(x[2]);
                    PutChar('.');
                    PutDec(x[3]);
                }
                break;
                case 'x': {
                    word x;
                    x = va_arg(ap, word);
                    PutHex(x);
                }
                break;
                case 'u': {
                    word x = va_arg(ap, word);
                    PutDec(x);
                }
                break;
                case 's': {
                    const char* x = va_arg(ap, const char*);
                    PutStr(x);
                }
                break;
                default:
                    PutChar(*s);
            }
        } else {
            PutChar(*s);
        }
        s++;
    }
    va_end(ap);
    PutChar(';');
}

///////////////////////////////////////////////////////////

byte WizGet1(word reg) {
  WIZ->addr = reg;
  return WIZ->data;
}
word WizGet2(word reg) {
  WIZ->addr = reg;
  byte z_hi = WIZ->data;
  byte z_lo = WIZ->data;
  return ((word)(z_hi) << 8) + (word)z_lo;
}
void WizGetN(word reg, void* buffer, word size) {
  volatile struct wiz_port* wiz = WIZ;
  byte* to = (byte*) buffer;
  wiz->addr = reg;
   for (word i=size; i; i--) {
    *to++ = wiz->data;
  }
}
void WizPut1(word reg, byte value) {
  WIZ->addr = reg;
  WIZ->data = value;
}
void WizPut2(word reg, word value) {
  WIZ->addr = reg;
  WIZ->data = (byte)(value >> 8);
  WIZ->data = (byte)(value);
}
void WizPutN(word reg, const void* data, word size) {
  volatile struct wiz_port* wiz = WIZ;
  const byte* from = (const byte*) data;
  wiz->addr = reg;
  for (word i=size; i; i--) {
    wiz->data = *from++;
  }
}

// WizTicks: 0.1ms but may have readbyte,readbyte error?
word WizTicks() {
    return WizGet2(0x0082/*TCNTR Tick Counter*/);
}

void WizReset() {
  WIZ->command = 128; // Reset
  Delay(5000);
  WIZ->command = 3;   // IND=1 AutoIncr=1 BlockPingResponse=0 PPPoE=0
  Delay(100);
 
  // GLOBAL OPTIONS FOR SOCKETLESS AND ALL SOCKETS:

  // Interval until retry: 1 second.
  WizPut2(RTR0, 10000 /* Tenths of milliseconds. */ );
  // Number of retries.
  WizPut1(RCR, 10);
}

void WizConfigure(const byte* ip_addr, const byte* ip_mask, const byte* ip_gateway) {
  WizPutN(0x0001/*gateway*/, ip_gateway, 4);
  WizPutN(0x0005/*mask*/, ip_mask, 4);
  WizPutN(0x000f/*ip_addr*/, ip_addr, 4);

  // Create locally assigned mac_addr from ip_addr.
  Vars->mac_addr [0] = 2;
  Vars->mac_addr [1] = 2;
  memcpy(Vars->mac_addr+2, ip_addr, 4);
  WizPutN(0x0009/*ether_mac*/, Vars->mac_addr, 6);
  PrintF("Conf a=%a m=%a g=%a MAC=2.2.%a", ip_addr, ip_mask, ip_gateway, Vars->mac_addr+2);
}

/////////////////////////////////////////////////////////////////////////////////////

void WizIssueCommand(PARAM_SOCK_AND byte cmd) {
  WizPut1(B+SK_CR, cmd);
  while (WizGet1(B+SK_CR)) {
    ++ *(char*)0x401;
  }
  
  if (cmd == 0x40) PutChar('<');
  else if (cmd == 0x20) PutChar('>');
  else PutChar('!');
}

void WizWaitStatus(PARAM_SOCK_AND byte want) {
  byte status;
  byte stuck = 200;
  do {
    ++ *(char*)0x400;
    status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("W", status);
  } while (status != want);
}
