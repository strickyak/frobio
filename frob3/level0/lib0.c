#include "frob3/axiom/bootrom3.h"
#include "frob3/axiom/romapi3.h"
#define __GOMAR__ 1

#if NEED_STDLIB_IN_NETLIB3

////////////////////////////////////////////////////////
///
///  GCC Standard Library Routines.

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
  void* z = dest;
  while (*src) *dest++ = *src++;
  return z;
}

char *strncpy(char *restrict dest, const char *restrict src, size_t n) {
  void* z = dest;
  int i = 0;
  while (*src) {
    *dest++ = *src++;
    i++;
    if (i>=n) break;
  }
  return z;
}

size_t strlen(const char *s) {
  const char* p = s;
  while (*p) p++;
  return p-s;
}
#endif // NEED_STDLIB_IN_NETLIB3

////////////////////////////////////////////////////////
const byte HexAlphabet[] = "0123456789abcdef";

const struct sock Socks[4] = {
    { 0x400, 0x4000, 0x6000, 0 },
    { 0x500, 0x4800, 0x6800, 1 },
    { 0x600, 0x5000, 0x7000, 2 },
    { 0x700, 0x5800, 0x7800, 3 },
};

const struct proto TcpProto = {
  SK_MR_TCP+SK_MR_ND, // TCP Protocol mode with No Delayed Ack.
  SK_SR_INIT,
  false,
  SK_CR_SEND,
};

////////////////////////////////////////////////////////

#if __GOMAR__
#undef PrintH
void PrintH(const char* format, ...) {
  const char ** fmt_ptr = &format;
#ifdef __GNUC__
  asm volatile ("ldx %0\n  nop\n  fcb 33\n  fcb 111" : : "m" (fmt_ptr) : "x");
#else
  asm {
      ldx fmt_ptr
      nop      // Step 1: nop
      fcb 33   // Step 2: brn
      fcb 111  // PrintH2 in emu/hyper.go
  }
#endif
}
#endif


////////////////////////////////////////////////////////

word StackPointer() {
  word result;
#ifdef __GNUC__
    asm ("tfr s,%0" : "=r" (result));
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
#if !__GOMAR__
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
#if !__GOMAR__
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
#if __GOMAR__
  PrintH("CH: %x %c\n", ch, (' ' <= ch && ch <= '~') ? ch : '?');
  return;
#endif
    if (ch == 13 || ch == 10) { // Carriage Return
      do {
        PutChar(' ');
      } while ((Vars->vdg_ptr & 31));
      return;
    }

    word p = Vars->vdg_ptr;
    if (ch == 8) { // Backspace
      if (p > Vars->vdg_begin) {
        *(byte*)p = 32;
        --p;
      }
      goto END;
    }

    if (ch == 1) { // Hilite previous char.
      if (p > Vars->vdg_begin) {
        *(byte*)(p-1) ^= 0x40;  // toggle the inverse bit.
      }
      goto END;
    }

    if (ch < 32) return;  // Ignore other control chars.

    // Only use 64-char ASCII.
    if (96 <= ch && ch <= 126) ch -= 32;
    byte codepoint = (byte)ch;

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
END:
    *(byte*)p = 0xEF;  // display Blue Box cursor.
    Vars->vdg_ptr = p;
}

void PutStr(const char* s) {
    while (*s) PutChar(*s++);
}

