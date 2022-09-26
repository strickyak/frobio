#ifndef _FROBIO_NCL_STD_H_
#define _FROBIO_NCL_STD_H_

#include "frobio/nytypes.h"

#define BUF_SIZE 200            /* instead of 1024 */

// prefixed_atoi() understands initial "0" for octal and "0x" for hex, and '-' before any of that.
int prefixed_atoi(const char *s);

#ifndef unix  // Use ordinary stdlib for unix.

void snprintf_s(char *buf, int max, const char *fmt, const char *s);
void snprintf_d(char *buf, int max, const char *fmt, int x);
void printf_d(const char *fmt, int x);
void printf_s(const char *fmt, const char *s);
char *strdup(const char *s);
char *strndup(const char *s, int n);
char *strdup_upper(const char *s);

#if 1  // If I don't provide one of these, HCF.

// avoid:
// 55 cmoc: warning: multiple definitions of symbol _memcpy in modules memcpy.os9_o, std.o
// 56 cmoc: warning: multiple definitions of symbol _strcat in modules std.o, strcat.os9_o
// 57 cmoc: warning: multiple definitions of symbol _strcpy in modules std.o, strcpy.os9_o
// 58 cmoc: warning: multiple definitions of symbol _strlen in modules std.o, strlen.os9_o

#define memcpy ncl_memcpy
#define strcat ncl_strcat
#define strcpy ncl_strcpy
#define strlen ncl_strlen

void* memcpy(void *d, const void *s, size_t sz);
int strcasecmp(const char *a, const char *b);
char* strcpy(char *d, const char *s);
size_t strlen(const char *p);
char* strcat(char *d, const char *s);
void bzero(char *p, int n);
#endif

#endif // !unix

#endif // _FROBIO_NCL_STD_H_
