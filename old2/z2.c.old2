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

