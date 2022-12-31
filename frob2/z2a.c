#include "frob2/froblib.h"

extern void decb_putchar(int);
extern void decb_putstr(const char*);
extern char* decb_readline();

int b = 2000;
int main() {
  char* p;
  int a = 100;

  decb_putchar('!');
  decb_putchar(' ');
  decb_putstr("HELLO DECB\n");
  do {
    p = decb_readline();
    decb_putstr(p);
  } while (*p != 'q' && *p != 'Q');

#if 0
  LogStatus("ok a=%d b=%d sum=%d\n", a, b, a+b);
  LogStatus("ok main=%d a=%d b=%d\n", (int)main, (int)&a, (int)&b);
#endif
  return 0;
}

// buf.c
//
// Buffers for accumulating strings and list items.

#include "frob2/froblib.h"

//chop

void BufCheck(Buf *p) {
  Assert (p->n >= 0);
  Assert (p->s != NULL);
#ifdef unix
  {}
#else
  Assert (p->s <= (char*)0xC000);
#endif
};

void BufInit(Buf *p) {
  p->s = (char*) Malloc(BUF_INITIAL_CAP);
  p->n = 0;
}

void BufInitWith(Buf *p, const char *s) {
  p->s = StrDup((const byte*)s);
  p->n = strlen(s);
}

void BufInitTake(Buf *p, char *s) {
  p->s = s;
  p->n = strlen(s);
}

void BufDel(Buf *p) {
  // OK to delete more than once, or after BufTake().
  Free(p->s);
  p->s = NULL;
  p->n = -1;
}

char *BufFinish(Buf *p) {
  BufCheck(p);
  p->s = (char*) ReAlloc(p->s, p->n + 1);
  p->s[p->n] = '\0';
  return p->s;
}

char *BufTake(Buf *p) {
  BufCheck(p);
  char *z = p->s;
  p->s = NULL;
  p->n = -1;
  return z;
}

const char *BufPeek(Buf *p) {
  return p->s;
}

void BufAppC(Buf *p, char c) {
  BufCheck(p);
  ++p->n;
  p->s = (char*) ReAlloc(p->s, p->n);
  p->s[p->n - 1] = c;
}

void BufAppS(Buf *p, const char *s, int n) {
  BufCheck(p);
  if (n < 0)
    n = strlen(s);
  p->s = (char*) ReAlloc(p->s, p->n + n);
  char *t = p->s + p->n;
  for (int i = 0; i < n; i++) {
    *t++ = *s++;
  }
  p->n += n;
}
void BufAppStr(Buf *p, const char *s) {
    BufAppS(p, s, strlen(s));
}

// Appending Elements.

void BufAppElemC(Buf *p, char c) {
  BufCheck(p);
  if (c <= ' ' || c > 'z' || c == '\\') {
    p->n += 2;
    p->s = (char*) ReAlloc(p->s, p->n);
    p->s[p->n - 2] = '\\';
    p->s[p->n - 1] = c;
  } else {
    p->n += 1;
    p->s = (char*) ReAlloc(p->s, p->n);
    p->s[p->n - 1] = c;
  }
}

void BufAppElemS(Buf *p, const char *s) {
  BufCheck(p);

  // Add space before next element, unless buf is empty.
  if (p->n)
    BufAppC(p, ' ');

  byte clean = true;
  for (const char *t = s; *t; t++) {
    if (*t <= ' ' || *t > 'z' || *t == '\\') {
      clean = false;
      break;
    }
  }
  if (clean && *s) {            // empty strings should not be clean.
    for (const char *t = s; *t; t++) {
      BufAppC(p, *t);
    }
  } else {
    BufAppC(p, '{');
    while (*s) {
      BufAppElemC(p, *s);
      s++;
    }
    BufAppC(p, '}');
  }
}


void BufAppDope(Buf *p, const char *s) {
  // (word) cast: Div optimizes to shift, if unsigned.
  int n = (word)p->n / sizeof(const char *);
  BufAppS(p, "        ", sizeof(const char *));
  ((const char **) p->s)[n] = s;
}

const char **BufTakeDope(Buf *p, int *lenP) {
  // (word) cast: Div optimizes to shift, if unsigned.
  *lenP = (word)p->n / sizeof(const char *);
  return (const char **) BufTake(p);
}

// Decoding elements.

// Return length of decoded element.
// Also the end of parsing.
int ElemLen(const char *s, const char **endP) {
  int n = 0;
  if (*s == '{') {
    // brace-wrapped element.
    s++;
    while (*s) {
      if (*s == '}') {
        s++;
        break;
      }
      if (*s == '\\') {
        ++s;                    // extra to jump over the backslash.
        ++n;
      }
      ++s;
      ++n;
    }
    // Warning: truncated element.
    *endP = s;
  } else {
    // bare element.
    const char *t = s;
    while (*t > ' ')
      ++t;
    *endP = t;
    n = t - s;
  }
  return n;
}

