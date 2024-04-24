// frob3/metal/metal.h
// Conventional types and constants for metal programs on Frobio.
// Assumes gcc6809 (our gcc 4.6.4 for M6809).

#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1
#define NULL (void *)0

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
typedef unsigned int size_t;
typedef void (*func_t)();

#define CASBUF 0x01DA  // Rock the CASBUF, rock the CASBUF!
#define VDG_RAM 0x0400
#define VDG_END 0x0600

#define Peek(A) (*(volatile byte *)(A))
#define Poke(A, X) (*(volatile byte *)(A)) = (X)

#define Peek2(A) (*(volatile word *)(A))
#define Poke2(A, X) (*(volatile word *)(A)) = (X)

// frob3/metal/eeprom.h:
void EepromEnableProtection();
void EepromDisableProtection();

// frob3/metal/console.h:
char PolCat();
byte IsThisGomar();
void Delay(word n);
word ScreenAt(byte x, byte y);
void PrintAt(byte x, byte y, const char *s);
void Fatal(const char *s);
void ClearScreen(byte ch);
void PrintHex(word loc, word val);

extern const char VdgHex[];

// frob3/metal/lemma-client.h:
byte WizGet1(word reg);
word WizGet2(word reg);
void WizPut1(word reg, byte value);
void WizPut2(word reg, word value);
void DetectWiz();
void WizIssueCommand(byte cmd);
word WizGetNumBytesWaiting();
byte GetLemma();  // Receive 1 byte from Lemma

// frob3/metal/arcfour.h:

struct rc4_engine;
void rc4_swap_byte(unsigned char *a, unsigned char *b);
void rc4_prepare_key(unsigned char *key_data_ptr, int key_data_len,
                     struct rc4_engine *engine);
void rc4(unsigned char *buffer_ptr, int buffer_len, struct rc4_engine *engine);

// standard.h:
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcpy(char *restrict dest, const char *restrict src);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t max);

// END
