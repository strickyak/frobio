#ifndef _FROB3_FROBLIB_H_
#define _FROB3_FROBLIB_H_

#include "frob3/frobtype.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#define EOF (-1)        // TODO not use this?


// was in decb/std4gcc.h {{{
void abort(void);
void exit(int status);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int ch, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *str);
int strcmp(const char* a, const char* b);
int atoi(const char* s);
// was in decb/std4gcc.h }}}

//@ void exit(int);
//@ size_t strlen(const char*);
//@ void memcpy(void*, const void*, size_t n);
//@ void memset(void*, byte b, size_t n);
char* strcat(char* dest, const char* src);
//@ void strcpy(char* dest, const char* src);
//@ void strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* left, const char* right);
int strncmp(const char* left, const char* right, size_t n);
int strcasecmp(const char* left, const char* right);
int strncasecmp(const char* left, const char* right, size_t n);
char* strdup(const char* s);
char* strndup(const char* s, size_t n);
int atoi(const char*);

// void sprintf(char*, const char* fmt, ...); // DANGER
#ifndef FOR_FIX
void printf(const char* fmt, ...);  // The slow one. // WRONG
void putchar(char ch); // WRONG
#endif

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// Check a function call returning a bool result.
#define CheckB(FN,ARGS) {\
  bool _ok_ = FN ARGS; \
  if (!_ok_) LogFatal("CANNOT %s", #FN); \
}

// Check a function call returning an errnum result.
#define CheckE(FN,ARGS) {\
  errnum _e_ = FN ARGS; \
  if (_e_) LogFatal("CANNOT %s: %d", #FN, _e_); \
}

// Check a function call returning a prob result.
#define CheckP(FN,ARGS) {\
  prob _ps_ = FN ARGS; \
  if (_ps_) LogFatal("CANNOT %s: %s", #FN, _ps_); \
}

#define OKAY ((errnum)0)

//////// Buf

typedef struct Buf {
  char *s;
  int n;
} Buf;
#define BUF_INITIAL_CAP 16

void BufCheck(Buf *buf);
void BufInit(Buf *buf);
void BufInitWith(Buf *buf, const char *s);
void BufInitTake(Buf *buf, char *s);
void BufDel(Buf *buf);
char *BufFinish(Buf *buf);
char *BufTake(Buf *buf);
const char *BufPeek(Buf *buf);
void BufAppC(Buf *buf, char c);
void BufAppS(Buf *buf, const char *s, int n);
void BufAppStr(Buf *buf, const char *s); // uses strlen(s)
void BufAppDope(Buf *buf, const char *s);
const char **BufTakeDope(Buf *buf, int *lenP);

// For NCL list elements.
void BufAppElemC(Buf *buf, char c);
void BufAppElemS(Buf *buf, const char *s);
int ElemLen(const char *s, const char **endP);
const char *ElemDecode(const char *s);

//////// stdio

typedef struct { int fd; } File;

// {
// Low Level: These do not invoke Malloc, Fail, Printf, Buf, etc.
//
// nystdio
// FPuts returns length of str, or -1 on error.
int FPuts(const char *str, File *f);
void PutHex(byte c, word w);
void Panic(const char* message);
void PC_Trace(byte c, const void* w);
// PError prints the given str and the value of ErrNo in decimal.
void PError(const char* str);
// PErrorFatal is like PError but also exits 127.
void PErrorFatal(const char* str); // exits 127.
void PErrNum(errnum e);

// }


// FOpen mode can be "r" or "w".
File* FOpen(const char* pathname, const char* mode);
// FGets returns bytes_read if good, or 0 and sets ErrNo on error.
word FGets(char *buf, int size, File *f);
// FClose returns 0 if good, or -1 on error.
int FClose(File *f);

extern File StdIn_File;
extern File StdOut_File;
extern File StdErr_File;
#define StdIn (&StdIn_File)
#define StdOut (&StdOut_File)
#define StdErr (&StdErr_File)

//////// Printf and formatting.

int Printf(const char* fmt, ...);
int EPrintf(const char* fmt, ...);
int FPrintf(File* f, const char* fmt, ...);
int SPrintf(char* dest, const char* fmt, ...);
int SnPrintf(char* dest, size_t max, const char* fmt, ...);
char* StrFormat(const char* fmt, ...);

extern byte ShortStaticBuffer[24];
extern const char LowerHexAlphabet[];
extern const char UpperHexAlphabet[];

const char* StaticFormatSignedInt(int x);
void BufAppStringQuoting(Buf* buf, const char* s, word precision);
void BufFormat(Buf* buf, const char* format, ...);
void BufFormatVA(Buf* buf, const char* format, va_list ap);
errnum WritLnAll(int path, const char* s, word n);

// CharUp(c): convert to upper case for 26 ascii letters.
char CharUp(char c);
// CharDown(c): convert to lower case for 26 ascii letters.
char CharDown(char c);