// Return decoded element.
const char *ElemDecode(const char *s) {
  Buf buf;
  BufInit(&buf);
  if (*s == '{') {
    // brace-wrapped element.
    s++;
    while (*s) {
      if (*s == '}') {
        s++;
        break;
      }
      if (*s == '\\') {
        ++s;                    // extra to jump over the backslash.
      }
      BufAppC(&buf, *s);
      ++s;
    }
    // Warning: truncated element.
  } else {
    // bare element.
    const char *t = s;
    while (*t > ' ')
      ++t;
    BufAppS(&buf, s, t - s);
  }
  BufFinish(&buf);
  return BufTake(&buf);
}
// ny: NitroYak: Yak Libs for NitrOS9

#include "frob2/froblib.h"

#define ARGC (*p_argc)
#define ARGV (*p_argv)
#define SKIP (--ARGC, ++ARGV)

//chop

char FlagChar;
const char *FlagArg;

//chop

// Call SkipArg() once before using GetFlag,
// to skip the unused argv0 cell.
void SkipArg(int* p_argc, char*** p_argv) {
    SKIP;
}

// Expects argc, argv to have had the initial argv0 removed (see SkipArg).
// When called, the next option will be found in arg[0].
// Recognizes "--" as a separator at the end of options.
// Recognizes "-" as not an option.
//
// Returns false if no more options.
// The argc non-option args are left starting at argv[0].
//
// Returns true if it consumed an option, or found an error:
//   Sets FlagChar to the option char, or to 1 if error.
//   Sets FlagArg if there is an argument, or to NULL.
bool GetFlag(int* p_argc, char*** p_argv, const char* flagDesc) {
    FlagChar = 0;
    FlagArg = NULL;
    if (!ARGC) return false;
    const char* s = ARGV[0];

    if (s[0] != '-') return false;
    if (!strcmp(s, "-")) return false;
    if (!strcmp(s, "--")) {
        SKIP;
        return false;
    }

    FlagChar = CharDown(s[1]);  // The option char.
    
    // Search for the FlagChar in the flagDesc.
    const char* desc = flagDesc;
    for (; *desc; desc++) if (*desc == FlagChar) break;

    // If option not found, call usage_fn and Fatal.
    if (!desc[0]) {
        SKIP;
        LogError("Bad FlagChar: -%c", FlagChar);
        FlagChar = '?';  // '?' means usage error.
        return true;
    }

    if (desc[1] == ':') {
        // If option takes an argument
        if (s[2]) {
            FlagArg = s+2;
        } else {
            SKIP;
            if (!ARGC) {
                LogError("Missing arg for option: -%c", FlagChar);
                FlagChar = 1;  // 1 means usage error.
                return true;
            }
            FlagArg = ARGV[0];
        }
    } else {
        if (s[2]) {
            LogError("arg not allowed after option: %s", s);
            FlagChar = 1;  // 1 means usage error.
            return true;
        }
    }
    SKIP;
    return true;
}
#include "frob2/froblib.h"
#include "frob2/frobos9.h"

#define debug_printf if(false)printf

void staticBufFillGap(Buf* buf, word width, word n, bool fill0);
byte* staticQFormatUnsignedLong(byte* p, unsigned long x);
byte* staticQFormatSignedLong(byte* p, signed long x);
byte* staticQFormatLongHex(byte* p, const byte* alphabet, unsigned long x);

//chop

byte ShortStaticBuffer[24];

//chop

// CharUp(c): convert to upper case for 26 ascii letters.
char CharUp(char c) {
  return ('a' <= c && c <= 'z') ? c - 32 : c;
}

// CharDown(c): convert to lower case for 26 ascii letters.
char CharDown(char c) {
  return ('A' <= c && c <= 'Z') ? c + 32 : c;
}

#if 0  // FOR CURLY
void BPutNumCurly(Buf* buf, byte c) {
          BPutChar(buf, '{');
          BPutU(buf, (word)c);
          BPutChar(buf, '}');
}//


void BEncodeCurly(Buf* buf, byte* str, int n) {
  BPutChar(buf, '"');
  for (int i = 0; i < n; i++) {
    byte c = str[i];
    if (32 <= c && c <= 127) {
      switch (c) {
        case '"':
        case '\'':
        case '\\':
        case '{':
        case '}':
          BPutNumCurly(buf, c);
            break;
        default:
          BPutChar(buf, c);
      }
    } else {
          BPutNumCurly(buf, c);
    }
  }
  BPutChar(buf, '"');
}//
#endif

// Hex Alphabets
const byte* LowerHexAlphabet = "0123456789abcdef";
const byte* UpperHexAlphabet = "0123456789ABCDEF";
//chop

void staticBufFillGap(Buf* buf, word width, word n, bool fill0) {
    if (width > n) {
        word gap = width - n;
        for (byte i = 0; i < gap; i++) {
            BufAppC(buf, fill0? '0' : ' ');
        }
    }
}

void BufAppStringQuoting(Buf* buf, const char* s, word precision) {
  char x;
  for (word i = 0; i<precision; i++) {
      x = s[i];
      switch (x) {
         case 9:
            x = 't';
         goto escaped_x;
         case 10:
            x = 'n';
         goto escaped_x;
         case 13:
            x = 'r';
         goto escaped_x;
         case '\"':
            x = '\"';
         goto escaped_x;
         case '\'':
            x = '\'';
         goto escaped_x;
         case '\\':
            x = '\\';
escaped_x:
            BufAppC(buf, '\\');
            BufAppC(buf, x);
         break;
         default:
              if (' ' <= x && x <= '~') {
                // "Printable" ASCII
                BufAppC(buf, x);
              } else {
                // Needs hex escape
                BufAppC(buf, '\\');
                BufAppC(buf, 'x');
                BufAppC(buf, LowerHexAlphabet[(byte)x>>4]);
                BufAppC(buf, LowerHexAlphabet[(byte)x&15]);
              }
      }  // switch
  }  // next i
}

