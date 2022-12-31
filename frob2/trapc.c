extern char *decb_readline();
extern void decb_putchar(int ch);
extern void decb_putstr(const char* s);

char* trapped_sp;


char FindDark(char** p) {
  while (**p) {
    if (33 <= **p && **p <= 126) return **p;
    (*p)++;
  }
  return '\0';
}

void PutHexDigit(char x) {
  x &= 15;
  if (x < 10) decb_putchar('0' + x);
  else decb_putchar('A' + x - 10);
}

void ShowRegs() {
  for (unsigned i=0; i<24; i++) {
    PutHexDigit(trapped_sp[i] >> 4);
    PutHexDigit(trapped_sp[i]);
    if ((i&3)==3) decb_putchar('-');
  }
}

void DecbTrap() {
  while(1) {
    decb_putstr("::");
    char *p = decb_readline();
    char ch = FindDark(&p);
    switch (ch) {
      case '\0':
        return;
      case 'r':
        ShowRegs();
        break;
      default:
        decb_putstr("??");
        break;
    }
  }
}
