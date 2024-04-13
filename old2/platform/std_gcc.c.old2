typedef unsigned int size_t;

#include "frob2/frobtype.h"
#include "frob2/platform/std_gcc.h"

void BasicPutChar(char ch);
void abort() {
  BasicPutChar('#');
  BasicPutChar('A');
  BasicPutChar('B');
  BasicPutChar('O');
  BasicPutChar('R');
  BasicPutChar('T');
  BasicPutChar('#');
  while (1) {}
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

char* strcat(char* dest, const char* src) {
   char* d = dest;
   const char* s = src;
   while (*d) d++;
   while (*s) {
     *d++ = *s++;
   }
   *d = '\0';
   return dest;
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

int strcmp(const char* a, const char* b) {
  while (*a == *b) {
    if (*a < *b) return -1;
    if (*a > *b) return +1;
    a++, b++;
  }
  if (*a) return +1;
  if (*b) return -1;
  return 0;
}

static char UPPERCHAR(char ch) {
    if ('a' <= ch && ch <= 'z') return ch + 'A' - 'a';
    return ch;
}

int strcasecmp(const char *a, const char *b) {
  while (*a && *b) {
    if ((byte) UPPERCHAR(*a) < (byte) UPPERCHAR(*b))
      return -1;
    if ((byte) UPPERCHAR(*a) > (byte) UPPERCHAR(*b))
      return +1;
    a++;
    b++;
  }
  // at least one is 0.
  if (*a) return +1;
  if (*b) return -1;
  return 0;
}

int atoi(const char* s) {
  int z = 0;
  bool negative = false;
  if (*s == '-') {
    negative = true;
    s++;
  }
  while ('0' <= *s && *s <= '9') {
    z = 10*z + (*s - '0');
  }
  return negative ? -z : z;
}