#if 0
static byte* QFormatUnsignedInt(byte* p, unsigned int x) {
  if (x > 9) {
    p = QFormatUnsignedInt(p, x / 10);
    *p++ = '0' + (byte)(x % 10);
  } else {
    *p++ = '0' + (byte)x;
  }
  return (*p = 0), p;
}//
static byte* QFormatSignedInt(byte* p, signed int x) {
  if (x<0) {
    *p++ = '-';
    p = QFormatUnsignedInt(p, (unsigned int)-x);
  } else {
    p = QFormatUnsignedInt(p, (unsigned int)x);
  }
  return p;
}//
const char* StaticFormatSignedInt(int x) {
    byte* p = QFormatSignedInt(ShortStaticBuffer, x);
    *p = 0;
    return (const char*)ShortStaticBuffer;
}//
#endif

byte* staticQFormatUnsignedLong(byte* p, unsigned long x) {
  if (x > 9) {
    p = staticQFormatUnsignedLong(p, x / 10);
    *p++ = '0' + (byte)(x % 10);
  } else {
    *p++ = '0' + (byte)x;
  }
  return (*p = 0), p;
}
byte* staticQFormatSignedLong(byte* p, signed long x) {
  if (x<0) {
    *p++ = '-';
    p = staticQFormatUnsignedLong(p, (unsigned long)-x);
  } else {
    p = staticQFormatUnsignedLong(p, (unsigned long)x);
  }
  return p;
}
const char* StaticFormatSignedLong(int x) {
    byte* p = staticQFormatSignedLong(ShortStaticBuffer, x);
    *p = 0;
    return (const char*)ShortStaticBuffer;
}

byte* staticQFormatLongHex(byte* p, const byte* alphabet, unsigned long x) {
  if (x > 15) {
    p = staticQFormatLongHex(p, alphabet, x >> 4);
    // TODO: report bug that (byte)x did not work.
    *p++ = alphabet[ (byte)(word)x & (byte)15 ];
  } else {
    *p++ = alphabet[ (byte)x ];
  }
  return (*p = 0), p;
}


