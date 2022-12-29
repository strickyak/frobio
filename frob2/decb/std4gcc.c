typedef unsigned int size_t;

#include "frob2/decb/std4gcc.h"

void abort(void) {
  decb_putstr(" *ABORT* *LOOP*\n");

  asm volatile("abort_loop: bra abort_loop");
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

    unsigned short emit = 0xA002; // for coco

    asm volatile(" lda %0 \n jmp [%1]": : "m" (ch), "m" (emit));
}

char* decb_readline() {
    char* LINBUF = 0x02DC; // for coco
    size_t LBUFMX = 250; // for coco

    for (size_t i = 0; i < LBUFMX; i++) LINBUF[i]='\0';

    asm volatile(" JSR     $A390");
    return LINBUF + 1;
}