//////// Strings and Memory

void* Malloc(word n);
void* ReAlloc(void* p, word n);
void Free(void* p);

word StrLen(const byte* p);
mstring StrDup(const byte* p);
void StrCpy(byte*a, const byte*b);
void StrCat(byte*a, const byte*b);
bool StrEq(const byte*a, const byte*b);
bool StrNEq(const byte*a, const byte*b, word n);
bool StrCaseEq(const byte*a, const byte*b);
//bool StrNCaseEq(const byte*a, const byte*b, word n);
int StrCmp(const byte*a, const byte*b);
int StrNCmp(const byte*a, const byte*b, word n);
int MemCmp(const byte*a, const byte*b);
void BZero(void* p, word len);

//////// GetFlag

// Call SkipArg() once before using GetFlag,
// to skip the unused argv0 cell.
void SkipArg(int* p_argc, char*** p_argv);

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
bool GetFlag(int* p_argc, char*** p_argv, const char* flagDesc);
extern char FlagChar;
extern const char *FlagArg;

//////// NyLib

// bytes instead of chars:
size_t StrLen(const byte* s);

// NyWhite returns true if input is white:
// Spaces and all control chars <32 are white.
bool NyWhite(char c);

// NySplit splits on NyWhite chars, which may be redundant.
// No empty words are returned.
// Return number of words out, which may be 0.
int NySplit(char* s, char**words_out, int max_words);

// String comparison.
bool NyStrEq(const char* a, const char* b);
bool NyCharEqIgnoreCase(char x, char y);
bool NyStrEqIgnoreCase(const char* a, const char* b);

void NyFormatDottedDecimalQuad(char* buffer, quad addr);

byte NyDeHex(byte ch);
void NySkipSpaces(const char** pp);
byte NyParseChar(const char** pp);
byte NyParseHexByte(const char** pp);
word NyParseHexWord(const char** pp);
word NyParseDecimalWord(const char** pp);
byte NyCheckByte(word a);
quad NyFormQuadFromBytes(byte a, byte b, byte c, byte d);
quad NyParseDottedDecimalQuad(const char** pp);
quad NyParseDottedDecimalQuadAndPort(const char** pp, word* port);

// ncl/std
int prefixed_atoi(const char*);

////////   Operating System Errors
extern errnum ErrNo;  // for OS Error number.

////////   Logging and Verbosity
extern byte Verbosity;
extern Buf LogBuf;
void Logger(const char* file, word line, byte level, const char* fmt, ...);

// Log Levels
#define LLFatal   1
#define LLError   2
#define LLStatus  3
#define LLStep    4
#define LLInfo    5
#define LLDetail  6
#define LLDebug   7 

#ifndef MAX_VERBOSE
#define MAX_VERBOSE LLInfo // Override this if needed.
#endif

#if LLFatal <= MAX_VERBOSE
#define LogFatal(fmt,...) do { \
    if (LLFatal <= Verbosity) \
        Logger(BASENAME, __LINE__, LLFatal, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogFatal(fmt,...) {}
#endif

#if LLError <= MAX_VERBOSE
#define LogError(fmt,...) do { \
    if (LLError <= Verbosity) \
        Logger(BASENAME, __LINE__, LLError, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogError(fmt,...) {}
#endif

#if LLStatus <= MAX_VERBOSE
#define LogStatus(fmt,...) do { \
    if (LLStatus <= Verbosity) \
        Logger(BASENAME, __LINE__, LLStatus, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogStatus(fmt,...) {}
#endif

#if LLStep <= MAX_VERBOSE
#define LogStep(fmt,...) do { \
    if (LLStep <= Verbosity) \
        Logger(BASENAME, __LINE__, LLStep, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogStep(fmt,...) {}
#endif

#if LLInfo <= MAX_VERBOSE
#define LogInfo(fmt,...) do { \
    if (LLInfo <= Verbosity) \
        Logger(BASENAME, __LINE__, LLInfo, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogInfo(fmt,...) {}
#endif

#if LLDetail <= MAX_VERBOSE
#define LogDetail(fmt,...) do { \
    if (LLDetail <= Verbosity) \
        Logger(BASENAME, __LINE__, LLDetail, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogDetail(fmt,...) {}
#endif

#if LLDebug <= MAX_VERBOSE
#define LogDebug(fmt,...) do { \
    if (LLDebug <= Verbosity) \
        Logger(BASENAME, __LINE__, LLDebug, (fmt), ##__VA_ARGS__); \
  } while (false)
#else
#define LogDebug(fmt,...) {}
#endif

//////// Assert

// Assert() logs Fatal if the cond is not met.
#define Assert(cond) do { if (!(cond)) { \
  LogFatal("ASSERT FAILED: %s:%u\n", \
      BASENAME, __LINE__); } } while(false) 


//////// Macros.

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

#endif // _FROB3_FROBLIB_H_
