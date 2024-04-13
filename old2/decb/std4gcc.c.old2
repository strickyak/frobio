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
  while (*a == *b) a++, b++;
  if (*a < *b) return -1;
  if (*a > *b) return +1;
  return 0;
}

int atoi(const char* s) {
  int z = 0;
  while ('0' <= *s && *s <= '9') {
    z = 10*z + (*s - '0');
  }
  return z;
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
