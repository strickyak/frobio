#ifndef _FROB2_DECB_STD4GCC_H_
#define _FROB2_DECB_STD4GCC_H_
// _printf
// _readline

void decb_putchar(int c);
void decb_putstr(const char* s);
char* decb_readline();

void abort(void);
void exit(int status);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int ch, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *str);
#endif // _FROB2_DECB_STD4GCC_H_