void BufFormatVA(Buf* buf, const char* format, va_list ap) {
    // Quick Buffer for integer formatting.
    byte qbuf[24]; // TODO use ShortStaticBuffer.

    for (const char* s = format; *s; s++) {
        if (*s != '%') {
            BufAppC(buf, *s);
            continue;
        }

        s++;
        bool fill0 = false, longingly = false;
        word width = 0;
        word precision = 0;
        bool use_precision = 0;
        if (*s == '0') {
            fill0 = true;
            s++;
        }
        while ('0'<=*s && *s<='9') {
          if (use_precision) {
            precision = 10*precision + (*s - '0');
          } else {
            width = 10*width + (*s - '0');
          }
          s++;
        }

       // TODO put all this, and 0-9, in a while.
        if (*s == '.') {
            use_precision = true;
            s++;
        }
        if (*s == '*') {
            int x = va_arg(ap, int);
            if (use_precision) {
                precision = x;
            } else {
                width = x;
            }
            s++;
        }
        if (*s == 'l') {
            longingly = true;
            s++;
        }


        switch (*s) {
        case 'X':
        case 'x': {
            unsigned long x;
            if (longingly) {
                x = va_arg(ap, unsigned long);
            } else {
                x = va_arg(ap, unsigned int);
            }
            
            byte n = (byte)(staticQFormatLongHex(qbuf, ((*s=='X')? UpperHexAlphabet: LowerHexAlphabet), x) - qbuf);
            staticBufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 'u': {
            unsigned long x;
            if (longingly) {
                x = va_arg(ap, unsigned long);
            } else {
                x = va_arg(ap, unsigned int);
            }
            
            byte n = (byte)(staticQFormatUnsignedLong(qbuf, x) - qbuf);
            staticBufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 'd': {
            signed long x;
            if (longingly) {
                x = va_arg(ap, signed long);
            } else {
                x = va_arg(ap, signed int);
            }
            
            byte n = (byte)(staticQFormatSignedLong(qbuf, x) - qbuf);
            staticBufFillGap(buf, width, n, fill0);
            BufAppS(buf, (const char*)qbuf, n);
        }
        break;
        case 's': {
            const char* x = va_arg(ap, const char*);
            if (!x) x = "<NULL>";
            //debug_printf("arg(s)%s ", x);
            precision = (precision == 0) ? strlen(x) : precision;
            staticBufFillGap(buf, width, precision, fill0);
            BufAppS(buf, x, precision);
        }
        break;
        case 'q': {
            const char* x = va_arg(ap, const char*);
            if (!x) x = "<NULL>";
            //debug_printf("arg(s)%s ", x);
            precision = (precision == 0) ? strlen(x) : precision;
            BufAppC(buf, '\"');
            BufAppStringQuoting(buf, x, precision);
            BufAppC(buf, '\"');
        }
        break;
        case 'c': {
            char ch = (char) va_arg(ap, int);
            if (' ' <= ch && ch <= '~') {
                BufAppC(buf, ch);
            } else {
                BufAppC(buf, '<');
                byte n = (byte)(staticQFormatUnsignedLong(qbuf, (byte)ch) - qbuf);
                BufAppS(buf, (const char*)qbuf, n);
                BufAppC(buf, '>');
            }
        }
        break;
        default:
            BufAppC(buf, *s);
        }
    }
}
void BufFormat(Buf* buf, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    BufFormatVA(buf, format, ap);
    va_end(ap);
}

// returns OKAY or ErrNo.
errnum WritLnAll(int path, const char* s, word n) {
    ErrNo = OKAY;
    word n0 = n;
    while (n>0) {
        int wrote = 0;
#ifdef unix
        wrote = write(path, s, n);
        ErrNo = wrote < 1 ? errno : 0;
#else
        ErrNo = Os9WritLn(path, s, n, &wrote);
#endif
        if (ErrNo) return ErrNo;
        s += wrote;
        n -= wrote;
    }
    return 0;
}

// return bytes_written or -1.
int Printf(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = buf.n;
    ErrNo = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) ErrNo = errno;
    #else
    WritLnAll(1, buf.s, buf.n);
    #endif
    BufDel(&buf);
    return (ErrNo) ? -1 : bytes_written;
}

int EPrintf(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = buf.n;
    errnum e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    WritLnAll(2, buf.s, buf.n);
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int FPrintf(File* f, const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    BufFinish(&buf);
    int bytes_written = buf.n;
    errnum e = 0;
    #ifdef unix
    bytes_written = write(1, buf.s, buf.n);
    if (bytes_written <= 0) e = errno;
    #else
    WritLnAll(f->fd, buf.s, buf.n);
    #endif
    BufDel(&buf);
    return (e) ? -1 : bytes_written;
}

int SPrintf(char* dest, const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    word n = buf.n;
    BufFinish(&buf);
    memcpy(dest, buf.s, n+1);
    BufDel(&buf);
    return n;
}

char* StrFormat(const char* fmt, ...) {
    Buf buf;
    BufInit(&buf);

    va_list ap;
    va_start(ap, fmt);
    BufFormatVA(&buf, fmt, ap);
    va_end(ap);

    return BufFinish(&buf);
}

// Debugging routines should not call Buf or Malloc.
void PutHex(byte c, word w) {
  // const byte* UpperHexAlphabet = "0123456789ABCDEF";
  // byte* staticQFormatLongHex(byte* p, const byte* alphabet, unsigned long x);
  byte* p = (byte*) ShortStaticBuffer;
  *p++ = '[';
  *p++ = c;
  *p++ = ':';
  p = staticQFormatLongHex(p, UpperHexAlphabet, w);
  *p++ = ']';
  *p = '0';
  FPuts((char*)ShortStaticBuffer, StdErr);
}

void Panic(const char* message) {
  FPuts("\n*** PANIC: ", StdErr);
  FPuts(message, StdErr);
  FPuts("\n", StdErr);
}

void PC_Trace(byte c, const void* w) {
  PutHex(c, (word)w); // TODO
}
// Malloc & Free, by Strick.
//
// Raise requested malloc size to the next power of two.
// There is a free list for each power of two.
// So if random sizes are used, we waste perhaps 25%.

#include "frob2/froblib.h"
#include "frob2/froblib/malloc.h"
#include "frob2/frobos9.h"

#define ZERO_MALLOC             // catch bugs faster.
#define ZERO_FREE               // catch bugs faster.
// #define AUDIT_MALLOC_FREE  // for leak and unmatched malloc/free detection.

extern word heap_min;
extern word heap_here;
extern word heap_max;
extern bool heap_retry;  // Not thread-safe.

extern struct MallocHead *buck_freelist[NBUCKETS];
extern int buck_num_alloc[NBUCKETS];
extern int buck_num_free[NBUCKETS];
extern int buck_num_brk[NBUCKETS];

#ifdef unix
extern byte MemoryPoolForUnix[1024000];
#endif

// forward
extern void MallocOOM(errnum e, word n, word cap);
extern byte which_bucket(int n, int *capP);
extern void heap_check_block(struct MallocHead *h, int cap);

// chop

// Heap boundaries.
word heap_min;
word heap_here;
word heap_max;
bool heap_retry;  // Not thread-safe.

struct MallocHead *buck_freelist[NBUCKETS];
int buck_num_alloc[NBUCKETS];
int buck_num_free[NBUCKETS];
int buck_num_brk[NBUCKETS];

#ifdef unix
byte MemoryPoolForUnix[1024000];
#endif

void MallocOOM(errnum e, word n, word cap) {
    PutHex('e', e);
    PutHex('n', n);
    PutHex('c', cap);
    PutHex('u', heap_here);
    PutHex('m', heap_max);
    Panic(" *oom* ");
}

void heap_check_block(struct MallocHead *h, int cap) {
  // PC_Trace('?', (char*)h);
  if (h->barrierA != 'A' || h->barrierZ != 'Z' || (cap && h->cap != cap)) {
    PutHex('h', (word)h);
    PutHex('A', h->barrierA);
    PutHex('Z', h->barrierZ);
    PutHex('c', h->cap);
    PutHex('C', cap);
    PC_Trace('*', (char *) h);
    Panic("corrupt heap");
  }
}

byte which_bucket(int n, int *capP) {
  byte b;
  int cap = SMALLEST_BUCKET;
  for (b = 0; b < NBUCKETS; b++) {
    if (n <= cap)
      break;
    cap += cap;
  }
  if (b >= NBUCKETS) {
    PutHex('m', n);
    Panic("malloc too big");
  }
  *capP = cap;
  return b;
}

#if 0
void ShowChains() {
  for (byte b = 0; b < NBUCKETS; b++) {
    printf_d("Bucket [%d]: ", b);
    for (struct MallocHead * p = buck_freelist[b]; p; p = p->next) {
      PutHex('=', p);
    }
    puts("\r");
  }
  puts("\r");
}//
#endif

void *Malloc(word n) {
  if (!heap_max) {
  //printf("m:first:%d\n", __LINE__);
#ifdef unix
      // byte MemoryPoolForUnix[102400];
       heap_min = (word)MemoryPoolForUnix;
       heap_here = (word)MemoryPoolForUnix;
       heap_max = (word)MemoryPoolForUnix + sizeof MemoryPoolForUnix;
#else
       word new_memory_size = 0;
       word end_of_new_mem = 0;
       errnum err = Os9Mem(&new_memory_size, &end_of_new_mem);
       if (err) MallocOOM(err, n, 0);
       //printf(" *** 1st Os9Mem e=%x => %x %x\n", err, new_memory_size, end_of_new_mem);
       heap_min = end_of_new_mem;
       heap_here = end_of_new_mem;

       // round up to multiple of 0x2000, and get approval for that.
       // HARDWIRED CONSTANTS FOR COCO3/MOOH 8K MEMORY PAGES.
       new_memory_size = (end_of_new_mem + 0x2000) & 0xE000;
       err = Os9Mem(&new_memory_size, &end_of_new_mem);
       //printf(" *** 2nd Os9Mem e=%x => %x %x\n", err, new_memory_size, end_of_new_mem);
       if (err) MallocOOM(err, n, 0);
       heap_max = end_of_new_mem;
#endif
       Assert(heap_max >= heap_min);
  }
  //printf("m:%d:%x,%x,%x\n", __LINE__, heap_min, heap_here, heap_max);
  int cap;
  byte b = which_bucket(n, &cap);
  //printf("m:%d:n=%d,b=%d\n", __LINE__, n, b);
  buck_num_alloc[b]++;

  // Try an existing unused block.

  struct MallocHead *h = buck_freelist[b];
  if (h) {
    h->cap = cap;
    heap_check_block(h, cap);
    buck_freelist[b] = h->next;
#ifdef ZERO_MALLOC
    memset((char *) (h + 1), 0, cap);
#endif
#ifdef AUDIT_MALLOC_FREE
    PC_Trace('M', (char *) h);
#endif
    return (char *) (h + 1);
  }

  // Break fresh memory.
  char *p = (char *) heap_here;
  word new_brk = heap_here + (word) (cap + sizeof(struct MallocHead));
  if (new_brk > heap_max) {
    if (heap_retry) {
        MallocOOM(255, n, cap); // Double fault.
    }

#ifdef unix
    Assert(!"TODO: Use sbrk on unix and OS9");
#else
    // Add another 0x2000 page.
    word new_memory_size = heap_max + 0x2000;
    word end_of_new_mem = 0;
    errnum err = Os9Mem(&new_memory_size, &end_of_new_mem);
    //printf(" *** Os9Mem e=%x => %x %x\n", err, new_memory_size, end_of_new_mem);
    if (err) MallocOOM(err, n, cap);
    heap_max = new_memory_size;
#endif

    heap_retry = true;
    p = (char*)Malloc(n);
    heap_retry = false;
    return p;
  }

  heap_here = new_brk;
  buck_num_brk[b]++;

  h = ((struct MallocHead *) p);
  h->barrierA = 'A';
  h->barrierZ = 'Z';
  h->cap = cap;
  h->next = NULL;
#ifdef ZERO_MALLOC
  memset((char *) (h + 1), 0, cap);
#endif
#ifdef AUDIT_MALLOC_FREE
  PC_Trace('M', (char *) h);
#endif
  return (char *) (h + 1);
}

void Free(void *p) {
  if (!p)
    return;

  struct MallocHead *h = ((struct MallocHead *) p) - 1;
  if (!h->cap) {                // TODO -- because double-frees.
    Panic("DoubleFree");
    return;
  }
  int cap;
  byte b = which_bucket(h->cap, &cap);
  heap_check_block(h, cap);
  buck_num_free[b]++;

#ifdef ZERO_FREE
  memset((char *) p, 0, cap);
#endif
  h->cap = 0;                   // TODO -- because double-frees.
  h->next = buck_freelist[b];
  buck_freelist[b] = h;
#ifdef AUDIT_MALLOC_FREE
  PC_Trace('F', (char *) h);
#endif
}

void *ReAlloc(void *p, word n) {
  if (!p)
    return Malloc(n);
  struct MallocHead *h = ((struct MallocHead *) p) - 1;
  if (n <= h->cap) {
    return (char *) p;
  }

  char *z = (char*)Malloc(n);
  memcpy(z, p, h->cap);
  Free(p);
  return z;
}
// ny: NitroYak: Yak Libs for NitrOS9

#include "frob2/froblib.h"

// Hereafter, pp will point to a pointer to const char,
// and will be an argument to all functions.
// In the body, P will act like a normal pointer, using (*pp).
#define P (*pp)

//chop
byte Verbosity = LLInfo;
//chop
byte ErrNo;
//chop

Buf LogBuf;
void Logger(const char* file, word line, byte level, const char* fmt, ...) {
    extern byte disable_irq_count;

    // fmt may be NULL just to flush when irqs are re-enabled.
    if (fmt) {
        if (!LogBuf.s) {
            // First time, allocate.
            LogBuf.s = (char*)Malloc(128);
        }
        if (level > Verbosity) return;
        if (level > MAX_VERBOSE) return;

        if (disable_irq_count) {
            if (Verbosity > LLError) {
                BufAppC(&LogBuf, '~');  // non-error just logs '~' during disabled irqs.
                return;
            }
        }

        BufFormat(&LogBuf, "%d:%s:%d ", level, file, line);
        if (level == LLFatal) {
            BufAppStr(&LogBuf, "FATAL: ");
        }

        va_list ap;
        va_start(ap, fmt);
        BufFormatVA(&LogBuf, fmt, ap);
        va_end(ap);
    }

    // Never flush to stderr while irqs are disabled.
    if (disable_irq_count) return;
   
    // Skip writing if empty. 
    if (!LogBuf.n) return;

    // Append \n if it is not already there.
    if (LogBuf.s[LogBuf.n-1] != '\n') {
        BufAppC(&LogBuf, '\n');
    }

    // Finish, Print, and Reset the buffer.
    char* s = BufFinish(&LogBuf);
    FPuts(s, StdErr);
    if (level == LLFatal) {
        exit(127);
    }
    LogBuf.n = 0;
}


bool NyWhite(char c) {
  return (byte)c <= ' ';
}

int NySplit(char* s, char* *words_out, int max_words) {
  int count = 0;
  while (count < max_words) {
    while (*s && NyWhite(*s)) s++; // skip white space.
    if (!*s) break;
    *words_out++ = s;
    ++count;
    while (*s && !NyWhite(*s)) s++; // skip non-white.
    if (!*s) break;
    *s++ = '\0';
  }
  return count;
}

bool NyStrEq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) return 0;
    a++;
    b++;
  }
  return (*a == *b);
}