void PutHexRecurse(word x) {
  if (x > 15u) {
    PutHexRecurse(x >> 4u);
  }
  PutChar( HexAlphabet[ 15u & x ] );
}
void PutHex(word x) {
  PutChar('$');
  PutHexRecurse(x);
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
                case 'd': {
                    int x = va_arg(ap, int);
                    if (x<0) {
                      PutChar('-');
                      x = -x;
                    }
                    PutDec((word)x);
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

/////////////////////////////////////////////////////////////////////////////////////

void WizIssueCommand(const struct sock* sockp, byte cmd) {
  WizPut1(B+SK_CR, cmd);
  while (WizGet1(B+SK_CR)) {
    // ++ *(char*)0x401;
  }
  // if (cmd == 0x40) PutChar('<');
  // else if (cmd == 0x20) PutChar('>');
  // else PutChar('!');
}

void WizWaitStatus(const struct sock* sockp, byte want) {
  byte status;
  byte stuck = 200;
  do {
    // ++ *(char*)0x400;
    status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("W", status);
  } while (status != want);
}

// Only called for TCP Client.
errnum WizCheck(PARAM_JUST_SOCK) {
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
  PrintH("WizCheck: B=%x SK_IR=%x ir=%x", B, SK_IR, ir);
      if (ir & SK_IR_TOUT) { // Timeout?
        PrintH(" WC:TImeout ");
        return SK_IR_TOUT;
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        PrintH(" WC:Disconn ");
        return SK_IR_DISC;
      }
      return OKAY;
}

errnum WizRecvGetBytesWaiting(const struct sock* sockp, word* bytes_waiting_out) {
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;

  *bytes_waiting_out = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
  return OKAY;
}

errnum WizRecvChunkTry(const struct sock* sockp, char* buf, size_t n) {
  word bytes_waiting = 0;
  errnum e = WizRecvGetBytesWaiting(SOCK_AND &bytes_waiting);
  if (e) return e;
  if( bytes_waiting < n) return NOTYET;

  word rd = WizGet2(B+SK_RX_RD0);
  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizGetN(R+begin, buf, first_n);
    WizGetN(R, buf+first_n, second_n);
  } else {
    WizGetN(R+begin, buf, n);
  }

  WizPut2(B+SK_RX_RD0, rd + n);
  WizIssueCommand(SOCK_AND  SK_CR_RECV); // Inform socket of changed SK_RX_RD.
  return OKAY;
}

errnum WizRecvChunk(const struct sock* sockp, char* buf, size_t n) {
  errnum e;
  do {
    e = WizRecvChunkTry(SOCK_AND buf, n);
  } while (e == NOTYET);
  return e;
}
errnum WizRecvChunkBytes(const struct sock* sockp, byte* buf, size_t n) {
  return WizRecvChunk(sockp, (char*)buf, n);
}
errnum TcpRecv(const struct sock* sockp, char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizRecvChunk(SOCK_AND  p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  return OKAY;
}
void WizReserveToSend(const struct sock* sockp,  size_t n) {
  PrintH("ResTS %x;", n);
  // Wait until free space is available.
  word free_size;
  do {
    free_size = WizGet2(B+SK_TX_FSR0);
    PrintH("Res free %x;", free_size);
  } while (free_size < n);

  struct sock_vars *sv = SV;
  sv->tx_ptr = WizGet2(B+SK_TX_WR0) & RING_MASK;
  sv->tx_to_go = n;
}

void WizBytesToSend(const struct sock* sockp, const byte* data, size_t n) {
  WizDataToSend(SOCK_AND (char*)data, n);
}
void WizDataToSend(const struct sock* sockp, const char* data, size_t n) {
  struct sock_vars *sv = SV;
  AssertLE(n, sv->tx_to_go);  // Must have already reserved.

  word begin = sv->tx_ptr;  // begin: Beneath RING_SIZE.
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  sv->tx_to_go -= n;
  sv->tx_ptr = (n + sv->tx_ptr) & RING_MASK;
}

void WizFinalizeSend(const struct sock* sockp, const struct proto *proto, size_t n) {
  struct sock_vars *sv = SV;
  word tx_wr = WizGet2(B+SK_TX_WR0);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  AssertEQ(sv->tx_to_go, 0);
  WizIssueCommand(SOCK_AND  proto->send_command);
}

errnum WizSendChunk(const struct sock* sockp,  const struct proto* proto, char* data, size_t n) {
  PrintH("Lib0WizSendChunk %x:%x %x %x %x %x...", n, data[0], data[1], data[2], data[3], data[4] );
  errnum e = WizCheck(JUST_SOCK);
  if (e) { PrintH("Lib0WizCheck-> ERR %x", e); }
  if (e) return e;
  WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND data, n);
  WizFinalizeSend(SOCK_AND proto, n);
  PrintH("Lib0WizSendChunk %x:%x Sent.", n, data[0]);
  return OKAY;
}
errnum TcpSend(const struct sock* sockp,  char* p, size_t n) {
  PrintH("TcpSend: sockp=%x p=%x n=%x", sockp, p, n);
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizSendChunk(SOCK_AND &TcpProto, p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  PrintH("TcpSend: Sent.");
  return OKAY;
}

const struct sock* SockNumber(byte i) {
  return Socks+i;
}
