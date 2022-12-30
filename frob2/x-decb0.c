// x-decb0 tries readline and printf in DECB via CMOC.

#include <frob2/froblib.h>

extern char* readline(void);

int main() {
  while (true) {
    decb_putstr(" ok: ");
    char* s = decb_readline();
    s[80] = '\0';
    LogInfo("back=%d len=%d line=%q", s[-1], strlen(s), s);
    if (s[0]=='q' || s[0]=='Q') break;
  }
  LogStatus("Done.");
  for (word i=0; i<65000; i++) {
    i=i;
  }
  return 0;
}