bool NyCharEqIgnoreCase(char x, char y) {
  if ('a' <= x && x <= 'z') x -= 32;
  if ('a' <= y && y <= 'z') y -= 32;
  return x == y;
}

bool NyStrEqIgnoreCase(const char* a, const char* b) {
  while (*a && *b) {
    if (!NyCharEqIgnoreCase(*a, *b)) return 0;
    a++;
    b++;
  }
  return NyCharEqIgnoreCase(*a, *b);
}

////////////// Parsing

byte NyDeHex(byte ch) {
  if ('0' <= ch && ch <= '9') {
    return ch - '0';
  }
  if ('A' <= ch && ch <= 'Z') {
    return ch - 'A' + 10;
  }
  if ('a' <= ch && ch <= 'z') {
    return ch - 'a' + 10;
  }
  Assert(0);
}

void NyFormatDottedDecimalQuad(char* buffer, quad addr) {
    byte* p = (byte*)&addr;
    SPrintf(buffer, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

void NySkipSpaces(const char** pp) {
  while (*P == ' ') {
    P++;
  }
}
byte NyParseChar(const char** pp) { // must be the current char
  return NyDeHex(*P++);
}
byte NyParseHexByte(const char** pp) { // must be 2 chars
  byte a = NyParseChar(pp);
  byte b = NyParseChar(pp);
  return (a << 4) | b;
}
word NyParseHexWord(const char** pp) { // must be 4 chars
  byte a = NyParseHexByte(pp);
  byte b = NyParseHexByte(pp);
  return ((word)a << 8) | b;
}

word NyParseDecimalWord(const char** pp) {
  word z = 0;
  while (*P) {
    if ('0' <= *P && *P <= '9') {
      z = 10*z + (*P - '0');
    } else {
      break;
    }
    P++;
  }
  return z;
}

byte NyCheckByte(word a) {
  if (a > 255) {
    LogFatal("Byte should be less than 256: %d", a);
  }
  return (byte)a;
}

quad NyFormQuadFromBytes(byte a, byte b, byte c, byte d) {
  quad z;
  byte* p = (byte*)&z;
  p[0]=a; p[1]=b; p[2]=c; p[3]=d;
  return z;
}

quad NyParseDottedDecimalQuad(const char** pp) {
  const char* orig = P;
  word a = NyParseDecimalWord(pp);
  if (*P != '.') goto no_dot;
  P++;
  word b = NyParseDecimalWord(pp);
  if (*P != '.') goto no_dot;
  P++;
  word c = NyParseDecimalWord(pp);
  if (*P != '.') goto no_dot;
  P++;
  word d = NyParseDecimalWord(pp);

  return NyFormQuadFromBytes(
      NyCheckByte(a), NyCheckByte(b), NyCheckByte(c), NyCheckByte(d));

no_dot:
  LogFatal("missing dot in dotted quad: %s", orig);
  return 0;
}

// On call: default port is in *port.
// On return: *port may be replaced with specified port.
quad NyParseDottedDecimalQuadAndPort(const char** pp, word* port) {
  quad addy = NyParseDottedDecimalQuad(pp);
  if (*P == ':') {
    P++;
    *port = NyParseDecimalWord(pp);
  }
  return addy;
}


size_t StrLen(const byte* s) {
    return strlen((const char*)s);
}
void StrCat(byte*a, const byte*b) {
    strcat((char*)a, (const char*)b);
}


bool StrEq(const byte*a, const byte*b) {
    return 0 == strcmp((const char*)a, (const char*)b);
}
bool StrNEq(const byte*a, const byte*b, word n) {
    return 0 != strcmp((const char*)a, (const char*)b);
}
bool StrCaseEq(const byte*a, const byte*b) {
    return 0 == strcasecmp((const char*)a, (const char*)b);
}
#if 0
bool StrNCaseEq(const byte*a, const byte*b, word n) {
    return 0 != strncasecmp((const char*)a, (const char*)b, n);
} //
#endif
#include "frob2/froblib.h"
#include "frob2/frobos9.h"

// forward
extern File StdIn_File;
extern File StdOut_File;
extern File StdErr_File;

//chop
File StdIn_File = {0};
//chop
File StdOut_File = {1};
//chop
File StdErr_File = {2};
//chop

// FOpen returns NULL and sets ErrNo on error.
File* FOpen(const char* pathname, const char* mode) {
    Assert(pathname);
    Assert(mode);
    Assert(mode[1]=='\0' && (mode[0]=='r'||mode[0]=='w'));
    int fd = -1;

    if (mode[0] == 'r') {
      ErrNo = Os9Open(pathname, 1/*=READ*/, &fd);
      if (ErrNo) return NULL;
    } else if (mode[0] == 'w') {
      Os9Delete(pathname);
      ErrNo = Os9Create(pathname, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
      if (ErrNo) return NULL;
    }
    Assert (fd>=0);

    File* f = (File*) Malloc(sizeof *f);
    f->fd = fd;
    return f;
}

// FOpen returns bytes_read, or 0 and sets ErrNo on error.
word FGets(char *buf, int size, File *f) {
    Assert(buf);
    Assert(size);
    Assert(f);
    int bytes_read = 0;
    ErrNo = Os9ReadLn(f->fd, buf, size-1, &bytes_read);
    if (ErrNo) return 0;
    buf[bytes_read] = '\0';
    return bytes_read;
}

// returns length of str, or -1 on error.
int FPuts(const char *str, File *f) {
    Assert(str);
    Assert(f);
    int n = strlen(str);
    ErrNo = WritLnAll(f->fd, str, n);
    return (ErrNo) ? -1 : n;
}

// returns 0, or -1 on error.
int FClose(File *f) {
    Assert(f);
    ErrNo = Os9Close(f->fd);
    if (ErrNo) return -1;
    f->fd = -1;
    Free(f);
    return 0;
}

void PError(const char* str) {
    int e = ErrNo;
    ErrNo = OKAY;
    Assert(str);
    FPuts(" ", StdErr);
    FPuts(str, StdErr);
    char ebuf[4];
    ebuf[0] = (char) (e / 100 + '0');
    e = e % 100;
    ebuf[1] = (char) (e / 10 + '0');
    e = e % 10;
    ebuf[2] = (char) (e + '0');
    ebuf[3] = '\0';
    FPuts(": ErrNo ", StdErr);
    FPuts(ebuf, StdErr);
    FPuts(".\n", StdErr);
}
#include "frob2/froblib.h"
#include "frob2/frobos9.h"

//chop

// prefixed_atoi() understands initial "0" for octal and "0x" for hex, and '-' before any of that.
int prefixed_atoi(const char *s) {
  int z = 0;
  byte neg = false;
  if (*s == '-') {
    neg = 1;
    s++;
  }
  if (*s == '0') {
    s++;
    if (*s == 'x') {
      // hex if starts 0x
      while ('0' <= *s && *s <= '9' || 'A' <= CharUp(*s) && CharUp(*s) <= 'F') {
        if ('0' <= *s && *s <= '9') {
          z = z * 16 + (*s - '0');
        } else {
          z = z * 16 + (CharUp(*s) + 10 - 'A');
        }
        s++;
      }
    } else {
      // octal if starts 0
      while ('0' <= *s && *s <= '7') {
        z = z * 8 + (*s - '0');
        s++;
      }
    }
  } else {
    // else decimal
    while ('0' <= *s && *s <= '9') {
      z = z * 10 + (*s - '0');
      s++;
    }
  }
  return neg ? -z : z;
}

int strcasecmp(const char *a, const char *b) {
  while (*a && *b) {
    if ((byte) CharUp(*a) < (byte) CharUp(*b))
      return -1;
    if ((byte) CharUp(*a) > (byte) CharUp(*b))
      return +1;
    a++;
    b++;
  }
  // at least one is 0.
  if (*a)
    return -1;
  if (*b)
    return +1;
  return 0;
}

char *strdup(const char *s) {
  int n = strlen(s);
  char *p = (char *) Malloc(n + 1);
  strcpy(p, s);
  return p;
}

mstring StrDup(const byte* p) {
    return strdup((const char*)p);
}

char *strndup(const char *s, size_t n) {
  char *p = (char *) Malloc(n + 1);
  strncpy(p, s, n+1);
  return p;
}
#include "frob2/froblib.h"
#include "frob2/frobos9.h"
#include "frob2/decb/std4gcc.h"

/* we need
EnableIrqsCounting
DisableIrqsCounting
Os9Open
Os9Close
Os9ReadLn
Os9WritLn
Os9Mem
Os9Delete
*/

byte disable_irq_count;

void DisableIrqsCounting() {}
void EnableIrqsCounting() {}

void STOP() {
    decb_putstr(" [STOP]");
    while (1) {
      disable_irq_count = disable_irq_count;
    }
}

IF_os9_THEN_asm void Os9Exit(byte status) {
    decb_putstr(" [EXIT]"); STOP();
}
IF_os9_THEN_asm errnum Os9Create(const char* path, int mode, int attrs, int* fd) {
    decb_putstr(" [Os9Create]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9Open(const char* path, int mode, int* fd) {
    decb_putstr(" [Os9Open]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9Delete(const char* path) {
    decb_putstr(" [Os9Delete]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read) {
    decb_putstr(" [Os9ReadLn]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written) {
    decb_putstr("{");
    int i;
    for (i = 0; i < max; i++) {
      if (buf[i]==0) break;
      decb_putchar(buf[i]);
      if (buf[i]==10 || buf[i]==13) { i++; break; }
    }
    decb_putstr("}");
    *bytes_written = i;
    return OKAY;
}
IF_os9_THEN_asm errnum Os9Close(int path) {
    decb_putstr(" [Os9Close]"); STOP(); return 68;
}

#define MAX_DECB_MEMORY_SIZE 0x8000
word decb_memory_size = 0x6000;
errnum Os9Mem(word* new_memory_size_inout, word* end_of_new_mem_out) {
    if (*new_memory_size_inout > MAX_DECB_MEMORY_SIZE) {
        *new_memory_size_inout = decb_memory_size;
        *end_of_new_mem_out = decb_memory_size;
        return 32 /* Memory Full */;
    }
    if (*new_memory_size_inout <= decb_memory_size) {
        *new_memory_size_inout = decb_memory_size;
        *end_of_new_mem_out = decb_memory_size;
        return OKAY;
    }

    *end_of_new_mem_out = MAX_DECB_MEMORY_SIZE;
    return OKAY;
}

void decb_putstr(const char* s) {
    while (*s) {
        decb_putchar(*s);
        ++s;
    }
}

#ifdef FROB_DECB_CMOC
extern char* readline();
char* decb_readline() {
  return readline();
}
extern void putchar(char c);
void decb_putchar(int c) {
  putchar((char)c);
}
#endif
typedef unsigned int size_t;

#include "frob2/decb/std4gcc.h"

void abort(void) {
  decb_putstr(" *ABORT* *LOOP*\n");

  while(1){}
}

void exit(int status) {
  decb_putstr(" *EXIT* ");
  abort();
}

void* memcpy(void* dest, const void* src, size_t n) {
   char* d = dest;
   const char* s = src;
   for (size_t i=0; i<n; i++) {
     d[i] = s[i];
   }
   return d;
}

void* memset(void* dest, int ch, size_t n) {
   char* d = dest;
   for (size_t i=0; i<n; i++) {
     d[i] = (char)ch;
   }
   return d;
}

char *strcpy(char *dest, const char *src) {
   char* d = dest;
   const char* s = src;
   while (*s) {
     *d++ = *s++;
   }
   *d = '\0';
   return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
   char* d = dest;
   const char* s = src;
   while (*s) {
     *d++ = *s++;
     n--;
     if (n==0) return dest;
   }
   *d = '\0';
   return dest;
}

size_t strlen(const char *str) {
    const char* s = str;
    while (*s) {
        s++;
    }
    return s - str;
}


void decb_putchar(int c) {
    char ch = (char)c;
    if (ch==10) ch=13;

    // unsigned short emit = 0xA002; // for coco
    // asm volatile(" lda %0 \n jsr [%1]": : "m" (ch), "m" (emit));

    // asm volatile(" lda %0 \n ldx 0xA002 \n jsr ,x" : : "m" (ch) : "a", "x" );

    asm volatile("lda %0\n\tjsr [$A002]" : : "m" (ch) : "a" );
}

char* decb_readline() {
    char* LINBUF = (char*) 0x02DC; // for coco
    size_t LBUFMX = 250; // for coco

    // Start at 1, not 0.
    for (size_t i = 1; i < LBUFMX; i++) LINBUF[i]='\0';

    // Not sure what is clobbered, so I save & restore all registers.
    asm volatile("pshs D,X,Y,U\n\tjsr $A390\n\tpuls D,X,Y,U");
    return LINBUF + 1;
}
