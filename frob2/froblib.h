// Decisions:
//   1. There are too many verisons of standard C library.
//      Many of them don't work, or have problems.
//      So we will write our own replacement library.
//      We will use capitalized names.  (Then even Unix
//      can use our replacement library for testing it.)
//   2. We will prefer unsigned byte and unsigned word.
//      Migrate our libraries to use that.
//   3. We will not support threads, for now.
//   4. Therefore we can use static buffers, briefly.
//   5. Therefore a global ErrNo, ErrLine, ErrFile, ErrBuf
//      will suffice.
//   6. Standard Logging framework.
//   7. Standard GetOpt framework.


#ifndef _FROB2_FROBLIB_H_
#define _FROB2_FROBLIB_H_

#include <stdarg.h>

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
#ifdef unix

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>

// Fundamental type definitions for nylib & frobio.
#ifndef __cplusplus
#define true 1
#define false 0
typedef unsigned char bool;  // use a byte for a bool.
#endif
typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned long word;  // word is 8 bytes, unsigned.
typedef unsigned long quad;  // quad is 8 bytes, unsigned, 4 bytes used.

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
#else

// Fundamental type definitions for nylib & frobio.
#define true 1
#define false 0
typedef unsigned char bool;  // use a byte for a bool.
typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned int word;   // word is 2 bytes, unsigned.
typedef unsigned long quad;  // quad is 4 bytes, unsigned.
typedef unsigned int size_t;

#define NULL ((void*)0)
#define EOF (-1)        // TODO not use this?

void exit(int);
size_t strlen(const char*);
void memcpy(void*, const void*, size_t n);
void memset(void*, byte b, size_t n);
void printf(const char* fmt, ...);  // The slow one.
void strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* left, const char* right);
int strncmp(const char* left, const char* right, size_t n);
char* strdup(const char*);

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
#endif

typedef char* mstring;  // a Malloc'ed string
typedef byte errnum;     // for OS error numbers.

#define MAXWORD ((word)(-1L))
#define OKAY ((errnum)0)         // todo -- eliminate

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

// Low Level: do not invoke Malloc, Fail, Printf, etc.
void FPuts(const char *str, File *f);
void PutHex(byte c, word w);
void Panic(const char* message);
void PC_Trace(byte c, const void* w);

// Fopen mode can be "r" or "w".
File* FOpen(const char* pathname, const char* mode);
void FGets(char *buf, int size, File *f);
void FClose(File *f);
void PError(const char* str);

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
char* StrFormat(const char* fmt, ...);

extern byte ShortStaticBuffer[24];
extern const byte* LowerHexAlphabet;
extern const byte* UpperHexAlphabet;

const char* StaticFormatSignedInt(int x);
void BufAppStringQuoting(Buf* buf, const char* s);
void BufFormat(Buf* buf, const char* format, ...);
void BufFormatVA(Buf* buf, const char* format, va_list ap);
void WritLnAll(int path, const char* s, word n);

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
bool StrNCaseEq(const byte*a, const byte*b, word n);
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

////////   Errors

extern Buf ErrBuf;
extern const char* ErrFile;
extern word ErrLine;
extern errnum ErrNo;  // for OS Error number.

// To declare failures
void SetFailure(const char* file, word line, byte e, const char* fmt, ...);
#define Fail(fmt,...) SetFailure(__FILE__, __LINE__, 0, (fmt), ##__VA_ARGS__)
#define FailE(E,fmt,...) SetFailure(__FILE__, __LINE__, (E), (fmt), ##__VA_ARGS__)

// To test for failures
#define Bad (ErrBuf.n)  // e.g. if (Bad) return;

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

#define LogFatal(fmt,...) \
    if (LLFatal <= MAX_VERBOSE && LLFatal <= Verbosity) \
        Logger(__FILE__, __LINE__, LLFatal, (fmt), ##__VA_ARGS__)

#define LogError(fmt,...) \
    if (LLError <= MAX_VERBOSE && LLError <= Verbosity) \
        Logger(__FILE__, __LINE__, LLError, (fmt), ##__VA_ARGS__)

#define LogStatus(fmt,...) \
    if (LLStatus <= MAX_VERBOSE && LLStatus <= Verbosity) \
        Logger(__FILE__, __LINE__, LLStatus, (fmt), ##__VA_ARGS__)

#define LogStep(fmt,...) \
    if (LLStep <= MAX_VERBOSE && LLStep <= Verbosity) \
        Logger(__FILE__, __LINE__, LLStep, (fmt), ##__VA_ARGS__)

#define LogInfo(fmt,...) \
    if (LLInfo <= MAX_VERBOSE && LLInfo <= Verbosity) \
        Logger(__FILE__, __LINE__, LLInfo, (fmt), ##__VA_ARGS__)

#define LogDetail(fmt,...) \
    if (LLDetail <= MAX_VERBOSE && LLDetail <= Verbosity) \
        Logger(__FILE__, __LINE__, LLDetail, (fmt), ##__VA_ARGS__)

#define LogDebug(fmt,...) \
    if (LLDebug <= MAX_VERBOSE && LLDebug <= Verbosity) \
        Logger(__FILE__, __LINE__, LLDebug, (fmt), ##__VA_ARGS__)

//////// Assert

#define Assert(cond) { if (!(cond)) { \
  LogFatal("***ASSERT FAILED: %s:%u: %s\n", \
      __FILE__, __LINE__, #cond); } }

#endif // _FROB2_FROBLIB_H_


/*
     1	#ifdef unix
     2	#include <stdio.h>
     3	#else
     4	#include <cmoc.h>
     5	#endif
     6	int main() {
     7	    printf("char : %d\n", (int)sizeof(char));
     8	    printf("short : %d\n", (int)sizeof(short));
     9	    printf("int : %d\n", (int)sizeof(int));
    10	    printf("long : %d\n", (int)sizeof(long));
    11	    printf("long long : %d\n", (int)sizeof(long long));
    12	    printf("size_t : %d\n", (int)sizeof(size_t));
    13	    printf("char* : %d\n", (int)sizeof(char*));
    13	    printf("0 < (size_t)(-1L) = %d\n", 0 < (size_t)(-1L));
    14	    return 0;
    15	}

*** cmoc --os9:  M6809

char : 1
short : 2
int : 2
long : 4
long long : 4
size_t : 2
char* : 2
0 < (size_t)(-1L) = 1

*** cc: x86_64 x86_64 x86_64 GNU/Linux

char : 1
short : 2
int : 4
long : 8
long long : 8
size_t : 8
char* : 8
0 < (size_t)(-1L) = 1

*/